/*
 * LicenseManager.h - declaration of LicenseManager
 */

#pragma once

#include <QString>
#include <QJsonObject>
#include <functional>

class LicenseManager
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
	};

	static VerificationResult verifyLicense( const QString& token = QString() );
	static bool isActivated();

	static void activateOnline( const QString& licenseKey,
								std::function<void( bool success, const QString& message, const VerificationResult& result )> callback );
};
