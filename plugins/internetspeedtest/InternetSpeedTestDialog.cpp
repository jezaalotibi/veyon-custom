/*
 * InternetSpeedTestDialog.cpp - implementation of InternetSpeedTestDialog
 */

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

#include "InternetSpeedTestDialog.h"
#include "InternetSpeedTestFeaturePlugin.h"

InternetSpeedTestDialog::InternetSpeedTestDialog( InternetSpeedTestFeaturePlugin& plugin,
												  const ComputerControlInterfaceList& computers,
												  QWidget* parent ) :
	QDialog( parent ),
	m_plugin( plugin ),
	m_computers( computers ),
	m_avgDownloadLabel( nullptr ),
	m_avgPingLabel( nullptr ),
	m_slowestHostLabel( nullptr ),
	m_tableWidget( nullptr ),
	m_startButton( nullptr ),
	m_exportButton( nullptr )
{
	setWindowTitle( tr("Internet Speed Test - فحص سرعة الإنترنت") );
	setMinimumSize( 750, 520 );
	setupUi();
}

void InternetSpeedTestDialog::setupUi()
{
	auto mainLayout = new QVBoxLayout( this );
	mainLayout->setSpacing( 12 );

	// Title header
	auto titleLabel = new QLabel( tr("<h2>⚡ اختبار سرعة الإنترنت للأجهزة (Internet Speed Test)</h2>"
									 "<p style='color:gray;'>قم بفحص أداء الإنترنت على أجهزة الطلاب ومراقبة زمن الاستجابة والسرعة في المعمل.</p>"), this );
	mainLayout->addWidget( titleLabel );

	// Cards layout
	auto cardsLayout = new QHBoxLayout();

	auto makeCard = []( const QString& title, QLabel*& valueLabel, const QString& color ) {
		auto box = new QGroupBox( title );
		box->setStyleSheet( QStringLiteral("QGroupBox { font-weight: bold; border: 2px solid %1; border-radius: 8px; margin-top: 6px; padding: 10px; }"
										   "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; color: %1; }").arg( color ) );
		auto layout = new QVBoxLayout( box );
		valueLabel = new QLabel( QStringLiteral("N/A") );
		valueLabel->setAlignment( Qt::AlignCenter );
		valueLabel->setStyleSheet( QStringLiteral("font-size: 20px; font-weight: bold; color: %1;").arg( color ) );
		layout->addWidget( valueLabel );
		return box;
	};

	cardsLayout->addWidget( makeCard( tr("متوسط سرعة التنزيل بالمعمل"), m_avgDownloadLabel, QStringLiteral("#059669") ) );
	cardsLayout->addWidget( makeCard( tr("متوسط زمن الاستجابة (Ping)"), m_avgPingLabel, QStringLiteral("#2563EB") ) );
	cardsLayout->addWidget( makeCard( tr("أبطأ جهاز بالشبكة حالياً"), m_slowestHostLabel, QStringLiteral("#D97706") ) );

	mainLayout->addLayout( cardsLayout );

	// Table
	m_tableWidget = new QTableWidget( this );
	m_tableWidget->setColumnCount( 5 );
	m_tableWidget->setHorizontalHeaderLabels( {
		tr("اسم الجهاز"),
		tr("عنوان IP"),
		tr("زمن الاستجابة (Ping)"),
		tr("سرعة التنزيل (Download)"),
		tr("الحالة")
	} );
	m_tableWidget->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
	m_tableWidget->setRowCount( m_computers.size() );

	for( int i = 0; i < m_computers.size(); ++i )
	{
		const auto& comp = m_computers[i]->computer();
		QString name = comp.displayName().isEmpty() ? comp.hostName() : comp.displayName();
		if( name.isEmpty() )
		{
			name = comp.hostAddress().toString();
		}

		m_tableWidget->setItem( i, 0, new QTableWidgetItem( name ) );
		m_tableWidget->setItem( i, 1, new QTableWidgetItem( comp.hostAddress().toString() ) );
		m_tableWidget->setItem( i, 2, new QTableWidgetItem( QStringLiteral("-") ) );
		m_tableWidget->setItem( i, 3, new QTableWidgetItem( QStringLiteral("-") ) );
		m_tableWidget->setItem( i, 4, new QTableWidgetItem( tr("جاهز للفحص") ) );
	}

	mainLayout->addWidget( m_tableWidget );

	// Bottom action buttons
	auto btnLayout = new QHBoxLayout();

	m_startButton = new QPushButton( tr("🚀 بدء الفحص الشامل للكل"), this );
	m_startButton->setStyleSheet( QStringLiteral("background-color: #0d9488; color: white; font-weight: bold; padding: 8px 16px; border-radius: 6px; font-size: 14px;") );
	connect( m_startButton, &QPushButton::clicked, this, &InternetSpeedTestDialog::startTest );
	btnLayout->addWidget( m_startButton );

	m_exportButton = new QPushButton( tr("📄 تصدير التقرير (CSV)"), this );
	connect( m_exportButton, &QPushButton::clicked, this, &InternetSpeedTestDialog::exportReport );
	btnLayout->addWidget( m_exportButton );

	btnLayout->addStretch();

	auto closeBtn = new QPushButton( tr("إغلاق"), this );
	connect( closeBtn, &QPushButton::clicked, this, &QDialog::accept );
	btnLayout->addWidget( closeBtn );

	mainLayout->addLayout( btnLayout );
}

void InternetSpeedTestDialog::startTest()
{
	m_startButton->setEnabled( false );
	m_startButton->setText( tr("⏳ جاري الفحص...") );

	for( int i = 0; i < m_tableWidget->rowCount(); ++i )
	{
		m_tableWidget->setItem( i, 2, new QTableWidgetItem( tr("جاري القياس...") ) );
		m_tableWidget->setItem( i, 3, new QTableWidgetItem( tr("جاري القياس...") ) );
		m_tableWidget->setItem( i, 4, new QTableWidgetItem( tr("قيد الاختبار") ) );
	}

	m_plugin.executeSpeedTest( m_computers, this );
}

void InternetSpeedTestDialog::updateResult( const QString& hostAddress, int pingMs, double downloadMbps )
{
	for( int i = 0; i < m_tableWidget->rowCount(); ++i )
	{
		auto ipItem = m_tableWidget->item( i, 1 );
		if( ipItem && ipItem->text() == hostAddress )
		{
			m_tableWidget->setItem( i, 2, new QTableWidgetItem( QStringLiteral("%1 ms").arg( pingMs ) ) );
			m_tableWidget->setItem( i, 3, new QTableWidgetItem( QStringLiteral("%1 Mbps").arg( downloadMbps, 0, 'f', 1 ) ) );
			QString status = (downloadMbps >= 20.0) ? tr("ممتاز ✓") : (downloadMbps >= 5.0 ? tr("جيد") : tr("ضعيف ⚠"));
			m_tableWidget->setItem( i, 4, new QTableWidgetItem( status ) );
			break;
		}
	}

	updateSummaryCards();
}

void InternetSpeedTestDialog::updateSummaryCards()
{
	double totalDownload = 0.0;
	int totalPing = 0;
	int count = 0;
	double minSpeed = 999999.0;
	QString slowestHost;

	for( int i = 0; i < m_tableWidget->rowCount(); ++i )
	{
		auto pingItem = m_tableWidget->item( i, 2 );
		auto downItem = m_tableWidget->item( i, 3 );
		auto nameItem = m_tableWidget->item( i, 0 );

		if( downItem && downItem->text().contains( QStringLiteral("Mbps") ) )
		{
			double speed = downItem->text().section( QLatin1Char(' '), 0, 0 ).toDouble();
			int ping = pingItem ? pingItem->text().section( QLatin1Char(' '), 0, 0 ).toInt() : 0;
			totalDownload += speed;
			totalPing += ping;
			count++;

			if( speed < minSpeed )
			{
				minSpeed = speed;
				slowestHost = nameItem ? nameItem->text() : QStringLiteral("Host");
			}
		}
	}

	if( count > 0 )
	{
		m_avgDownloadLabel->setText( QStringLiteral("%1 Mbps").arg( totalDownload / count, 0, 'f', 1 ) );
		m_avgPingLabel->setText( QStringLiteral("%1 ms").arg( totalPing / count ) );
		m_slowestHostLabel->setText( slowestHost + QStringLiteral(" (%1 Mbps)").arg( minSpeed, 0, 'f', 1 ) );
	}

	if( count == m_tableWidget->rowCount() )
	{
		m_startButton->setEnabled( true );
		m_startButton->setText( tr("🚀 بدء الفحص الشامل للكل") );
	}
}

void InternetSpeedTestDialog::exportReport()
{
	QString path = QFileDialog::getSaveFileName( this, tr("حفظ تقرير سرعة الإنترنت"),
												QStringLiteral("Internet_Speed_Report.csv"),
												tr("ملفات CSV (*.csv)") );
	if( path.isEmpty() )
	{
		return;
	}

	QFile file( path );
	if( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
	{
		QTextStream out( &file );
		out.setGenerateByteOrderMark( true );
		out << "اسم الجهاز,عنوان IP,زمن الاستجابة (ms),سرعة التنزيل (Mbps),الحالة\n";
		for( int i = 0; i < m_tableWidget->rowCount(); ++i )
		{
			out << m_tableWidget->item( i, 0 )->text() << ","
				<< m_tableWidget->item( i, 1 )->text() << ","
				<< m_tableWidget->item( i, 2 )->text() << ","
				<< m_tableWidget->item( i, 3 )->text() << ","
				<< m_tableWidget->item( i, 4 )->text() << "\n";
		}
		file.close();
		QMessageBox::information( this, tr("تصدير التقرير"), tr("تم تصدير التقرير بنجاح!") );
	}
}
