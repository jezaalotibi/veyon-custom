/*
 * ScreenRecorderFeaturePlugin.cpp - implementation of ScreenRecorderFeaturePlugin
 */

#include <QDateTime>
#include <QDir>
#include <QImage>
#include <QMessageBox>
#include <QProcess>

#include "ScreenRecorderFeaturePlugin.h"
#include "Filesystem.h"
#include "VeyonConfiguration.h"
#include "VeyonMasterInterface.h"

ScreenRecorderFeaturePlugin::ScreenRecorderFeaturePlugin( QObject* parent ) :
	QObject( parent ),
	m_screenRecorderFeature( Feature( QStringLiteral( "ScreenRecorder" ),
									  Feature::Flag::Mode | Feature::Flag::Master,
									  Feature::Uid( "b790d199-a411-4792-a1f9-906154e21a22" ),
									  Feature::Uid(),
									  tr( "Record Screen" ), tr( "Stop Recording" ),
									  tr( "Click to start or stop recording screens of selected student computers." ),
									  QStringLiteral(":/screenrecorder/media-record.png") ) ),
	m_features( { m_screenRecorderFeature } )
{
	m_recordTimer = new QTimer( this );
	m_recordTimer->setInterval( 500 ); // 2 frames per second
	connect( m_recordTimer, &QTimer::timeout, this, &ScreenRecorderFeaturePlugin::captureFrames );
}

bool ScreenRecorderFeaturePlugin::controlFeature( Feature::Uid featureUid,
												  Operation operation, const QVariantMap& arguments,
												  const ComputerControlInterfaceList& computerControlInterfaces )
{
	Q_UNUSED(arguments)

	if( featureUid == m_screenRecorderFeature.uid() )
	{
		if( operation == Operation::Start )
		{
			startRecording( computerControlInterfaces );
			return true;
		}
		if( operation == Operation::Stop )
		{
			stopRecording( computerControlInterfaces );
			return true;
		}
	}

	return false;
}

bool ScreenRecorderFeaturePlugin::startFeature( VeyonMasterInterface& master, const Feature& feature,
												const ComputerControlInterfaceList& computerControlInterfaces )
{
	Q_UNUSED(master)
	Q_UNUSED(feature)
	Q_UNUSED(computerControlInterfaces)
	return false;
}

void ScreenRecorderFeaturePlugin::startRecording( const ComputerControlInterfaceList& computerControlInterfaces )
{
	const QString baseDir = VeyonCore::filesystem().screenshotDirectoryPath() + QDir::separator() + QStringLiteral("Recordings");
	const QString timestamp = QDateTime::currentDateTime().toString( QStringLiteral("yyyy-MM-dd_HH-mm-ss") );

	for( const auto& iface : computerControlInterfaces )
	{
		QString host = iface->computer().hostName().isEmpty() ? iface->computer().hostAddress() : iface->computer().hostName();
		QString sessionDir = baseDir + QDir::separator() + QStringLiteral("%1_%2").arg( host, timestamp );
		QDir().mkpath( sessionDir );

		RecordingSession session;
		session.outputDir = sessionDir;
		session.frameIndex = 0;
		m_sessions.insert( iface, session );
	}

	m_activeInterfaces = computerControlInterfaces;
	if( !m_recordTimer->isActive() )
	{
		m_recordTimer->start();
	}
}

void ScreenRecorderFeaturePlugin::stopRecording( const ComputerControlInterfaceList& computerControlInterfaces )
{
	for( const auto& iface : computerControlInterfaces )
	{
		if( m_sessions.contains( iface ) )
		{
			auto session = m_sessions.take( iface );
			QString sessionDir = session.outputDir;
			QString mp4Path = sessionDir + QStringLiteral(".mp4");
			QProcess::startDetached( QStringLiteral("ffmpeg"),
									 { QStringLiteral("-y"), QStringLiteral("-framerate"), QStringLiteral("2"),
									   QStringLiteral("-i"), sessionDir + QDir::separator() + QStringLiteral("frame_%05d.jpg"),
									   QStringLiteral("-c:v"), QStringLiteral("libx264"), QStringLiteral("-pix_fmt"), QStringLiteral("yuv420p"),
									   mp4Path } );
		}
		m_activeInterfaces.removeAll( iface );
	}

	if( m_activeInterfaces.isEmpty() )
	{
		m_recordTimer->stop();
	}
}

void ScreenRecorderFeaturePlugin::captureFrames()
{
	for( const auto& iface : m_activeInterfaces )
	{
		if( !m_sessions.contains( iface ) || !iface->hasValidFramebuffer() )
		{
			continue;
		}

		auto& session = m_sessions[iface];
		QImage img = iface->framebuffer();
		if( !img.isNull() )
		{
			session.frameIndex++;
			QString frameName = QStringLiteral("frame_%1.jpg").arg( session.frameIndex, 5, 10, QLatin1Char('0') );
			img.save( session.outputDir + QDir::separator() + frameName, "JPG", 80 );
		}
	}
}
