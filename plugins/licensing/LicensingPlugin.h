/*
 * LicensingPlugin.h - declaration of LicensingPlugin
 */

#pragma once

#include "Feature.h"
#include "FeatureProviderInterface.h"
#include "PluginInterface.h"

class LicensingPlugin : public QObject, FeatureProviderInterface, PluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "io.veyon.Veyon.Plugins.Licensing")
	Q_INTERFACES(PluginInterface FeatureProviderInterface)
public:
	explicit LicensingPlugin( QObject* parent = nullptr );
	~LicensingPlugin() override = default;

	Plugin::Uid uid() const override
	{
		return Plugin::Uid{ QStringLiteral("37e819b1-5244-4b53-8d48-69238914bda1") };
	}

	QVersionNumber version() const override
	{
		return QVersionNumber( 1, 0 );
	}

	QString name() const override
	{
		return QStringLiteral("Licensing");
	}

	QString description() const override
	{
		return tr( "Cloud online license activation and entitlement system" );
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

private:
	const Feature m_licensingFeature;
	const FeatureList m_features;
};
