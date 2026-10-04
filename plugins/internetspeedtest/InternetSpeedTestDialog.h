/*
 * InternetSpeedTestDialog.h - declaration of InternetSpeedTestDialog
 */

#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>

#include "ComputerControlInterface.h"
#include "FeatureMessage.h"

class InternetSpeedTestFeaturePlugin;

class InternetSpeedTestDialog : public QDialog
{
	Q_OBJECT
public:
	explicit InternetSpeedTestDialog( InternetSpeedTestFeaturePlugin& plugin,
									  const ComputerControlInterfaceList& computers,
									  QWidget* parent = nullptr );
	~InternetSpeedTestDialog() override = default;

	void updateResult( const QString& hostAddress, int pingMs, double downloadMbps );

private Q_SLOTS:
	void startTest();
	void exportReport();

private:
	void setupUi();
	void updateSummaryCards();

	InternetSpeedTestFeaturePlugin& m_plugin;
	ComputerControlInterfaceList m_computers;

	QLabel* m_avgDownloadLabel;
	QLabel* m_avgPingLabel;
	QLabel* m_slowestHostLabel;
	QTableWidget* m_tableWidget;
	QPushButton* m_startButton;
	QPushButton* m_exportButton;
};
