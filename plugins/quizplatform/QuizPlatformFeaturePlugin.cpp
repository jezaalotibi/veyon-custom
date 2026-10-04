/*
 * QuizPlatformFeaturePlugin.cpp - implementation of QuizPlatformFeaturePlugin
 */

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSettings>
#include <QUrl>

#include "QuizPlatformFeaturePlugin.h"
#include "QuizPlatformDialog.h"
#include "ComputerControlInterface.h"
#include "FeatureWorkerManager.h"
#include "PlatformCoreFunctions.h"
#include "PlatformUserFunctions.h"
#include "VeyonMasterInterface.h"
#include "VeyonServerInterface.h"

QuizPlatformFeaturePlugin::QuizPlatformFeaturePlugin( QObject* parent ) :
	QObject( parent ),
	m_quizFeature( QStringLiteral( "QuizPlatform" ),
				   Feature::Flag::Action | Feature::Flag::Master,
				   Feature::Uid( "f9104882-7711-4fa9-8822-1209384812aa" ),
				   {},
				   tr( "Quizzes & Activities" ), tr( "Quizzes & Activities" ),
				   tr( "Create and dispatch interactive quizzes and exams to students with full lockdown security." ),
				   QStringLiteral(":/quizplatform/quiz.png") ),
	m_features( { m_quizFeature } )
{
}

bool QuizPlatformFeaturePlugin::controlFeature( Feature::Uid featureUid, Operation operation,
												const QVariantMap& arguments,
												const ComputerControlInterfaceList& computerControlInterfaces )
{
	Q_UNUSED(featureUid)
	Q_UNUSED(operation)
	Q_UNUSED(arguments)
	Q_UNUSED(computerControlInterfaces)
	return false;
}

bool QuizPlatformFeaturePlugin::startFeature( VeyonMasterInterface& master,
											  const Feature& feature,
											  const ComputerControlInterfaceList& computerControlInterfaces )
{
	if( feature.uid() == m_quizFeature.uid() )
	{
		auto dialog = new QuizPlatformDialog( *this, computerControlInterfaces, master.mainWindow() );
		dialog->setAttribute( Qt::WA_DeleteOnClose );
		dialog->show();
		return true;
	}

	return false;
}

void QuizPlatformFeaturePlugin::dispatchQuizToStudents( const ComputerControlInterfaceList& computers,
														const QString& url,
														const QString& title,
														bool lockdown,
														bool blockOtherSites )
{
	FeatureMessage msg{ m_quizFeature.uid(), FeatureCommand::StartQuizSession };
	msg.addArgument( Argument::QuizUrl, url );
	msg.addArgument( Argument::QuizTitle, title );
	msg.addArgument( Argument::LockdownMode, lockdown );
	msg.addArgument( Argument::BlockOtherSites, blockOtherSites );

	sendFeatureMessage( msg, computers );
}

bool QuizPlatformFeaturePlugin::handleFeatureMessage( VeyonServerInterface& server,
													  const MessageContext& messageContext,
													  const FeatureMessage& message )
{
	Q_UNUSED(messageContext)

	if( message.featureUid() == m_quizFeature.uid() )
	{
		// Forward message to user session worker
		server.featureWorkerManager().sendMessageToUnmanagedSessionWorker( message );
		return true;
	}

	return false;
}

bool QuizPlatformFeaturePlugin::handleFeatureMessage( VeyonWorkerInterface& worker, const FeatureMessage& message )
{
	Q_UNUSED(worker)

	if( message.featureUid() == m_quizFeature.uid() )
	{
		if( message.command<FeatureCommand>() == FeatureCommand::StartQuizSession )
		{
			QString urlString = message.argument( Argument::QuizUrl ).toString();
			bool lockdown = message.argument( Argument::LockdownMode ).toBool();

			QUrl url( urlString, QUrl::TolerantMode );
			if( url.scheme().isEmpty() )
			{
				url = QUrl( QStringLiteral("https://") + urlString, QUrl::TolerantMode );
			}

			if( lockdown )
			{
#ifdef Q_OS_WIN
				QString browser;
				const QStringList candidatePaths = {
					QStringLiteral("C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe"),
					QStringLiteral("C:\\Program Files\\Microsoft\\Edge\\Application\\msedge.exe"),
					QStringLiteral("C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe"),
					QStringLiteral("C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe")
				};
				for( const auto& candidate : candidatePaths )
				{
					if( QFile::exists( candidate ) )
					{
						browser = candidate;
						break;
					}
				}

				if( browser.isEmpty() )
				{
					QSettings edgeReg( QStringLiteral("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\msedge.exe"), QSettings::NativeFormat );
					browser = edgeReg.value( QStringLiteral(".") ).toString();
				}

				QString kioskDataDir = QDir::toNativeSeparators( QDir::tempPath() + QStringLiteral("/veyon_quiz_exam") );
				const QStringList kioskArgs = {
					QStringLiteral("--new-window"),
					QStringLiteral("--kiosk"),
					url.toString(),
					QStringLiteral("--edge-kiosk-type=fullscreen"),
					QStringLiteral("--user-data-dir=") + kioskDataDir,
					QStringLiteral("--no-first-run"),
					QStringLiteral("--disable-pinch"),
					QStringLiteral("--overscroll-history-navigation=0")
				};

				if( !browser.isEmpty() && QProcess::startDetached( browser, kioskArgs ) )
				{
					QCoreApplication::quit();
					return true;
				}

				QProcess::startDetached( QStringLiteral("cmd.exe"),
										 { QStringLiteral("/c"), QStringLiteral("start"),
										   QStringLiteral("msedge"), QStringLiteral("--new-window"),
										   QStringLiteral("--kiosk"), url.toString(),
										   QStringLiteral("--edge-kiosk-type=fullscreen"),
										   QStringLiteral("--user-data-dir=") + kioskDataDir } );
#else
				QProcess::startDetached( QStringLiteral("google-chrome"), { QStringLiteral("--kiosk"), QStringLiteral("--no-first-run"), url.toString() } );
#endif
			}
			else
			{
				QDesktopServices::openUrl( url );
			}

			QCoreApplication::quit();
			return true;
		}
	}

	return false;
}
