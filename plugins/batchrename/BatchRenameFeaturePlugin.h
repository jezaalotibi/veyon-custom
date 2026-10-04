/*
 * BatchRenameFeaturePlugin.h - declaration of BatchRenameFeaturePlugin
 */

#pragma once

#include "Feature.h"
#include "FeatureProviderInterface.h"
#include "FeatureMessage.h"

class BatchRenameFeaturePlugin : public QObject, FeatureProviderInterface, PluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "io.veyon.Veyon.Plugins.BatchRename")
	Q_INTERFACES(PluginInterface FeatureProviderInterface)
public:
	explicit BatchRenameFeaturePlugin( QObject* parent = nullptr );
	~BatchRenameFeaturePlugin() override = default;

	Plugin::Uid uid() const override
	{
		return Plugin::Uid{ QStringLiteral("c3841a10-2394-4d82-b7e1-8f479a32c10a") };
	}

	QVersionNumber version() const override
	{
		return QVersionNumber( 1, 0 );
	}

	QString name() const override
	{
		return QStringLiteral("BatchRename");
	}

	QString description() const override
	{
		return tr( "Batch rename and number student accounts and hostnames sequentially" );
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

	enum class FeatureCommand
	{
		ExecuteRename = 1,
		RenameResponse = 2
	};

	enum class Argument
	{
		TargetName = 0,
		RenameHostname,
		PasswordType,
		Password
	};

	void sendRenameCommand( ComputerControlInterface::Pointer computer,
							const QString& targetName,
							bool renameHostname,
							const QString& passwordType,
							const QString& password );

private:
	const Feature m_batchRenameFeature;
	const FeatureList m_features;
};
