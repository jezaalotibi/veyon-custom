/*
 * LicensingPlugin.cpp - implementation of LicensingPlugin
 */

#include "LicensingPlugin.h"
#include "LicenseActivationDialog.h"
#include "ThemeManager.h"
#include "VeyonMasterInterface.h"

LicensingPlugin::LicensingPlugin( QObject* parent ) :
	QObject( parent ),
	m_licensingFeature( QStringLiteral( "Licensing" ),
						Feature::Flag::Action | Feature::Flag::Master,
						Feature::Uid( "37e819b1-5244-4b53-8d48-69238914bda1" ),
						{},
						tr( "License Activation" ), tr( "License Activation" ),
						tr( "Activate and manage software license online via cloud." ),
						QStringLiteral(":/licensing/license.png") ),
	m_themeFeature( QStringLiteral( "SaudiThemes" ),
					Feature::Flag::Action | Feature::Flag::Master,
					Feature::Uid( "48f93a12-8172-4e09-92db-a5170d10b802" ),
					{},
					tr( "Saudi Themes" ), tr( "Saudi Themes" ),
					tr( "Select from 3 authentic Saudi Arabian UI themes (Vision 2030, Founding Day, Modern Najd)." ),
					QStringLiteral(":/licensing/license.png") ),
	m_features( { m_licensingFeature, m_themeFeature } )
{
}

bool LicensingPlugin::controlFeature( Feature::Uid featureUid, Operation operation,
									  const QVariantMap& arguments,
									  const ComputerControlInterfaceList& computerControlInterfaces )
{
	Q_UNUSED(featureUid)
	Q_UNUSED(operation)
	Q_UNUSED(arguments)
	Q_UNUSED(computerControlInterfaces)
	return false;
}

bool LicensingPlugin::startFeature( VeyonMasterInterface& master,
									const Feature& feature,
									const ComputerControlInterfaceList& computerControlInterfaces )
{
	Q_UNUSED(computerControlInterfaces)

	if( feature.uid() == m_licensingFeature.uid() )
	{
		auto dlg = new LicenseActivationDialog( master.mainWindow() );
		dlg->setAttribute( Qt::WA_DeleteOnClose );
		dlg->show();
		return true;
	}
	else if( feature.uid() == m_themeFeature.uid() )
	{
		ThemeManager::showThemeSelectionDialog( master.mainWindow() );
		return true;
	}

	return false;
}

bool LicensingPlugin::handleFeatureMessage( VeyonServerInterface& server,
											const MessageContext& messageContext,
											const FeatureMessage& message )
{
	Q_UNUSED(server)
	Q_UNUSED(messageContext)
	Q_UNUSED(message)
	return false;
}
