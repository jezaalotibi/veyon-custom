/*
 * ThemeManager.h - declaration of ThemeManager
 */

#pragma once

#include "VeyonCore.h"

#include <QString>

class QWidget;

class VEYON_CORE_EXPORT ThemeManager
{
public:
	enum class Theme
	{
		Default = 0,
		Vision2030,
		FoundingDay,
		ModernNajd
	};

	static Theme currentTheme();
	static bool setTheme( Theme theme, QWidget* parent = nullptr );
	static void applyCurrentTheme();
	static QString themeName( Theme theme );
	static QString themeStylesheet( Theme theme );
	static void showThemeSelectionDialog( QWidget* parent = nullptr );
};
