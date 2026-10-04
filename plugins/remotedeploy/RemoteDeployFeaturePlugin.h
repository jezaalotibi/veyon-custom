/*
 * RemoteDeployFeaturePlugin.h - declaration of RemoteDeployFeaturePlugin
 */

#pragma once

#include "Feature.h"
#include "FeatureProviderInterface.h"
#include "FeatureMessage.h"

class RemoteDeployFeaturePlugin : public QObject, FeatureProviderInterface, PluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "io.veyon.Veyon.Plugins.RemoteDeploy")
	Q_INTERFACES(PluginInterface FeatureProviderInterface)
public:
	explicit RemoteDeployFeaturePlugin( QObject* parent = nullptr );
	~RemoteDeployFeaturePlugin() override = default;

	Plugin::Uid uid() const override
	{
		return Plugin::Uid{ QStringLiteral("a2198be0-9831-4a11-b021-3e479a29d10b") };
	}

	QVersionNumber version() const override
	{
		return QVersionNumber( 1, 0 );
	}

	QString name() const override
	{
		return QStringLiteral("RemoteDeploy");
	}

	QString description() const override
	{
		return tr( "Remote network deployment utility for discovery and silent student installation" );
	}

	QString vendor() const override
	{
		return QStringLiteral("Veyon Community");
	}

	QString copyright() const override
	{
		return QStringLiteral("Custom Veyon Project");
	}

	const FeatureList& featureList() const override
	{
		return m_features;
	}

	bool controlFeature( Feature::Uid featureUid, Operation operation, const QVariantMap& arguments,
						const ComputerControlInterfaceList& computerControlInterfaces ) override;

	bool startFeature( VeyonMasterInterface& master,
					   const Feature& feature,
					   const ComputerControlInterfaceList& computerControlInterfaces ) override;

	bool handleFeatureMessage( VeyonServerInterface& server,
							   const MessageContext& messageContext,
							   const FeatureMessage& message ) override;

	bool handleFeatureMessage( VeyonWorkerInterface& worker, const FeatureMessage& message ) override;

private:
	const Feature m_remoteDeployFeature;
	const FeatureList m_features;
};
