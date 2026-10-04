/*
 * LicenseActivationDialog.h - declaration of LicenseActivationDialog
 */

#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class LicenseActivationDialog : public QDialog
{
	Q_OBJECT
public:
	explicit LicenseActivationDialog( QWidget* parent = nullptr );
	~LicenseActivationDialog() override = default;

private Q_SLOTS:
	void copyMachineId();
	void performOnlineActivation();
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
