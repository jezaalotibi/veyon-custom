/*
 * InternetAccessFeaturePlugin.h - declaration of InternetAccessFeaturePlugin
 */

#pragma once

#include "Feature.h"
#include "FeatureProviderInterface.h"

class InternetAccessFeaturePlugin : public QObject, FeatureProviderInterface, PluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "io.veyon.Veyon.Plugins.InternetAccess")
	Q_INTERFACES(PluginInterface FeatureProviderInterface)
public:
	explicit InternetAccessFeaturePlugin( QObject* parent = nullptr );
	~InternetAccessFeaturePlugin() override = default;

	Plugin::Uid uid() const override
	{
		return Plugin::Uid{ QStringLiteral("c9823f41-0177-4b72-91f1-33291884a1e9") };
	}

	QVersionNumber version() const override
	{
		return QVersionNumber( 1, 0 );
	}

	QString name() const override
	{
		return QStringLiteral("InternetAccess");
	}

	QString description() const override
	{
		return tr( "Block and unblock internet access on client computers" );
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

	bool handleFeatureMessage( VeyonServerInterface& server,
							   const MessageContext& messageContext,
							   const FeatureMessage& message ) override;

private:
	enum class FeatureCommand {
		BlockInternet,
		UnblockInternet
	};

	static void applyInternetBlock( bool block );

	const Feature m_internetAccessFeature;
	const FeatureList m_features;
};
