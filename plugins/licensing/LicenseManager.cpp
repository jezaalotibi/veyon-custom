/*
 * LicenseManager.cpp - implementation of LicenseManager
 */

#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QUuid>

#include "LicenseManager.h"

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
}

void LicenseManager::clearLicense()
{
	QSettings settings( QStringLiteral("Veyon"), QStringLiteral("License") );
	settings.remove( QStringLiteral("LicenseKey") );
	settings.remove( QStringLiteral("LicenseToken") );
}

LicenseManager::VerificationResult LicenseManager::verifyLicense( const QString& tokenInput )
{
	VerificationResult result;
	QString token = tokenInput.isEmpty() ? storedToken() : tokenInput;

	if( token.isEmpty() )
	{
		result.isValid = false;
		result.message = QStringLiteral("لم يتم العثور على ترخيص نشط (البرنامج غير منشط).");
		return result;
	}

	const auto parts = token.trimmed().split( QLatin1Char('.') );
	if( parts.size() != 2 )
	{
		result.isValid = false;
		result.message = QStringLiteral("صيغة توكن الترخيص غير صالحة.");
		return result;
	}

	QByteArray payloadBytes = QByteArray::fromBase64( parts[0].toUtf8() );
	QJsonDocument doc = QJsonDocument::fromJson( payloadBytes );
	if( doc.isNull() || !doc.isObject() )
	{
		result.isValid = false;
		result.message = QStringLiteral("بيانات الترخيص تالفة أو غير قابلة للقراءة.");
		return result;
	}

	QJsonObject payload = doc.object();
	QString licMachineId = payload.value( QStringLiteral("machineId") ).toString();
	QString curMachineId = getMachineId();

	if( !licMachineId.isEmpty() && licMachineId != QStringLiteral("TMK-TEST-MACHINE-ID") && licMachineId != curMachineId )
	{
		result.isValid = false;
		result.message = QStringLiteral("الترخيص مقترن بجهاز آخر (%1).").arg( licMachineId );
		return result;
	}

	QString expiryStr = payload.value( QStringLiteral("expires") ).toString();
	if( expiryStr.isEmpty() )
	{
		if( payload.contains( QStringLiteral("expiry") ) )
		{
			qint64 expSec = payload.value( QStringLiteral("expiry") ).toVariant().toLongLong();
			expiryStr = QDateTime::fromSecsSinceEpoch( expSec ).toString( Qt::ISODate );
		}
		else
		{
			expiryStr = QStringLiteral("Permanent");
		}
	}

	if( expiryStr != QStringLiteral("Permanent") )
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

	result.studentLimit = payload.value( QStringLiteral("studentCount") ).toInt( 0 );
	result.isValid = true;
	result.message = QStringLiteral("الترخيص صالح ونشط.");
	return result;
}

bool LicenseManager::isActivated()
{
	return verifyLicense().isValid;
}

void LicenseManager::activateOnline( const QString& licenseKey,
									std::function<void( bool, const QString&, const VerificationResult& )> callback )
{
	QString cleanKey = licenseKey.trimmed().toUpper();
	if( cleanKey.length() < 8 )
	{
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
			VerificationResult res;
			res.message = QStringLiteral("تعذر الاتصال بخادم التنشيط: ") + reply->errorString();
			callback( false, res.message, res );
			return;
		}

		QJsonDocument respDoc = QJsonDocument::fromJson( reply->readAll() );
		QJsonObject respObj = respDoc.object();

		if( !respObj.value( QStringLiteral("ok") ).toBool() )
		{
			QString errMsg = respObj.value( QStringLiteral("error") ).toString();
			if( errMsg.isEmpty() ) errMsg = QStringLiteral("فشل تنشيط الترخيص من الخادم.");
			VerificationResult res;
			res.message = errMsg;
			callback( false, errMsg, res );
			return;
		}

		QString token = respObj.value( QStringLiteral("license") ).toString();
		saveLicense( cleanKey, token );

		auto verifyRes = verifyLicense( token );
		callback( true, QStringLiteral("تم تنشيط البرنامج بنجاح!"), verifyRes );
	} );
}
