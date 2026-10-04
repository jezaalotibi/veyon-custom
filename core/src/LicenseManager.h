/*
 * LicenseManager.h - declaration of LicenseManager
 */

#pragma once

#include "VeyonCore.h"

#include <QString>
#include <QJsonObject>
#include <functional>

class QWidget;

class VEYON_CORE_EXPORT LicenseManager
{
public:
	static QString getMachineId();
	static QString storedLicenseKey();
	static QString storedToken();
	static void saveLicense( const QString& key, const QString& token );
	static void clearLicense();

	struct VerificationResult
	{
		bool isValid{ false };
		QString message;
		QString expiryDate;
		int studentLimit{ 0 };
		QString schoolName;
	};

	static VerificationResult verifyLicense( const QString& token = QString() );
	static bool isActivated();

	static bool requireActivation( QWidget* parent = nullptr, const QString& featureName = QString() );

	static int failedAttempts();
	static qint64 lockoutRemainingSeconds();
	static void recordFailedAttempt();
	static void resetFailedAttempts();

	static void activateOnline( const QString& licenseKey,
								std::function<void( bool success, const QString& message, const VerificationResult& result )> callback );
};
