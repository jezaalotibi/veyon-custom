/*
 * QuizPlatformDialog.h - declaration of QuizPlatformDialog
 */

#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>

#include "ComputerControlInterface.h"

class QuizPlatformFeaturePlugin;

class QuizPlatformDialog : public QDialog
{
	Q_OBJECT
public:
	explicit QuizPlatformDialog( QuizPlatformFeaturePlugin& plugin,
								 const ComputerControlInterfaceList& computers,
								 QWidget* parent = nullptr );
	~QuizPlatformDialog() override = default;

private Q_SLOTS:
	void launchQuiz();

private:
	void setupUi();

	QuizPlatformFeaturePlugin& m_plugin;
	ComputerControlInterfaceList m_computers;

	QLineEdit* m_quizTitleEdit;
	QLineEdit* m_quizUrlEdit;
	QComboBox* m_durationCombo;
	QCheckBox* m_lockdownCheckBox;
	QCheckBox* m_showResultsCheckBox;
	QCheckBox* m_blockOtherSitesCheckBox;
	QPushButton* m_launchButton;
};
