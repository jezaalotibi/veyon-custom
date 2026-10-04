/*
 * InternetSpeedTestFeaturePlugin.h - declaration of InternetSpeedTestFeaturePlugin
 */

#pragma once

#include "Feature.h"
#include "FeatureProviderInterface.h"
#include "FeatureMessage.h"

class InternetSpeedTestDialog;

class InternetSpeedTestFeaturePlugin : public QObject, FeatureProviderInterface, PluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "io.veyon.Veyon.Plugins.InternetSpeedTest")
	Q_INTERFACES(PluginInterface FeatureProviderInterface)
public:
	explicit InternetSpeedTestFeaturePlugin( QObject* parent = nullptr );
	~InternetSpeedTestFeaturePlugin() override = default;

	Plugin::Uid uid() const override
	{
		return Plugin::Uid{ QStringLiteral("c2179831-2856-4ec8-b391-7603b12361ef") };
	}

	QVersionNumber version() const override
	{
		return QVersionNumber( 1, 0 );
	}

	QString name() const override
	{
		return QStringLiteral("InternetSpeedTest");
	}

	QString description() const override
	{
		return tr( "Test internet download speed and ping latency on student computers" );
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

	bool handleFeatureMessage( ComputerControlInterface::Pointer computerControlInterface,
							   const FeatureMessage& message ) override;

	void executeSpeedTest( const ComputerControlInterfaceList& computers, InternetSpeedTestDialog* dialog );

	enum class FeatureCommand
	{
		RunSpeedTest,
		SpeedTestResult
	};

	enum class Argument
	{
		PingMs,
		DownloadMbps
	};

private:
	const Feature m_speedTestFeature;
	const FeatureList m_features;

	InternetSpeedTestDialog* m_activeDialog{ nullptr };
};
