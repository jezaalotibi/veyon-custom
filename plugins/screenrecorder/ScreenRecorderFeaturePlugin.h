/*
 * ScreenRecorderFeaturePlugin.h - declaration of ScreenRecorderFeaturePlugin class
 */

#pragma once

#include <QTimer>
#include <QMap>
#include "Feature.h"
#include "FeatureProviderInterface.h"
#include "ComputerControlInterface.h"

class ScreenRecorderFeaturePlugin : public QObject, FeatureProviderInterface, PluginInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "io.veyon.Veyon.Plugins.ScreenRecorder")
	Q_INTERFACES(PluginInterface FeatureProviderInterface)
public:
	explicit ScreenRecorderFeaturePlugin( QObject* parent = nullptr );
	~ScreenRecorderFeaturePlugin() override = default;

	Plugin::Uid uid() const override
	{
		return Plugin::Uid{ QStringLiteral("e84920bb-11fa-4f28-b801-4478129e924a") };
	}

	QVersionNumber version() const override
	{
		return QVersionNumber( 1, 0 );
	}

	QString name() const override
	{
		return QStringLiteral("ScreenRecorder");
	}

	QString description() const override
	{
		return tr( "Record screens of student computers to video or image sequence" );
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

	bool startFeature( VeyonMasterInterface& master, const Feature& feature,
					   const ComputerControlInterfaceList& computerControlInterfaces ) override;

private Q_SLOTS:
	void captureFrames();

private:
	struct RecordingSession
	{
		QString outputDir;
		int frameIndex{ 0 };
	};

	void startRecording( const ComputerControlInterfaceList& computerControlInterfaces );
	void stopRecording( const ComputerControlInterfaceList& computerControlInterfaces );

	QTimer* m_recordTimer{ nullptr };
	ComputerControlInterfaceList m_activeInterfaces;
	QMap<ComputerControlInterface::Pointer, RecordingSession> m_sessions;

	const Feature m_screenRecorderFeature;
	const FeatureList m_features;
};
