/*
 * BatchRenameFeaturePlugin.cpp - implementation of BatchRenameFeaturePlugin
 */

#include <QProcess>

#include "BatchRenameFeaturePlugin.h"
#include "BatchRenameDialog.h"
#include "LicenseManager.h"
#include "ComputerControlInterface.h"
#include "FeatureWorkerManager.h"
#include "PlatformCoreFunctions.h"
#include "VeyonMasterInterface.h"
#include "VeyonServerInterface.h"

BatchRenameFeaturePlugin::BatchRenameFeaturePlugin( QObject* parent ) :
	QObject( parent ),
	m_batchRenameFeature( QStringLiteral("BatchRename"),
						  Feature::Flag::Action | Feature::Flag::Master,
						  Feature::Uid( "c3841a10-2394-4d82-b7e1-8f479a32c10a" ),
						  {},
						  tr( "Batch Rename Devices" ), tr( "Batch Rename" ),
						  tr( "Sequentially rename Windows local accounts and computer hostnames for students." ),
						  QStringLiteral(":/batchrename/batchrename.png") ),
	m_features( { m_batchRenameFeature } )
{
}

bool BatchRenameFeaturePlugin::controlFeature( Feature::Uid featureUid, Operation operation,
											   const QVariantMap& arguments,
											   const ComputerControlInterfaceList& computerControlInterfaces )
{
	Q_UNUSED(featureUid)
	Q_UNUSED(operation)
	Q_UNUSED(arguments)
	Q_UNUSED(computerControlInterfaces)
	return false;
}

bool BatchRenameFeaturePlugin::startFeature( VeyonMasterInterface& master,
											 const Feature& feature,
											 const ComputerControlInterfaceList& computerControlInterfaces )
{
	if( feature.uid() == m_batchRenameFeature.uid() )
	{
		// Strict Subscription Guard
		if( !LicenseManager::requireActivation( master.mainWindow(), tr("تسمية وترقيم الأجهزة والحسابات تسلسلياً") ) )
		{
			return false;
		}

		auto dialog = new BatchRenameDialog( *this, computerControlInterfaces, master.mainWindow() );
		dialog->setAttribute( Qt::WA_DeleteOnClose );
		dialog->show();
		return true;
	}

	return false;
}

void BatchRenameFeaturePlugin::sendRenameCommand( ComputerControlInterface& computer,
												  const QString& targetName,
												  bool renameHostname,
												  const QString& passwordType,
												  const QString& password )
{
	FeatureMessage msg{ m_batchRenameFeature.uid(), FeatureCommand::ExecuteRename };
	msg.addArgument( QStringLiteral("targetName"), targetName );
	msg.addArgument( QStringLiteral("renameHostname"), renameHostname );
	msg.addArgument( QStringLiteral("passwordType"), passwordType );
	msg.addArgument( QStringLiteral("password"), password );

	sendFeatureMessage( msg, { &computer } );
}

bool BatchRenameFeaturePlugin::handleFeatureMessage( VeyonServerInterface& server,
													 const MessageContext& messageContext,
													 const FeatureMessage& message )
{
	Q_UNUSED(server)
	Q_UNUSED(messageContext)

	if( message.featureUid() == m_batchRenameFeature.uid() )
	{
		if( message.command<FeatureCommand>() == FeatureCommand::ExecuteRename )
		{
			QString targetName = message.argument<QString>( QStringLiteral("targetName") );
			bool renameHostname = message.argument<bool>( QStringLiteral("renameHostname") );
			QString passwordType = message.argument<QString>( QStringLiteral("passwordType") );
			QString password = message.argument<QString>( QStringLiteral("password") );

#ifdef Q_OS_WIN
			// 1. Rename Windows Local User if possible
			QString scriptUser = QStringLiteral(
				"$ErrorActionPreference = 'SilentlyContinue'; "
				"$cur = [Environment]::UserName; "
				"if ($cur -and $cur -ne '%1') { Rename-LocalUser -Name $cur -NewName '%1'; Set-LocalUser -Name '%1' -FullName '%1' }; "
				"wmic useraccount where name='$cur' rename '%1' 2>$null;"
			).arg( targetName );
			QProcess::execute( QStringLiteral("powershell.exe"), { QStringLiteral("-NoProfile"), QStringLiteral("-NonInteractive"), QStringLiteral("-Command"), scriptUser } );

			// 2. Set Password if requested
			if( passwordType == QStringLiteral("none") )
			{
				QProcess::execute( QStringLiteral("net.exe"), { QStringLiteral("user"), targetName, QStringLiteral("") } );
			}
			else if( passwordType == QStringLiteral("custom") && !password.isEmpty() )
			{
				QProcess::execute( QStringLiteral("net.exe"), { QStringLiteral("user"), targetName, password } );
			}

			// 3. Rename Hostname if requested
			if( renameHostname )
			{
				QString scriptHost = QStringLiteral(
					"$ErrorActionPreference = 'SilentlyContinue'; "
					"Rename-Computer -NewName '%1' -Force;"
				).arg( targetName.left( 15 ) );
				QProcess::execute( QStringLiteral("powershell.exe"), { QStringLiteral("-NoProfile"), QStringLiteral("-NonInteractive"), QStringLiteral("-Command"), scriptHost } );
			}
#endif
			return true;
		}
	}

	return false;
}

bool BatchRenameFeaturePlugin::handleFeatureMessage( VeyonWorkerInterface& worker, const FeatureMessage& message )
{
	Q_UNUSED(worker)
	Q_UNUSED(message)
	return false;
}
