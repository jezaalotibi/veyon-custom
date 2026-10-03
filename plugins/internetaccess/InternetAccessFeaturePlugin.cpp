/*
 * InternetAccessFeaturePlugin.cpp - implementation of InternetAccessFeaturePlugin
 */

#include <QProcess>

#include "InternetAccessFeaturePlugin.h"
#include "ComputerControlInterface.h"
#include "FeatureMessage.h"
#include "VeyonServerInterface.h"
#include "VeyonCore.h"

InternetAccessFeaturePlugin::InternetAccessFeaturePlugin( QObject* parent ) :
	QObject( parent ),
	m_internetAccessFeature( QStringLiteral( "InternetAccess" ),
							 Feature::Flag::Mode | Feature::Flag::AllComponents,
							 Feature::Uid( "d32e9871-3311-4cb5-8291-a1b2c3d4e5f6" ),
							 Feature::Uid(),
							 tr( "Block Internet" ), tr( "Unblock Internet" ),
							 tr( "Click this button to block or unblock internet access on all student computers." ),
							 QStringLiteral( ":/internetaccess/internet-access-control.png" ) ),
	m_features( { m_internetAccessFeature } )
{
	// Clean up any stale block on service initialization
	if( VeyonCore::component() == VeyonCore::Component::Service )
	{
		connect( VeyonCore::instance(), &VeyonCore::initialized, this, []() {
			applyInternetBlock( false );
		} );
	}
}

bool InternetAccessFeaturePlugin::controlFeature( Feature::Uid featureUid, Operation operation,
												  const QVariantMap& arguments,
												  const ComputerControlInterfaceList& computerControlInterfaces )
{
	Q_UNUSED(arguments)

	if( featureUid == m_internetAccessFeature.uid() )
	{
		if( operation == Operation::Start )
		{
			sendFeatureMessage( FeatureMessage{ featureUid, FeatureCommand::BlockInternet }, computerControlInterfaces );
			return true;
		}
		if( operation == Operation::Stop )
		{
			sendFeatureMessage( FeatureMessage{ featureUid, FeatureCommand::UnblockInternet }, computerControlInterfaces );
			return true;
		}
	}

	return false;
}

bool InternetAccessFeaturePlugin::handleFeatureMessage( VeyonServerInterface& server,
														const MessageContext& messageContext,
														const FeatureMessage& message )
{
	Q_UNUSED(server)
	Q_UNUSED(messageContext)

	if( message.featureUid() == m_internetAccessFeature.uid() )
	{
		if( message.command<FeatureCommand>() == FeatureCommand::BlockInternet )
		{
			applyInternetBlock( true );
			return true;
		}
		if( message.command<FeatureCommand>() == FeatureCommand::UnblockInternet )
		{
			applyInternetBlock( false );
			return true;
		}
	}

	return false;
}

void InternetAccessFeaturePlugin::applyInternetBlock( bool block )
{
#ifdef Q_OS_WIN
	// Remove any existing rule first
	QProcess::execute( QStringLiteral("netsh"), { QStringLiteral("advfirewall"), QStringLiteral("firewall"),
												  QStringLiteral("delete"), QStringLiteral("rule"),
												  QStringLiteral("name=VeyonInternetBlock") } );

	if( block )
	{
		// Block outbound HTTP (80) and HTTPS (443) while allowing LAN traffic
		QProcess::execute( QStringLiteral("netsh"), { QStringLiteral("advfirewall"), QStringLiteral("firewall"),
													  QStringLiteral("add"), QStringLiteral("rule"),
													  QStringLiteral("name=VeyonInternetBlock"),
													  QStringLiteral("dir=out"), QStringLiteral("action=block"),
													  QStringLiteral("protocol=TCP"),
													  QStringLiteral("remoteport=80,443") } );
	}
#else
	if( block )
	{
		QProcess::execute( QStringLiteral("iptables"), { QStringLiteral("-I"), QStringLiteral("OUTPUT"),
														 QStringLiteral("-p"), QStringLiteral("tcp"),
														 QStringLiteral("-m"), QStringLiteral("multiport"),
														 QStringLiteral("--dports"), QStringLiteral("80,443"),
														 QStringLiteral("-j"), QStringLiteral("DROP") } );
	}
	else
	{
		QProcess::execute( QStringLiteral("iptables"), { QStringLiteral("-D"), QStringLiteral("OUTPUT"),
														 QStringLiteral("-p"), QStringLiteral("tcp"),
														 QStringLiteral("-m"), QStringLiteral("multiport"),
														 QStringLiteral("--dports"), QStringLiteral("80,443"),
														 QStringLiteral("-j"), QStringLiteral("DROP") } );
	}
#endif
}
