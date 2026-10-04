/*
 * LicenseActivationDialog.cpp - implementation of LicenseActivationDialog
 */

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>

#include "LicenseActivationDialog.h"
#include "LicenseManager.h"

LicenseActivationDialog::LicenseActivationDialog( QWidget* parent ) :
	QDialog( parent ),
	m_machineIdEdit( nullptr ),
	m_licenseKeyEdit( nullptr ),
	m_statusLabel( nullptr ),
	m_expiryLabel( nullptr ),
	m_seatsLabel( nullptr ),
	m_activateButton( nullptr ),
	m_deactivateButton( nullptr )
{
	setWindowTitle( tr("تنشيط وترخيص البرنامج - License Activation") );
	setMinimumSize( 540, 420 );
	setupUi();
	updateStatusDisplay();
}

void LicenseActivationDialog::setupUi()
{
	auto mainLayout = new QVBoxLayout( this );
	mainLayout->setSpacing( 14 );

	// Header
	auto title = new QLabel( tr("<h2>🔐 نظام تنشيط وترخيص البرنامج</h2>"
								"<p style='color:gray;'>قم بالتحقق السحابي وتفعيل ترخيص البرنامج عبر خادم الترخيص.</p>"), this );
	mainLayout->addWidget( title );

	// Machine ID group
	auto midBox = new QGroupBox( tr("معرّف هذا الجهاز (Machine ID)"), this );
	auto midLayout = new QHBoxLayout( midBox );

	m_machineIdEdit = new QLineEdit( LicenseManager::getMachineId(), this );
	m_machineIdEdit->setReadOnly( true );
	m_machineIdEdit->setStyleSheet( QStringLiteral("font-weight: bold; font-size: 14px; background: #f3f4f6;") );
	midLayout->addWidget( m_machineIdEdit );

	auto copyBtn = new QPushButton( tr("📋 نسخ"), this );
	connect( copyBtn, &QPushButton::clicked, this, &LicenseActivationDialog::copyMachineId );
	midLayout->addWidget( copyBtn );

	mainLayout->addWidget( midBox );

	// Status box
	auto statusBox = new QGroupBox( tr("حالة الترخيص الحالية"), this );
	auto statusForm = new QFormLayout( statusBox );
	statusForm->setSpacing( 8 );

	m_statusLabel = new QLabel( this );
	statusForm->addRow( tr("الحالة:"), m_statusLabel );

	m_expiryLabel = new QLabel( this );
	statusForm->addRow( tr("تاريخ الانتهاء:"), m_expiryLabel );

	m_seatsLabel = new QLabel( this );
	statusForm->addRow( tr("عدد الأجهزة المصرحة:"), m_seatsLabel );

	mainLayout->addWidget( statusBox );

	// Activation key input
	auto keyBox = new QGroupBox( tr("تنشيط ترخيص جديد (Online Activation)"), this );
	auto keyLayout = new QVBoxLayout( keyBox );

	m_licenseKeyEdit = new QLineEdit( this );
	m_licenseKeyEdit->setPlaceholderText( tr("أدخل مفتاح التنشيط: VEYON-XXXX-XXXXXXXXXXXX / LIC-...") );
	m_licenseKeyEdit->setText( LicenseManager::storedLicenseKey() );
	keyLayout->addWidget( m_licenseKeyEdit );

	auto actBtnLayout = new QHBoxLayout();
	m_activateButton = new QPushButton( tr("☁️ تنشيط عبر السحابة (Cloud Activate)"), this );
	m_activateButton->setStyleSheet( QStringLiteral("background-color: #2563eb; color: white; font-weight: bold; padding: 8px 16px; border-radius: 6px;") );
	connect( m_activateButton, &QPushButton::clicked, this, &LicenseActivationDialog::performOnlineActivation );
	actBtnLayout->addWidget( m_activateButton );

	m_deactivateButton = new QPushButton( tr("إلغاء التنشيط"), this );
	connect( m_deactivateButton, &QPushButton::clicked, this, &LicenseActivationDialog::deactiveLicense );
	actBtnLayout->addWidget( m_deactivateButton );

	keyLayout->addLayout( actBtnLayout );
	mainLayout->addWidget( keyBox );

	// Bottom
	auto btnLayout = new QHBoxLayout();
	btnLayout->addStretch();
	auto closeBtn = new QPushButton( tr("إغلاق"), this );
	connect( closeBtn, &QPushButton::clicked, this, &QDialog::accept );
	btnLayout->addWidget( closeBtn );

	mainLayout->addLayout( btnLayout );
}

void LicenseActivationDialog::updateStatusDisplay()
{
	auto res = LicenseManager::verifyLicense();
	if( res.isValid )
	{
		m_statusLabel->setText( QStringLiteral("<b style='color:green;'>✓ %1</b>").arg( res.message ) );
		m_expiryLabel->setText( QStringLiteral("<b>%1</b>").arg( res.expiryDate ) );
		m_seatsLabel->setText( res.studentLimit > 0 ? QStringLiteral("<b>%1 جهاز</b>").arg( res.studentLimit ) : tr("غير محدود") );
		m_deactivateButton->setEnabled( true );
	}
	else
	{
		m_statusLabel->setText( QStringLiteral("<b style='color:red;'>✗ %1</b>").arg( res.message ) );
		m_expiryLabel->setText( tr("غير متوفر") );
		m_seatsLabel->setText( tr("0 أجهزة") );
		m_deactivateButton->setEnabled( false );
	}
}

void LicenseActivationDialog::copyMachineId()
{
	QApplication::clipboard()->setText( m_machineIdEdit->text() );
	QMessageBox::information( this, tr("نسخ المعرّف"), tr("تم نسخ معرّف الجهاز إلى الحافظة بنجاح.") );
}

void LicenseActivationDialog::performOnlineActivation()
{
	QString key = m_licenseKeyEdit->text().trimmed();
	if( key.isEmpty() )
	{
		QMessageBox::warning( this, tr("مفتاح فارغ"), tr("يرجى إدخال مفتاح التنشيط أولاً.") );
		return;
	}

	m_activateButton->setEnabled( false );
	m_activateButton->setText( tr("⏳ جاري الاتصال بالسحابة...") );

	LicenseManager::activateOnline( key, [this]( bool ok, const QString& msg, const LicenseManager::VerificationResult& res ) {
		m_activateButton->setEnabled( true );
		m_activateButton->setText( tr("☁️ تنشيط عبر السحابة (Cloud Activate)") );

		if( ok )
		{
			QMessageBox::information( this, tr("نجاح التنشيط"), tr("🎉 تم تنشيط البرنامج بنجاح!\nصلاحية الترخيص: %1").arg( res.expiryDate ) );
			updateStatusDisplay();
		}
		else
		{
			QMessageBox::critical( this, tr("فشل التنشيط"), msg );
			updateStatusDisplay();
		}
	} );
}

void LicenseActivationDialog::deactiveLicense()
{
	auto rep = QMessageBox::question( this, tr("إلغاء التنشيط"), tr("هل أنت متأكد من رغبتك في إزالة الترخيص من هذا الجهاز؟"),
									 QMessageBox::Yes | QMessageBox::No );
	if( rep == QMessageBox::Yes )
	{
		LicenseManager::clearLicense();
		updateStatusDisplay();
		QMessageBox::information( this, tr("إلغاء التنشيط"), tr("تمت إزالة الترخيص بنجاح.") );
	}
}
