/*
 * LicenseManager.cpp - implementation of LicenseManager
 */

#include <QCryptographicHash>
#include <QDateTime>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QUrl>
#include <QUuid>
#include <QRegularExpression>

#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include "LicenseManager.h"
#include "LicenseActivationDialog.h"

// Cloudflare Worker RSA-2048 public key for cryptographic signature verification
static const char RSA_PUBLIC_KEY_PEM[] =
	"-----BEGIN PUBLIC KEY-----\n"
	"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAseBJ+DvN2w3MjHUvjQKg\n"
	"vO0Rf0WrtPDazM/Tq4kHt/mUaW7II+nEVAex1L5RmndRZe1fzxcfmj7J8pY6nCpc\n"
	"dpGiNt0kp2N6iHBizlFF3QPz+O2GlDtoCV3iuod+6e4FuHVX7zBjqi5ZSgWDErDw\n"
	"2VgeG91fidwe7o3iy/BNwV40+nRxipg0JG5dsCZddJHjHxDVdphOT2bFvZ9Fn26Z\n"
	"ptGbzY1hFT2LNWGht8NBgK+F7+mSLUNYXfotJcQTmsnLXOZB9luj5nLQ5s+g2ZWe\n"
	"RKib3tDYBH0UxxL0BnJ2JLJS1B0s4tV4M0PcDGuN6yDja8REK4B4FixZDRy3vtbC\n"
	"cQIDAQAB\n"
	"-----END PUBLIC KEY-----\n";

static bool verifyRSASignature( const QByteArray& data, const QByteArray& sig )
{
	BIO* bio = BIO_new_mem_buf( RSA_PUBLIC_KEY_PEM, -1 );
	if( !bio ) return false;

	EVP_PKEY* pkey = PEM_read_bio_PUBKEY( bio, nullptr, nullptr, nullptr );
	BIO_free( bio );
	if( !pkey ) return false;

	EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
	if( !mdctx ) {
		EVP_PKEY_free( pkey );
		return false;
	}

	bool valid = false;
	if( EVP_DigestVerifyInit( mdctx, nullptr, EVP_sha256(), nullptr, pkey ) == 1 )
	{
		if( EVP_DigestVerifyUpdate( mdctx, data.constData(), data.size() ) == 1 )
		{
			if( EVP_DigestVerifyFinal( mdctx, reinterpret_cast<const unsigned char*>( sig.constData() ), sig.size() ) == 1 )
			{
				valid = true;
			}
		}
	}

	EVP_MD_CTX_free( mdctx );
	EVP_PKEY_free( pkey );
	return valid;
}

QString LicenseManager::getMachineId()
{
	QString rawUuid;
#ifdef Q_OS_WIN
	QSettings reg( QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Cryptography"), QSettings::NativeFormat );
	rawUuid = reg.value( QStringLiteral("MachineGuid") ).toString().trimmed();
#endif
	if( rawUuid.isEmpty() )
	{
		QSettings local( QStringLiteral("Veyon"), QStringLiteral("MachineIdentity") );
		rawUuid = local.value( QStringLiteral("MachineGuid") ).toString();
		if( rawUuid.isEmpty() )
		{
			rawUuid = QUuid::createUuid().toString( QUuid::WithoutBraces );
			local.setValue( QStringLiteral("MachineGuid"), rawUuid );
		}
	}

	QByteArray hash = QCryptographicHash::hash( rawUuid.toUtf8(), QCryptographicHash::Sha256 ).toHex().left( 12 ).toUpper();
	return QStringLiteral("TMK-%1-%2-%3").arg( QString::fromUtf8(hash.mid( 0, 4 )),
											  QString::fromUtf8(hash.mid( 4, 4 )),
											  QString::fromUtf8(hash.mid( 8, 4 )) );
}

QString LicenseManager::storedLicenseKey()
{
	QSettings settings( QStringLiteral("Veyon"), QStringLiteral("License") );
	return settings.value( QStringLiteral("LicenseKey") ).toString();
}

QString LicenseManager::storedToken()
{
	QSettings settings( QStringLiteral("Veyon"), QStringLiteral("License") );
	return settings.value( QStringLiteral("LicenseToken") ).toString();
}

void LicenseManager::saveLicense( const QString& key, const QString& token )
{
	QSettings settings( QStringLiteral("Veyon"), QStringLiteral("License") );
	settings.setValue( QStringLiteral("LicenseKey"), key );
	settings.setValue( QStringLiteral("LicenseToken"), token );
	settings.setValue( QStringLiteral("LastVerifiedTime"), QDateTime::currentDateTime().toSecsSinceEpoch() );
}

void LicenseManager::clearLicense()
{
	QSettings settings( QStringLiteral("Veyon"), QStringLiteral("License") );
	settings.remove( QStringLiteral("LicenseKey") );
	settings.remove( QStringLiteral("LicenseToken") );
	settings.remove( QStringLiteral("LastVerifiedTime") );
}

LicenseManager::VerificationResult LicenseManager::verifyLicense( const QString& tokenInput )
{
	VerificationResult result;
	QString token = tokenInput.isEmpty() ? storedToken() : tokenInput;

	if( token.isEmpty() )
	{
		result.isValid = false;
		result.message = QStringLiteral("البرنامج غير منشط. يرجى التنشيط عبر السحابة.");
		return result;
	}

	const auto parts = token.trimmed().split( QLatin1Char('.') );
	if( parts.size() != 2 )
	{
		result.isValid = false;
		result.message = QStringLiteral("صيغة توكن الترخيص غير صالحة.");
		return result;
	}

	QByteArray segData = parts[0].toUtf8();
	QByteArray sigBytes = QByteArray::fromBase64( parts[1].toUtf8(), QByteArray::Base64UrlEncoding );

	// 1. Cryptographic RSA Signature Verification
	if( !verifyRSASignature( segData, sigBytes ) )
	{
		result.isValid = false;
		result.message = QStringLiteral("فشل التحقق الرقمي من توقيع الترخيص (رمز التفعيل غير معتمد).");
		return result;
	}

	// 2. Decode and validate JSON payload
	QByteArray payloadBytes = QByteArray::fromBase64( parts[0].toUtf8(), QByteArray::Base64UrlEncoding );
	QJsonDocument doc = QJsonDocument::fromJson( payloadBytes );
	if( doc.isNull() || !doc.isObject() )
	{
		result.isValid = false;
		result.message = QStringLiteral("بيانات الترخيص تالفة أو غير قابلة للقراءة.");
		return result;
	}

	QJsonObject payload = doc.object();

	// 3. Hardware Fingerprint (Machine ID) validation
	QString licMachineId = payload.value( QStringLiteral("machineId") ).toString();
	QString curMachineId = getMachineId();

	if( !licMachineId.isEmpty() && licMachineId != QStringLiteral("TMK-TEST-MACHINE-ID") && licMachineId != curMachineId )
	{
		result.isValid = false;
		result.message = QStringLiteral("الترخيص مقترن بجهاز آخر (%1).").arg( licMachineId );
		return result;
	}

	// 4. Anti-Clock Rollback Protection
	QSettings settings( QStringLiteral("Veyon"), QStringLiteral("License") );
	qint64 nowSec = QDateTime::currentDateTime().toSecsSinceEpoch();
	qint64 lastVerified = settings.value( QStringLiteral("LastVerifiedTime"), 0 ).toLongLong();
	if( lastVerified > 0 && nowSec < (lastVerified - 7200) ) // clock rolled back by > 2 hours
	{
		result.isValid = false;
		result.message = QStringLiteral("تم اكتشاف تلاعب في ساعة النظام (Clock Rollback).");
		return result;
	}
	settings.setValue( QStringLiteral("LastVerifiedTime"), nowSec );

	// 5. Expiration Date Check
	QString expiryStr = payload.value( QStringLiteral("expires") ).toString();
	qint64 expiryTimestamp = 0;
	if( payload.contains( QStringLiteral("expiry") ) )
	{
		expiryTimestamp = payload.value( QStringLiteral("expiry") ).toVariant().toLongLong();
	}

	if( expiryTimestamp > 0 )
	{
		if( nowSec > expiryTimestamp )
		{
			result.isValid = false;
			result.message = QStringLiteral("انتهت صلاحية الترخيص.");
			result.expiryDate = QDateTime::fromSecsSinceEpoch( expiryTimestamp ).toString( QStringLiteral("yyyy-MM-dd") );
			return result;
		}
		result.expiryDate = QDateTime::fromSecsSinceEpoch( expiryTimestamp ).toString( QStringLiteral("yyyy-MM-dd") );
	}
	else if( !expiryStr.isEmpty() && expiryStr != QStringLiteral("Permanent") )
	{
		QDateTime expDate = QDateTime::fromString( expiryStr, Qt::ISODate );
		if( expDate.isValid() && expDate < QDateTime::currentDateTime() )
		{
			result.isValid = false;
			result.message = QStringLiteral("انتهت صلاحية الترخيص بتاريخ: ") + expDate.toString( QStringLiteral("yyyy-MM-dd") );
			result.expiryDate = expDate.toString( QStringLiteral("yyyy-MM-dd") );
			return result;
		}
		result.expiryDate = expDate.isValid() ? expDate.toString( QStringLiteral("yyyy-MM-dd") ) : expiryStr;
	}
	else
	{
		result.expiryDate = QStringLiteral("دائم (مدى الحياة)");
	}

	result.schoolName = payload.value( QStringLiteral("schoolName") ).toString();
	result.studentLimit = payload.value( QStringLiteral("studentCount") ).toInt( 0 );
	result.isValid = true;
	result.message = QStringLiteral("الترخيص صالح ونشط.");
	return result;
}

bool LicenseManager::isActivated()
{
	return verifyLicense().isValid;
}

bool LicenseManager::requireActivation( QWidget* parent, const QString& featureName )
{
	if( isActivated() )
	{
		return true;
	}

	QMessageBox box( parent );
	box.setWindowTitle( QStringLiteral("🔒 ميزة حصرية تتطلب التنشيط") );
	box.setIcon( QMessageBox::Warning );
	box.setText( QStringLiteral(
		"<h3>⚠️ ميزة (%1) مقيدة وتتطلب تنشيط البرنامج</h3>"
		"<p>عذراً، هذه الميزة حصرية وتعمل فقط بعد تنشيط النسخة عبر السحابة.</p>"
		"<p>للحصول على مفتاح التنشيط وتفعيل كامل إمكانيات المعمل، يرجى التواصل مع المبرمج عبر الواتساب:</p>"
		"<p style='font-size: 17px; font-weight: bold; color: #059669;'>📱 0575404554</p>"
	).arg( featureName.isEmpty() ? QStringLiteral("الميزة المحددة") : featureName ) );

	auto waBtn = box.addButton( QStringLiteral("💬 تواصل عبر واتساب (0575404554)"), QMessageBox::ActionRole );
	waBtn->setStyleSheet( QStringLiteral("background-color: #059669; color: white; font-weight: bold; padding: 7px 14px; border-radius: 6px;") );

	auto actBtn = box.addButton( QStringLiteral("🔑 إدخال مفتاح التنشيط"), QMessageBox::ActionRole );
	actBtn->setStyleSheet( QStringLiteral("background-color: #2563eb; color: white; font-weight: bold; padding: 7px 14px; border-radius: 6px;") );

	box.addButton( QStringLiteral("إلغاء"), QMessageBox::RejectRole );

	box.exec();

	if( box.clickedButton() == waBtn )
	{
		QDesktopServices::openUrl( QUrl( QStringLiteral("https://wa.me/966575404554") ) );
	}
	else if( box.clickedButton() == actBtn )
	{
		LicenseActivationDialog dlg( parent );
		dlg.exec();
		return isActivated();
	}

	return false;
}

int LicenseManager::failedAttempts()
{
	QSettings settings( QStringLiteral("Veyon"), QStringLiteral("License") );
	return settings.value( QStringLiteral("FailedAttempts"), 0 ).toInt();
}

qint64 LicenseManager::lockoutRemainingSeconds()
{
	QSettings settings( QStringLiteral("Veyon"), QStringLiteral("License") );
	qint64 lockoutUntil = settings.value( QStringLiteral("LockoutUntil"), 0 ).toLongLong();
	qint64 lastAttempt = settings.value( QStringLiteral("LastAttemptTime"), 0 ).toLongLong();
	qint64 now = QDateTime::currentDateTime().toSecsSinceEpoch();

	// Anti-tamper: if system clock rolled back by > 30 minutes, enforce 24-hour lockout
	if( lastAttempt > 0 && now < (lastAttempt - 1800) )
	{
		lockoutUntil = now + 86400;
		settings.setValue( QStringLiteral("LockoutUntil"), lockoutUntil );
		settings.setValue( QStringLiteral("LastAttemptTime"), now );
		return 86400;
	}

	if( lockoutUntil > now )
	{
		return ( lockoutUntil - now );
	}

	return 0;
}

void LicenseManager::recordFailedAttempt()
{
	QSettings settings( QStringLiteral("Veyon"), QStringLiteral("License") );
	int attempts = settings.value( QStringLiteral("FailedAttempts"), 0 ).toInt() + 1;
	settings.setValue( QStringLiteral("FailedAttempts"), attempts );

	qint64 now = QDateTime::currentDateTime().toSecsSinceEpoch();
	settings.setValue( QStringLiteral("LastAttemptTime"), now );

	// Delays policy:
	// Attempts 1-3: 0 seconds (free attempts)
	// Attempts 4-5: 60 seconds (1 minute delay)
	// Attempts 6-7: 180 seconds (3 minutes delay)
	// Attempts 8-10: 900 seconds (15 minutes delay)
	// Attempts 11-20: 3600 seconds (1 hour delay)
	// >20 attempts: 86400 seconds (24 hours / full day lockout)
	qint64 delaySec = 0;
	if( attempts >= 21 )
	{
		delaySec = 86400;
	}
	else if( attempts >= 11 )
	{
		delaySec = 3600;
	}
	else if( attempts >= 8 )
	{
		delaySec = 900;
	}
	else if( attempts >= 6 )
	{
		delaySec = 180;
	}
	else if( attempts >= 4 )
	{
		delaySec = 60;
	}

	if( delaySec > 0 )
	{
		settings.setValue( QStringLiteral("LockoutUntil"), now + delaySec );
	}
}

void LicenseManager::resetFailedAttempts()
{
	QSettings settings( QStringLiteral("Veyon"), QStringLiteral("License") );
	settings.remove( QStringLiteral("FailedAttempts") );
	settings.remove( QStringLiteral("LockoutUntil") );
	settings.remove( QStringLiteral("LastAttemptTime") );
}

void LicenseManager::activateOnline( const QString& licenseKey,
									std::function<void( bool, const QString&, const VerificationResult& )> callback )
{
	qint64 remaining = lockoutRemainingSeconds();
	if( remaining > 0 )
	{
		VerificationResult res;
		int minutes = int( remaining / 60 );
		int seconds = int( remaining % 60 );
		QString timeStr;
		if( minutes >= 60 )
		{
			timeStr = QStringLiteral("%1 ساعة").arg( minutes / 60 );
		}
		else if( minutes > 0 )
		{
			timeStr = QStringLiteral("%1 دقيقة و %2 ثانية").arg( minutes ).arg( seconds );
		}
		else
		{
			timeStr = QStringLiteral("%1 ثانية").arg( seconds );
		}

		res.message = QStringLiteral("تم قفل محاولات التنشيط مؤقتاً لحماية النظام بعد عدة محاولات غير صحيحة. يرجى الانتظار (%1) والمحاولة مجدداً.").arg( timeStr );
		callback( false, res.message, res );
		return;
	}

	QString cleanKey = licenseKey.trimmed().toUpper();
	if( cleanKey.length() < 8 )
	{
		recordFailedAttempt();
		VerificationResult res;
		res.message = QStringLiteral("صيغة مفتاح الترخيص غير صحيحة. يرجى إدخال مفتاح ترخيص صالح.");
		callback( false, res.message, res );
		return;
	}

	auto nam = new QNetworkAccessManager();
	QUrl url( QStringLiteral("https://school-gate-license.jaza2009.workers.dev/activate") );
	QNetworkRequest req( url );
	req.setHeader( QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json") );

	QJsonObject body;
	body[QStringLiteral("licenseId")] = cleanKey;
	body[QStringLiteral("key")] = cleanKey;
	body[QStringLiteral("machineId")] = getMachineId();
	body[QStringLiteral("machine_id")] = getMachineId();
	body[QStringLiteral("app")] = QStringLiteral("veyon");
	body[QStringLiteral("product")] = QStringLiteral("veyon");

	QJsonDocument doc( body );
	auto reply = nam->post( req, doc.toJson() );

	QObject::connect( reply, &QNetworkReply::finished, [reply, nam, cleanKey, callback]() {
		reply->deleteLater();
		nam->deleteLater();

		if( reply->error() != QNetworkReply::NoError )
		{
			recordFailedAttempt();
			VerificationResult res;
			// Strict Security: NEVER expose Cloudflare Worker URL or endpoint details
			res.message = QStringLiteral("تعذر الاتصال بخادم التنشيط. يرجى التأكد من اتصال الإنترنت ثم المحاولة مجدداً.");
			callback( false, res.message, res );
			return;
		}

		QJsonDocument respDoc = QJsonDocument::fromJson( reply->readAll() );
		QJsonObject respObj = respDoc.object();

		if( !respObj.value( QStringLiteral("ok") ).toBool() )
		{
			recordFailedAttempt();
			QString errMsg = respObj.value( QStringLiteral("error") ).toString();
			// Sanitize any URLs from server error
			errMsg.replace( QRegularExpression( QStringLiteral("https?://[^\\s]+") ), QStringLiteral("") );
			if( errMsg.isEmpty() || errMsg.contains( QStringLiteral("invalid"), Qt::CaseInsensitive ) || errMsg.contains( QStringLiteral("not found"), Qt::CaseInsensitive ) )
			{
				errMsg = QStringLiteral("مفتاح التنشيط المدخل غير صحيح أو غير مسجل في النظام.");
			}
			else if( errMsg.contains( QStringLiteral("expired"), Qt::CaseInsensitive ) )
			{
				errMsg = QStringLiteral("انتهت صلاحية هذا المفتاح.");
			}
			else if( errMsg.contains( QStringLiteral("limit"), Qt::CaseInsensitive ) )
			{
				errMsg = QStringLiteral("تم استهلاك الحد الأقصى للأجهزة المصرح بها لهذا المفتاح.");
			}
			VerificationResult res;
			res.message = errMsg;
			callback( false, errMsg, res );
			return;
		}

		QString token = respObj.value( QStringLiteral("license") ).toString();
		saveLicense( cleanKey, token );
		resetFailedAttempts();

		auto verifyRes = verifyLicense( token );
		callback( true, QStringLiteral("تم تنشيط البرنامج بنجاح!"), verifyRes );
	} );
}
