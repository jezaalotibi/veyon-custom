/*
 * RemoteDeployFeaturePlugin.cpp - implementation of RemoteDeployFeaturePlugin
 */

#include "RemoteDeployFeaturePlugin.h"
#include "RemoteDeployDialog.h"
#include "LicenseManager.h"
#include "ComputerControlInterface.h"
#include "FeatureWorkerManager.h"
#include "PlatformCoreFunctions.h"
#include "VeyonMasterInterface.h"
#include "VeyonServerInterface.h"

RemoteDeployFeaturePlugin::RemoteDeployFeaturePlugin( QObject* parent ) :
	QObject( parent ),
	m_remoteDeployFeature( QStringLiteral("RemoteDeploy"),
						   Feature::Flag::Action | Feature::Flag::Master,
						   Feature::Uid( "a2198be0-9831-4a11-b021-3e479a29d10b" ),
						   {},
						   tr( "Remote Deploy Utility" ), tr( "Remote Deploy" ),
						   tr( "Remote network deployment utility for discovery and silent student installation" ),
						   QStringLiteral(":/remotedeploy/remotedeploy.png") ),
	m_features( { m_remoteDeployFeature } )
{
}

bool RemoteDeployFeaturePlugin::controlFeature( Feature::Uid featureUid, Operation operation,
												const QVariantMap& arguments,
												const ComputerControlInterfaceList& computerControlInterfaces )
{
	Q_UNUSED(featureUid)
	Q_UNUSED(operation)
	Q_UNUSED(arguments)
	Q_UNUSED(computerControlInterfaces)
	return false;
}

bool RemoteDeployFeaturePlugin::startFeature( VeyonMasterInterface& master,
											  const Feature& feature,
											  const ComputerControlInterfaceList& computerControlInterfaces )
{
	Q_UNUSED(computerControlInterfaces)

	if( feature.uid() == m_remoteDeployFeature.uid() )
	{
		// Strict Subscription Guard
		if( !LicenseManager::requireActivation( master.mainWindow(), tr("أداة النشر والتثبيت عن بعد للأجهزة") ) )
		{
			return false;
		}

		auto dialog = new RemoteDeployDialog( *this, master.mainWindow() );
		dialog->setAttribute( Qt::WA_DeleteOnClose );
		dialog->show();
		return true;
	}

	return false;
}

bool RemoteDeployFeaturePlugin::handleFeatureMessage( VeyonServerInterface& server,
													  const MessageContext& messageContext,
													  const FeatureMessage& message )
{
	Q_UNUSED(server)
	Q_UNUSED(messageContext)
	Q_UNUSED(message)
	return false;
}

bool RemoteDeployFeaturePlugin::handleFeatureMessage( VeyonWorkerInterface& worker, const FeatureMessage& message )
{
	Q_UNUSED(worker)
	Q_UNUSED(message)
	return false;
}
