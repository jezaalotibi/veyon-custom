/*
 * QuizPlatformFeaturePlugin.h - declaration of QuizPlatformFeaturePlugin
 */

#pragma once

#include "Feature.h"
#include "FeatureProviderInterface.h"
#include "FeatureMessage.h"

class QuizPlatformFeaturePlugin : public QObject, FeatureProviderInterface, PluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "io.veyon.Veyon.Plugins.QuizPlatform")
	Q_INTERFACES(PluginInterface FeatureProviderInterface)
public:
	explicit QuizPlatformFeaturePlugin( QObject* parent = nullptr );
	~QuizPlatformFeaturePlugin() override = default;

	Plugin::Uid uid() const override
	{
		return Plugin::Uid{ QStringLiteral("f9104882-7711-4fa9-8822-1209384812aa") };
	}

	QVersionNumber version() const override
	{
		return QVersionNumber( 1, 0 );
	}

	QString name() const override
	{
		return QStringLiteral("QuizPlatform");
	}

	QString description() const override
	{
		return tr( "Interactive quiz platform and exam activities for students" );
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

	void dispatchQuizToStudents( const ComputerControlInterfaceList& computers,
								 const QString& url,
								 const QString& title,
								 bool lockdown,
								 bool blockOtherSites );

	enum class FeatureCommand
	{
		StartQuizSession
	};

	enum class Argument
	{
		QuizUrl,
		QuizTitle,
		LockdownMode,
		BlockOtherSites
	};

private:
	const Feature m_quizFeature;
	const FeatureList m_features;
};
