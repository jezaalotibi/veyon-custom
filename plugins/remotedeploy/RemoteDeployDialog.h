/*
 * RemoteDeployDialog.h - declaration of RemoteDeployDialog
 */

#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QProgressBar>
#include <QCheckBox>
#include <QTextEdit>
#include <QLabel>

class RemoteDeployFeaturePlugin;

class RemoteDeployDialog : public QDialog
{
	Q_OBJECT
public:
	explicit RemoteDeployDialog( RemoteDeployFeaturePlugin& plugin, QWidget* parent = nullptr );
	~RemoteDeployDialog() override = default;

private:
	void setupUi();
	void startNetworkScan();
	void startDeployment();
	void copyLog();
	void addLog( const QString& message, const QString& color = QStringLiteral("#1f2937") );
	void addDiscoveredDevice( const QString& ip, const QString& hostname, bool isOnline );

	RemoteDeployFeaturePlugin& m_plugin;

	QLineEdit* m_startIpEdit;
	QLineEdit* m_endIpEdit;
	QPushButton* m_scanButton;
	QProgressBar* m_scanProgressBar;

	QTableWidget* m_devicesTable;
	QPushButton* m_selectAllButton;
	QPushButton* m_deselectAllButton;

	QLineEdit* m_adminUserEdit;
	QLineEdit* m_adminPasswordEdit;
	QLineEdit* m_roomNameEdit;
	QCheckBox* m_studentServiceOnlyCheck;
	QCheckBox* m_importKeyCheck;

	QPushButton* m_deployButton;
	QPushButton* m_copyLogButton;
	QTextEdit* m_logTextEdit;

	bool m_isScanning;
	bool m_isDeploying;
};
