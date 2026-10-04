/*
 * BatchRenameDialog.h - declaration of BatchRenameDialog
 */

#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

#include "ComputerControlInterface.h"

class BatchRenameFeaturePlugin;

class BatchRenameDialog : public QDialog
{
	Q_OBJECT
public:
	explicit BatchRenameDialog( BatchRenameFeaturePlugin& plugin,
								const ComputerControlInterfaceList& computers,
								QWidget* parent = nullptr );
	~BatchRenameDialog() override = default;

	void handleRenameResult( const QString& hostAddress, bool success, const QString& message );

private:
	void setupUi();
	void startRenaming();
	void copyLog();
	void addLog( const QString& message, const QString& color = QStringLiteral("#1f2937") );

	BatchRenameFeaturePlugin& m_plugin;
	ComputerControlInterfaceList m_computers;

	QLineEdit* m_prefixEdit;
	QSpinBox* m_startNumberSpin;
	QCheckBox* m_renameHostnameCheck;
	QComboBox* m_passwordCombo;
	QLineEdit* m_customPasswordEdit;
	QPushButton* m_startButton;
	QPushButton* m_copyLogButton;
	QTextEdit* m_logTextEdit;
	QLabel* m_deviceCountLabel;
	QVBoxLayout* m_cardsLayout;

	struct DeviceCard
	{
		QString hostAddress;
		QString computerName;
		QString targetName;
		QLabel* statusBadge;
	};

	QList<DeviceCard> m_deviceCards;
	int m_currentIndex;
	bool m_isRunning;
};
