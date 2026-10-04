/*
 * ThemeManager.cpp - implementation of ThemeManager
 */

#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QMessageBox>

#include "ThemeManager.h"
#include "LicenseManager.h"

ThemeManager::Theme ThemeManager::currentTheme()
{
	QSettings settings( QStringLiteral("Veyon"), QStringLiteral("UI") );
	int val = settings.value( QStringLiteral("SelectedTheme"), 0 ).toInt();
	if( val < 0 || val > 3 ) val = 0;
	return static_cast<Theme>( val );
}

QString ThemeManager::themeName( Theme theme )
{
	switch( theme )
	{
	case Theme::Vision2030:
		return QStringLiteral("🇸🇦 ستايل رؤية المملكة 2030 (الأخضر الملكي الزمردي)");
	case Theme::FoundingDay:
		return QStringLiteral("🏛️ ستايل يوم التأسيس والتراث الأصيل (العنابي والطين الذهبي)");
	case Theme::ModernNajd:
		return QStringLiteral("🌌 ستايل نجد العصري الحديث (الكحلي الليلي والكهرمان الذهبي)");
	case Theme::Default:
	default:
		return QStringLiteral("◻️ المظهر الافتراضي الكلاسيكي");
	}
}

QString ThemeManager::themeStylesheet( Theme theme )
{
	switch( theme )
	{
	case Theme::Vision2030:
		return QStringLiteral(
			"QMainWindow, QDialog, QWidget { background-color: #0b1c14; color: #f1f5f9; font-family: 'Segoe UI', Tahoma, Arial; }\n"
			"QToolBar { background-color: #042918; border-bottom: 2px solid #006C35; spacing: 6px; padding: 4px; }\n"
			"QToolButton { background-color: #0c3823; color: #ffffff; border: 1px solid #006C35; border-radius: 6px; padding: 6px 12px; font-weight: bold; }\n"
			"QToolButton:hover { background-color: #006C35; border: 1px solid #d4af37; color: #ffffff; }\n"
			"QToolButton:checked { background-color: #006C35; border: 2px solid #d4af37; color: #d4af37; }\n"
			"QMenuBar, QMenu { background-color: #042918; color: #f8fafc; border: 1px solid #006C35; }\n"
			"QMenu::item:selected { background-color: #006C35; color: #ffffff; }\n"
			"QPushButton { background-color: #006C35; color: #ffffff; border: 1px solid #047857; border-radius: 6px; padding: 6px 16px; font-weight: bold; }\n"
			"QPushButton:hover { background-color: #059669; border: 1px solid #d4af37; }\n"
			"QTableWidget, QListView, QTreeView { background-color: #081a12; color: #f1f5f9; gridline-color: #006C35; border: 1px solid #006C35; border-radius: 6px; }\n"
			"QHeaderView::section { background-color: #042918; color: #d4af37; border: 1px solid #006C35; padding: 4px; font-weight: bold; }\n"
			"QGroupBox { border: 1px solid #006C35; border-radius: 8px; margin-top: 12px; font-weight: bold; color: #d4af37; }\n"
			"QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top right; padding: 0 6px; }\n"
			"QLineEdit, QComboBox { background-color: #0f2d1e; color: #ffffff; border: 1px solid #006C35; border-radius: 6px; padding: 5px; }\n"
			"QStatusBar { background-color: #042918; color: #94a3b8; border-top: 1px solid #006C35; }\n"
			"QScrollBar:vertical { background: #042918; width: 12px; }\n"
			"QScrollBar::handle:vertical { background: #006C35; border-radius: 6px; }\n"
		);

	case Theme::FoundingDay:
		return QStringLiteral(
			"QMainWindow, QDialog, QWidget { background-color: #1e080e; color: #f5ebd9; font-family: 'Segoe UI', Tahoma, Arial; }\n"
			"QToolBar { background-color: #380d18; border-bottom: 2px solid #b45309; spacing: 6px; padding: 4px; }\n"
			"QToolButton { background-color: #4a0e1c; color: #fef3c7; border: 1px solid #781c32; border-radius: 6px; padding: 6px 12px; font-weight: bold; }\n"
			"QToolButton:hover { background-color: #6b1428; border: 1px solid #d4af37; color: #ffffff; }\n"
			"QToolButton:checked { background-color: #6b1428; border: 2px solid #d4af37; color: #d4af37; }\n"
			"QMenuBar, QMenu { background-color: #380d18; color: #fef3c7; border: 1px solid #781c32; }\n"
			"QMenu::item:selected { background-color: #6b1428; color: #ffffff; }\n"
			"QPushButton { background-color: #6b1428; color: #fef3c7; border: 1px solid #991b1b; border-radius: 6px; padding: 6px 16px; font-weight: bold; }\n"
			"QPushButton:hover { background-color: #881337; border: 1px solid #d4af37; }\n"
			"QTableWidget, QListView, QTreeView { background-color: #240a11; color: #fef3c7; gridline-color: #4a0e1c; border: 1px solid #781c32; border-radius: 6px; }\n"
			"QHeaderView::section { background-color: #380d18; color: #d4af37; border: 1px solid #781c32; padding: 4px; font-weight: bold; }\n"
			"QGroupBox { border: 1px solid #781c32; border-radius: 8px; margin-top: 12px; font-weight: bold; color: #d4af37; }\n"
			"QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top right; padding: 0 6px; }\n"
			"QLineEdit, QComboBox { background-color: #2e0d16; color: #ffffff; border: 1px solid #781c32; border-radius: 6px; padding: 5px; }\n"
			"QStatusBar { background-color: #380d18; color: #d4af37; border-top: 1px solid #781c32; }\n"
			"QScrollBar:vertical { background: #380d18; width: 12px; }\n"
			"QScrollBar::handle:vertical { background: #6b1428; border-radius: 6px; }\n"
		);

	case Theme::ModernNajd:
		return QStringLiteral(
			"QMainWindow, QDialog, QWidget { background-color: #0b1120; color: #f1f5f9; font-family: 'Segoe UI', Tahoma, Arial; }\n"
			"QToolBar { background-color: #0f172a; border-bottom: 2px solid #f59e0b; spacing: 6px; padding: 4px; }\n"
			"QToolButton { background-color: #1e293b; color: #ffffff; border: 1px solid #334155; border-radius: 6px; padding: 6px 12px; font-weight: bold; }\n"
			"QToolButton:hover { background-color: #334155; border: 1px solid #f59e0b; color: #f59e0b; }\n"
			"QToolButton:checked { background-color: #1e293b; border: 2px solid #f59e0b; color: #f59e0b; }\n"
			"QMenuBar, QMenu { background-color: #0f172a; color: #f1f5f9; border: 1px solid #334155; }\n"
			"QMenu::item:selected { background-color: #1e293b; color: #f59e0b; }\n"
			"QPushButton { background-color: #d97706; color: #ffffff; border: 1px solid #b45309; border-radius: 6px; padding: 6px 16px; font-weight: bold; }\n"
			"QPushButton:hover { background-color: #f59e0b; }\n"
			"QTableWidget, QListView, QTreeView { background-color: #090e1a; color: #f8fafc; gridline-color: #1e293b; border: 1px solid #334155; border-radius: 6px; }\n"
			"QHeaderView::section { background-color: #0f172a; color: #f59e0b; border: 1px solid #334155; padding: 4px; font-weight: bold; }\n"
			"QGroupBox { border: 1px solid #334155; border-radius: 8px; margin-top: 12px; font-weight: bold; color: #f59e0b; }\n"
			"QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top right; padding: 0 6px; }\n"
			"QLineEdit, QComboBox { background-color: #1e293b; color: #ffffff; border: 1px solid #334155; border-radius: 6px; padding: 5px; }\n"
			"QStatusBar { background-color: #0f172a; color: #94a3b8; border-top: 1px solid #334155; }\n"
			"QScrollBar:vertical { background: #0f172a; width: 12px; }\n"
			"QScrollBar::handle:vertical { background: #d97706; border-radius: 6px; }\n"
		);

	case Theme::Default:
	default:
		return QString();
	}
}

bool ThemeManager::setTheme( Theme theme, QWidget* parent )
{
	if( theme != Theme::Default )
	{
		if( !LicenseManager::requireActivation( parent, QStringLiteral("المظهر والاستايل السعودي") ) )
		{
			return false;
		}
	}

	QSettings settings( QStringLiteral("Veyon"), QStringLiteral("UI") );
	settings.setValue( QStringLiteral("SelectedTheme"), static_cast<int>( theme ) );

	qApp->setStyleSheet( themeStylesheet( theme ) );
	return true;
}

void ThemeManager::applyCurrentTheme()
{
	Theme theme = currentTheme();
	if( theme != Theme::Default )
	{
		if( !LicenseManager::isActivated() )
		{
			// Reset to default if not activated
			theme = Theme::Default;
			QSettings settings( QStringLiteral("Veyon"), QStringLiteral("UI") );
			settings.setValue( QStringLiteral("SelectedTheme"), 0 );
		}
	}

	qApp->setStyleSheet( themeStylesheet( theme ) );
}

void ThemeManager::showThemeSelectionDialog( QWidget* parent )
{
	auto dlg = new QDialog( parent );
	dlg->setWindowTitle( QStringLiteral("🎨 اختيار المظهر والاستايل السعودي") );
	dlg->setMinimumWidth( 460 );

	auto layout = new QVBoxLayout( dlg );
	layout->setSpacing( 12 );

	auto header = new QLabel( QStringLiteral("<h3>🇸🇦 اختر مظهر واجهة البرنامج</h3>"
											"<p style='color:gray;'>اختر من بين 3 استايلات عربية سعودية فاخرة تعكس الهوية الوطنية.</p>"), dlg );
	layout->addWidget( header );

	auto group = new QGroupBox( QStringLiteral("الاستايلات المتاحة"), dlg );
	auto grpLayout = new QVBoxLayout( group );
	grpLayout->setSpacing( 10 );

	auto btnGroup = new QButtonGroup( dlg );

	auto rDefault = new QRadioButton( themeName( Theme::Default ), group );
	auto r2030 = new QRadioButton( themeName( Theme::Vision2030 ), group );
	auto rFounding = new QRadioButton( themeName( Theme::FoundingDay ), group );
	auto rNajd = new QRadioButton( themeName( Theme::ModernNajd ), group );

	btnGroup->addButton( rDefault, 0 );
	btnGroup->addButton( r2030, 1 );
	btnGroup->addButton( rFounding, 2 );
	btnGroup->addButton( rNajd, 3 );

	grpLayout->addWidget( rDefault );
	grpLayout->addWidget( r2030 );
	grpLayout->addWidget( rFounding );
	grpLayout->addWidget( rNajd );

	Theme cur = currentTheme();
	switch( cur )
	{
	case Theme::Vision2030: r2030->setChecked( true ); break;
	case Theme::FoundingDay: rFounding->setChecked( true ); break;
	case Theme::ModernNajd: rNajd->setChecked( true ); break;
	default: rDefault->setChecked( true ); break;
	}

	layout->addWidget( group );

	auto btnLayout = new QHBoxLayout();
	auto applyBtn = new QPushButton( QStringLiteral("تطبيق المظهر"), dlg );
	applyBtn->setStyleSheet( QStringLiteral("background-color: #059669; color: white; font-weight: bold; padding: 7px 16px; border-radius: 6px;") );
	auto cancelBtn = new QPushButton( QStringLiteral("إلغاء"), dlg );

	btnLayout->addStretch();
	btnLayout->addWidget( applyBtn );
	btnLayout->addWidget( cancelBtn );
	layout->addLayout( btnLayout );

	QObject::connect( cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject );
	QObject::connect( applyBtn, &QPushButton::clicked, [dlg, btnGroup]() {
		int id = btnGroup->checkedId();
		Theme selected = static_cast<Theme>( id );
		if( ThemeManager::setTheme( selected, dlg ) )
		{
			dlg->accept();
		}
	} );

	dlg->exec();
}
