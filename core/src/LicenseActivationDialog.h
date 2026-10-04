/*
 * LicenseActivationDialog.h - declaration of LicenseActivationDialog
 */

#pragma once

#include "VeyonCore.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class VEYON_CORE_EXPORT LicenseActivationDialog : public QDialog
{
	Q_OBJECT
public:
	explicit LicenseActivationDialog( QWidget* parent = nullptr );
	~LicenseActivationDialog() override = default;

private slots:
	void performOnlineActivation();
	void copyMachineId();
	void openWhatsApp();
	void deactiveLicense();

private:
	void setupUi();
	void updateStatusDisplay();

	QLineEdit* m_machineIdEdit;
	QLineEdit* m_licenseKeyEdit;
	QLabel* m_statusLabel;
	QLabel* m_expiryLabel;
	QLabel* m_seatsLabel;
	QPushButton* m_activateButton;
	QPushButton* m_deactivateButton;
};
