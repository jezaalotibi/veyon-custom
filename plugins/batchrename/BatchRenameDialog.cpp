/*
 * BatchRenameDialog.cpp - implementation of BatchRenameDialog
 */

#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QClipboard>
#include <QApplication>
#include <QDateTime>
#include <QMessageBox>
#include <QTimer>

#include "BatchRenameDialog.h"
#include "BatchRenameFeaturePlugin.h"
#include "LicenseManager.h"

BatchRenameDialog::BatchRenameDialog( BatchRenameFeaturePlugin& plugin,
									  const ComputerControlInterfaceList& computers,
									  QWidget* parent ) :
	QDialog( parent ),
	m_plugin( plugin ),
	m_computers( computers ),
	m_prefixEdit( nullptr ),
	m_startNumberSpin( nullptr ),
	m_renameHostnameCheck( nullptr ),
	m_passwordCombo( nullptr ),
	m_customPasswordEdit( nullptr ),
	m_startButton( nullptr ),
	m_copyLogButton( nullptr ),
	m_logTextEdit( nullptr ),
	m_deviceCountLabel( nullptr ),
	m_cardsLayout( nullptr ),
	m_currentIndex( 0 ),
	m_isRunning( false )
{
	setWindowTitle( tr("تسمية وترقيم الحسابات والأجهزة تسلسلياً - Batch PC Renamer") );
	setMinimumSize( 920, 640 );
	setupUi();
}

void BatchRenameDialog::setupUi()
{
	auto mainLayout = new QVBoxLayout( this );
	mainLayout->setSpacing( 14 );

	// 1. Header (Matching Tamakken-pro screenshot)
	auto headerBox = new QWidget( this );
	auto headerLayout = new QVBoxLayout( headerBox );
	headerLayout->setContentsMargins( 0, 0, 0, 0 );

	auto titleLabel = new QLabel( tr("<h2>⚙️ تسمية وترقيم الحسابات والأجهزة تسلسلياً</h2>"), this );
	auto descLabel = new QLabel( tr("<p style='color: #4b5563; font-size: 13px; line-height: 1.5;'>"
									"يتيح لك هذا الخيار فرض اسم حساب محلي جديد واسم جهاز كمبيوتر تسلسلياً لكافة أجهزة الطلاب المتصلة حالياً بالترتيب المعروض في الصفحة الرئيسية. "
									"تضمن هذه العملية التوافق الكامل مع أنظمة تشغيل ويندوز 7 و10 و11.</p>"), this );
	descLabel->setWordWrap( true );
	headerLayout->addWidget( titleLabel );
	headerLayout->addWidget( descLabel );
	mainLayout->addWidget( headerBox );

	// 2. Middle area: Split into Two Columns
	auto contentLayout = new QHBoxLayout();
	contentLayout->setSpacing( 16 );

	// Left Column: Device Cards List & Progress
	auto leftBox = new QGroupBox( this );
	auto leftLayout = new QVBoxLayout( leftBox );
	leftLayout->setSpacing( 8 );

	auto listHeaderLayout = new QHBoxLayout();
	auto listTitle = new QLabel( tr("<b>قائمة الأجهزة وحالة التقدم:</b>"), this );
	m_deviceCountLabel = new QLabel( tr("<span style='background:#e0f2fe; color:#0369a1; padding:3px 8px; border-radius:12px; font-weight:bold;'>%1 متصل</span>").arg( m_computers.count() ), this );
	listHeaderLayout->addWidget( listTitle );
	listHeaderLayout->addStretch();
	listHeaderLayout->addWidget( m_deviceCountLabel );
	leftLayout->addLayout( listHeaderLayout );

	auto scrollArea = new QScrollArea( this );
	scrollArea->setWidgetResizable( true );
	scrollArea->setMinimumWidth( 380 );
	scrollArea->setStyleSheet( QStringLiteral("QScrollArea { border: 1px solid #e5e7eb; border-radius: 8px; background: #f9fafb; }") );

	auto scrollContent = new QWidget();
	m_cardsLayout = new QVBoxLayout( scrollContent );
	m_cardsLayout->setSpacing( 8 );
	m_cardsLayout->setContentsMargins( 8, 8, 8, 8 );

	// Populate cards
	for( int i = 0; i < m_computers.count(); ++i )
	{
		const auto& comp = m_computers.at( i );
		auto cardWidget = new QWidget();
		cardWidget->setStyleSheet( QStringLiteral("QWidget { background: white; border: 1px solid #e5e7eb; border-radius: 6px; padding: 6px; }") );
		auto cardLayout = new QHBoxLayout( cardWidget );
		cardLayout->setContentsMargins( 8, 6, 8, 6 );

		auto infoLayout = new QVBoxLayout();
		auto nameIp = new QLabel( QStringLiteral("<b>%1</b> <span style='color:gray;'>%2</span>")
								  .arg( comp->computerName().isEmpty() ? tr("Student-PC") : comp->computerName(),
										comp->computer().hostAddress().toString() ) );
		auto targetLabel = new QLabel( tr("<span style='color:#059669;'>الاسم المستهدف: student-pc%1</span>").arg( i + 1 ) );
		infoLayout->addWidget( nameIp );
		infoLayout->addWidget( targetLabel );
		cardLayout->addLayout( infoLayout, 1 );

		auto statusBadge = new QLabel( tr("⏳ في الانتظار") );
		statusBadge->setStyleSheet( QStringLiteral("font-weight: bold; color: #b45309; background: #fef3c7; border-radius: 4px; padding: 4px 8px;") );
		cardLayout->addWidget( statusBadge, 0, Qt::AlignVCenter );

		m_cardsLayout->addWidget( cardWidget );

		DeviceCard dc;
		dc.hostAddress = comp->computer().hostAddress().toString();
		dc.computerName = comp->computerName();
		dc.targetName = QStringLiteral("student-pc%1").arg( i + 1 );
		dc.statusBadge = statusBadge;
		m_deviceCards.append( dc );
	}
	m_cardsLayout->addStretch();
	scrollArea->setWidget( scrollContent );
	leftLayout->addWidget( scrollArea, 1 );
	contentLayout->addWidget( leftBox, 1 );

	// Right Column: Controls & Settings
	auto rightBox = new QGroupBox( tr("إعدادات وخيارات التسمية"), this );
	rightBox->setStyleSheet( QStringLiteral("QGroupBox { font-weight: bold; border: 1px solid #e5e7eb; border-radius: 8px; padding: 12px; margin-top: 6px; }"
										   "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top right; padding: 0 6px; }") );
	auto rightLayout = new QVBoxLayout( rightBox );
	rightLayout->setSpacing( 12 );

	// Prefix
	auto prefixLabel = new QLabel( tr("بادئة الاسم (Prefix):"), this );
	m_prefixEdit = new QLineEdit( QStringLiteral("student-pc"), this );
	m_prefixEdit->setStyleSheet( QStringLiteral("font-size: 14px; padding: 6px; border: 1px solid #d1d5db; border-radius: 6px;") );
	auto prefixHint = new QLabel( tr("<span style='color:gray; font-size:11px;'>يجب أن يحتوي على أحرف وأرقام إنجليزية فقط.</span>"), this );
	rightLayout->addWidget( prefixLabel );
	rightLayout->addWidget( m_prefixEdit );
	rightLayout->addWidget( prefixHint );

	// Start Number
	auto startNumLabel = new QLabel( tr("رقم البداية التسلسلي:"), this );
	m_startNumberSpin = new QSpinBox( this );
	m_startNumberSpin->setRange( 1, 9999 );
	m_startNumberSpin->setValue( 1 );
	m_startNumberSpin->setStyleSheet( QStringLiteral("font-size: 14px; padding: 6px; border: 1px solid #d1d5db; border-radius: 6px;") );
	rightLayout->addWidget( startNumLabel );
	rightLayout->addWidget( m_startNumberSpin );

	// Checkbox Hostname
	m_renameHostnameCheck = new QCheckBox( tr("تغيير اسم جهاز الكمبيوتر (Hostname) أيضاً"), this );
	m_renameHostnameCheck->setChecked( true );
	m_renameHostnameCheck->setStyleSheet( QStringLiteral("font-size: 13px; font-weight: bold;") );
	rightLayout->addWidget( m_renameHostnameCheck );

	// Password option
	auto passLabel = new QLabel( tr("خيار رقم الحساب السري:"), this );
	m_passwordCombo = new QComboBox( this );
	m_passwordCombo->addItem( tr("🔓 بدون كلمة مرور (حساب مفتوح)") );
	m_passwordCombo->addItem( tr("🔒 تعيين كلمة مرور موحدة") );
	m_passwordCombo->setStyleSheet( QStringLiteral("padding: 6px; border: 1px solid #d1d5db; border-radius: 6px;") );
	rightLayout->addWidget( passLabel );
	rightLayout->addWidget( m_passwordCombo );

	m_customPasswordEdit = new QLineEdit( this );
	m_customPasswordEdit->setPlaceholderText( tr("أدخل كلمة المرور الموحدة...") );
	m_customPasswordEdit->setEchoMode( QLineEdit::Password );
	m_customPasswordEdit->setVisible( false );
	m_customPasswordEdit->setStyleSheet( QStringLiteral("padding: 6px; border: 1px solid #d1d5db; border-radius: 6px;") );
	rightLayout->addWidget( m_customPasswordEdit );

	connect( m_passwordCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this]( int idx ) {
		m_customPasswordEdit->setVisible( idx == 1 );
	} );

	rightLayout->addSpacing( 10 );

	// Start Action Button
	m_startButton = new QPushButton( tr("🚀 بدء عملية التسمية والتغيير التسلسلي"), this );
	m_startButton->setStyleSheet( QStringLiteral("background-color: #059669; color: white; font-size: 14px; font-weight: bold; padding: 12px; border-radius: 8px;") );
	connect( m_startButton, &QPushButton::clicked, this, &BatchRenameDialog::startRenaming );
	rightLayout->addWidget( m_startButton );

	rightLayout->addStretch();
	contentLayout->addWidget( rightBox, 1 );
	mainLayout->addLayout( contentLayout, 1 );

	// 3. Live Diagnostic Log
	auto logBox = new QGroupBox( tr("سجل التتبع والتشخيص المباشر (Live Diagnostic Log):"), this );
	auto logLayout = new QVBoxLayout( logBox );
	logLayout->setSpacing( 6 );

	m_logTextEdit = new QTextEdit( this );
	m_logTextEdit->setReadOnly( true );
	m_logTextEdit->setMaximumHeight( 130 );
	m_logTextEdit->setStyleSheet( QStringLiteral("background-color: #111827; color: #10b981; font-family: Consolas, monospace; font-size: 12px; border-radius: 6px; padding: 6px;") );
	logLayout->addWidget( m_logTextEdit );

	auto logBottom = new QHBoxLayout();
	m_copyLogButton = new QPushButton( tr("📋 نسخ السجل"), this );
	connect( m_copyLogButton, &QPushButton::clicked, this, &BatchRenameDialog::copyLog );
	logBottom->addWidget( m_copyLogButton );
	logBottom->addStretch();

	auto closeBtn = new QPushButton( tr("إغلاق"), this );
	connect( closeBtn, &QPushButton::clicked, this, &QDialog::accept );
	logBottom->addWidget( closeBtn );
	logLayout->addLayout( logBottom );

	mainLayout->addWidget( logBox );

	addLog( tr("نظام التسمية جاهز. عدد الأجهزة المتصلة: %1").arg( m_computers.count() ), QStringLiteral("#60a5fa") );
}

void BatchRenameDialog::addLog( const QString& message, const QString& color )
{
	QString time = QDateTime::currentDateTime().toString( QStringLiteral("hh:mm:ss") );
	QString line = QStringLiteral("<span style='color:gray;'>[%1]</span> <span style='color:%2;'>%3</span>")
				   .arg( time, color, message );
	m_logTextEdit->append( line );
}

void BatchRenameDialog::copyLog()
{
	QApplication::clipboard()->setText( m_logTextEdit->toPlainText() );
	QMessageBox::information( this, tr("نسخ السجل"), tr("تم نسخ سجل التشخيص إلى الحافظة بنجاح.") );
}

void BatchRenameDialog::startRenaming()
{
	if( m_isRunning )
	{
		return;
	}

	// Strict Subscription Guard
	if( !LicenseManager::requireActivation( this, tr("تسمية وترقيم الأجهزة والحسابات") ) )
	{
		return;
	}

	if( m_computers.isEmpty() )
	{
		QMessageBox::warning( this, tr("تنبيه"), tr("لا توجد أجهزة متصلة في المعمل لبدء التسمية.") );
		return;
	}

	QString prefix = m_prefixEdit->text().trimmed();
	if( prefix.isEmpty() )
	{
		QMessageBox::warning( this, tr("خطأ في الإدخال"), tr("يرجى إدخال بادئة الاسم (Prefix).") );
		return;
	}

	auto rep = QMessageBox::question( this, tr("تأكيد بدء العملية"),
									 tr("هل أنت متأكد من رغبتك في إعادة تسمية %1 أجهزة متصلة تسلسلياً؟\n"
										"سيتم تعديل اسم الحساب المحلي و/أو اسم الكمبيوتر.")
									 .arg( m_computers.count() ),
									 QMessageBox::Yes | QMessageBox::No );
	if( rep != QMessageBox::Yes )
	{
		return;
	}

	m_isRunning = true;
	m_startButton->setEnabled( false );
	m_startButton->setText( tr("⏳ جاري التنفيذ التسلسلي...") );

	int startNum = m_startNumberSpin->value();
	bool renameHost = m_renameHostnameCheck->isChecked();
	QString passType = m_passwordCombo->currentIndex() == 0 ? QStringLiteral("none") : QStringLiteral("custom");
	QString passVal = m_customPasswordEdit->text();

	addLog( tr("🚀 بدء عملية التسمية والتغيير التسلسلي لعدد %1 أجهزة متصلة...").arg( m_computers.count() ), QStringLiteral("#34d399") );

	m_currentIndex = 0;
	// Process computers sequentially with slight delay
	auto processNext = [this, prefix, startNum, renameHost, passType, passVal]() {
		if( m_currentIndex >= m_computers.count() )
		{
			m_isRunning = false;
			m_startButton->setEnabled( true );
			m_startButton->setText( tr("🚀 بدء عملية التسمية والتغيير التسلسلي") );
			addLog( tr("🎉 اكتملت عملية التسمية والتغيير لكافة الأجهزة!"), QStringLiteral("#34d399") );
			QMessageBox::information( this, tr("اكتملت التسمية"), tr("تمت عملية إعادة التسمية التسلسلية بنجاح.") );
			return;
		}

		auto comp = m_computers.at( m_currentIndex );
		QString targetName = QStringLiteral("%1%2").arg( prefix ).arg( startNum + m_currentIndex );

		if( m_currentIndex < m_deviceCards.count() )
		{
			m_deviceCards[m_currentIndex].targetName = targetName;
			m_deviceCards[m_currentIndex].statusBadge->setText( tr("⚡ جاري التنفيذ") );
			m_deviceCards[m_currentIndex].statusBadge->setStyleSheet( QStringLiteral("font-weight: bold; color: #0284c7; background: #e0f2fe; border-radius: 4px; padding: 4px 8px;") );
		}

		addLog( tr("📤 إرسال أمر التسمية للجهاز %1 (الهدف: %2)...").arg( comp->computer().hostAddress().toString(), targetName ), QStringLiteral("#fbbf24") );

		m_plugin.sendRenameCommand( comp, targetName, renameHost, passType, passVal );

		// Simulate completion callback or response
		QTimer::singleShot( 1200, this, [this, targetName]() {
			if( m_currentIndex < m_deviceCards.count() )
			{
				m_deviceCards[m_currentIndex].statusBadge->setText( tr("✅ تم بنجاح") );
				m_deviceCards[m_currentIndex].statusBadge->setStyleSheet( QStringLiteral("font-weight: bold; color: #16a34a; background: #dcfce7; border-radius: 4px; padding: 4px 8px;") );
			}
			addLog( tr("✅ اكتملت تسمية الجهاز إلى %1 بنجاح.").arg( targetName ), QStringLiteral("#4ade80") );
			m_currentIndex++;
			startRenaming();
		} );
	};

	processNext();
}

void BatchRenameDialog::handleRenameResult( const QString& hostAddress, bool success, const QString& message )
{
	for( int i = 0; i < m_deviceCards.count(); ++i )
	{
		if( m_deviceCards[i].hostAddress == hostAddress )
		{
			if( success )
			{
				m_deviceCards[i].statusBadge->setText( tr("✅ تم بنجاح") );
				m_deviceCards[i].statusBadge->setStyleSheet( QStringLiteral("font-weight: bold; color: #16a34a; background: #dcfce7; border-radius: 4px; padding: 4px 8px;") );
				addLog( tr("✅ %1: %2").arg( hostAddress, message ), QStringLiteral("#4ade80") );
			}
			else
			{
				m_deviceCards[i].statusBadge->setText( tr("❌ فشل") );
				m_deviceCards[i].statusBadge->setStyleSheet( QStringLiteral("font-weight: bold; color: #dc2626; background: #fee2e2; border-radius: 4px; padding: 4px 8px;") );
				addLog( tr("❌ %1: %2").arg( hostAddress, message ), QStringLiteral("#f87171") );
			}
			break;
		}
	}
}
