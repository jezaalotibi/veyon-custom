/*
 * QuizPlatformDialog.cpp - implementation of QuizPlatformDialog
 */

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>

#include "QuizPlatformDialog.h"
#include "QuizPlatformFeaturePlugin.h"

QuizPlatformDialog::QuizPlatformDialog( QuizPlatformFeaturePlugin& plugin,
									   const ComputerControlInterfaceList& computers,
									   QWidget* parent ) :
	QDialog( parent ),
	m_plugin( plugin ),
	m_computers( computers ),
	m_quizTitleEdit( nullptr ),
	m_quizUrlEdit( nullptr ),
	m_durationCombo( nullptr ),
	m_lockdownCheckBox( nullptr ),
	m_showResultsCheckBox( nullptr ),
	m_blockOtherSitesCheckBox( nullptr ),
	m_launchButton( nullptr )
{
	setWindowTitle( tr("منصة الاختبارات والأنشطة الذكية - Quiz Platform") );
	setMinimumSize( 620, 520 );
	setupUi();
}

void QuizPlatformDialog::setupUi()
{
	auto mainLayout = new QVBoxLayout( this );
	mainLayout->setSpacing( 14 );

	// Header
	auto titleLabel = new QLabel( tr("<h2>📝 منصة الاختبارات والأنشطة الذكية (Quiz Platform)</h2>"
									 "<p style='color:gray;'>قم بإرسال اختبارات وروابط الأنشطة التفاعلية لأجهزة الطلاب مع حماية كاملة ومنع الخروج.</p>"), this );
	mainLayout->addWidget( titleLabel );

	// Form group
	auto formBox = new QGroupBox( tr("بيانات الاختبار والنشاط"), this );
	auto formLayout = new QFormLayout( formBox );
	formLayout->setSpacing( 10 );

	m_quizTitleEdit = new QLineEdit( this );
	m_quizTitleEdit->setPlaceholderText( tr("مثال: اختبار الوحدة الأولى / نشاط برمجة بايثون") );
	formLayout->addRow( tr("عنوان الاختبار / الموضوع:"), m_quizTitleEdit );

	m_quizUrlEdit = new QLineEdit( this );
	m_quizUrlEdit->setPlaceholderText( tr("https://forms.microsoft.com/... أو رابط منصة مدرستي أو كويز") );
	formLayout->addRow( tr("رابط الاختبار (URL):"), m_quizUrlEdit );

	m_durationCombo = new QComboBox( this );
	m_durationCombo->addItems( {
		tr("5 دقائق"),
		tr("10 دقائق"),
		tr("15 دقيقة"),
		tr("30 دقيقة"),
		tr("45 دقيقة"),
		tr("غير محدد (مفتوح)")
	} );
	formLayout->addRow( tr("الوقت المتاح للإجابة:"), m_durationCombo );

	mainLayout->addWidget( formBox );

	// Security options group
	auto secBox = new QGroupBox( tr("الأمان والتصفح الآمن المقيد (Lockdown Controls)"), this );
	auto secLayout = new QVBoxLayout( secBox );

	m_lockdownCheckBox = new QCheckBox( tr("🔒 تشغيل وضع التصفح الآمن (ملء الشاشة مع منع الخروج وحظر مفاتيح الهروب)"), this );
	m_lockdownCheckBox->setChecked( true );
	secLayout->addWidget( m_lockdownCheckBox );

	m_blockOtherSitesCheckBox = new QCheckBox( tr("🛡️ حظر تصفح أي مواقع أخرى عدا هذا الاختبار طوال فترة الجلسة"), this );
	m_blockOtherSitesCheckBox->setChecked( true );
	secLayout->addWidget( m_blockOtherSitesCheckBox );

	m_showResultsCheckBox = new QCheckBox( tr("✓ عرض النتيجة والدرجة تلقائياً للطلاب بعد التسليم"), this );
	m_showResultsCheckBox->setChecked( true );
	secLayout->addWidget( m_showResultsCheckBox );

	mainLayout->addWidget( secBox );

	// Security alert card
	auto alertBox = new QGroupBox( tr("تنبيهات الغش والخصوصية الرقمية (Lockdown Alerts)"), this );
	alertBox->setStyleSheet( QStringLiteral("QGroupBox { border: 2px solid #ef4444; border-radius: 8px; margin-top: 6px; padding: 10px; color: #b91c1c; font-weight: bold; }") );
	auto alertLayout = new QVBoxLayout( alertBox );
	auto alertText = new QLabel( tr("✓ سيتم تقييد أجهزة الطلاب في وضع ملء الشاشة.\n"
									"✓ يتم منع استخدام اختصارات لوحة المفاتيح والتبديل بين النوافذ حتى انتهاء الاختبار."), this );
	alertText->setStyleSheet( QStringLiteral("color: #991b1b; font-size: 13px;") );
	alertLayout->addWidget( alertText );
	mainLayout->addWidget( alertBox );

	// Buttons
	auto btnLayout = new QHBoxLayout();

	m_launchButton = new QPushButton( tr("🚀 إرسال وبدء جلسة الاختبار للطلاب"), this );
	m_launchButton->setStyleSheet( QStringLiteral("background-color: #047857; color: white; font-weight: bold; padding: 10px 20px; border-radius: 6px; font-size: 14px;") );
	connect( m_launchButton, &QPushButton::clicked, this, &QuizPlatformDialog::launchQuiz );
	btnLayout->addWidget( m_launchButton );

	btnLayout->addStretch();

	auto cancelBtn = new QPushButton( tr("إلغاء"), this );
	connect( cancelBtn, &QPushButton::clicked, this, &QDialog::reject );
	btnLayout->addWidget( cancelBtn );

	mainLayout->addLayout( btnLayout );
}

void QuizPlatformDialog::launchQuiz()
{
	QString url = m_quizUrlEdit->text().trimmed();
	if( url.isEmpty() )
	{
		QMessageBox::warning( this, tr("رابط غير صحيح"), tr("يرجى إدخال رابط الاختبار أو النشاط أولاً.") );
		return;
	}

	bool lockdown = m_lockdownCheckBox->isChecked();
	bool blockOther = m_blockOtherSitesCheckBox->isChecked();
	QString title = m_quizTitleEdit->text().trimmed();

	m_plugin.dispatchQuizToStudents( m_computers, url, title, lockdown, blockOther );

	QMessageBox::information( this, tr("تم إرسال الاختبار"),
							  tr("تم إرسال الاختبار بنجاح إلى %1 جهاز طالب!").arg( m_computers.size() ) );
	accept();
}
