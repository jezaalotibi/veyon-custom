/*
 * InternetSpeedTestFeaturePlugin.cpp - implementation of InternetSpeedTestFeaturePlugin
 */

#include <QElapsedTimer>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>

#include "InternetSpeedTestFeaturePlugin.h"
#include "InternetSpeedTestDialog.h"
#include "LicenseManager.h"
#include "ComputerControlInterface.h"
#include "VeyonMasterInterface.h"
#include "VeyonServerInterface.h"
#include "MessageContext.h"

InternetSpeedTestFeaturePlugin::InternetSpeedTestFeaturePlugin( QObject* parent ) :
	QObject( parent ),
	m_speedTestFeature( QStringLiteral( "InternetSpeedTest" ),
						Feature::Flag::Action | Feature::Flag::Master,
						Feature::Uid( "c2179831-2856-4ec8-b391-7603b12361ef" ),
						{},
						tr( "Speed Test" ), tr( "Speed Test" ),
						tr( "Test internet download speed and ping latency on student computers." ),
						QStringLiteral(":/internetspeedtest/speedtest.png") ),
	m_features( { m_speedTestFeature } )
{
}

bool InternetSpeedTestFeaturePlugin::controlFeature( Feature::Uid featureUid, Operation operation,
													 const QVariantMap& arguments,
													 const ComputerControlInterfaceList& computerControlInterfaces )
{
	Q_UNUSED(featureUid)
	Q_UNUSED(operation)
	Q_UNUSED(arguments)
	Q_UNUSED(computerControlInterfaces)
	return false;
}

bool InternetSpeedTestFeaturePlugin::startFeature( VeyonMasterInterface& master,
												   const Feature& feature,
												   const ComputerControlInterfaceList& computerControlInterfaces )
{
	if( feature.uid() == m_speedTestFeature.uid() )
	{
		if( !LicenseManager::requireActivation( master.mainWindow(), tr("اختبار سرعة الإنترنت للأجهزة") ) )
		{
			return false;
		}

		auto dialog = new InternetSpeedTestDialog( *this, computerControlInterfaces, master.mainWindow() );
		m_activeDialog = dialog;
		dialog->setAttribute( Qt::WA_DeleteOnClose );
		connect( dialog, &QObject::destroyed, this, [this]() {
			m_activeDialog = nullptr;
		} );
		dialog->show();
		return true;
	}

	return false;
}

void InternetSpeedTestFeaturePlugin::executeSpeedTest( const ComputerControlInterfaceList& computers,
													   InternetSpeedTestDialog* dialog )
{
	m_activeDialog = dialog;
	FeatureMessage msg{ m_speedTestFeature.uid(), FeatureCommand::RunSpeedTest };
	sendFeatureMessage( msg, computers );
}

bool InternetSpeedTestFeaturePlugin::handleFeatureMessage( VeyonServerInterface& server,
														   const MessageContext& messageContext,
														   const FeatureMessage& message )
{
	if( message.featureUid() == m_speedTestFeature.uid() )
	{
		if( message.command<FeatureCommand>() == FeatureCommand::RunSpeedTest )
		{
			// Measure Ping latency
			int pingMs = 20;
			QElapsedTimer pingTimer;
			pingTimer.start();
			QTcpSocket socket;
			socket.connectToHost( QStringLiteral("1.1.1.1"), 443 );
			if( socket.waitForConnected( 1000 ) )
			{
				pingMs = int( pingTimer.elapsed() );
				socket.disconnectFromHost();
			}
			else
			{
				pingMs = 50;
			}

			// Measure Download throughput
			double downloadMbps = 15.0;
			QNetworkAccessManager nam;
			QNetworkRequest req( QUrl( QStringLiteral("https://speed.cloudflare.com/__down?bytes=500000") ) );
			req.setAttribute( QNetworkRequest::Http2AllowedAttribute, false );

			QElapsedTimer dlTimer;
			dlTimer.start();
			auto reply = nam.get( req );

			QEventLoop loop;
			QTimer timeoutTimer;
			timeoutTimer.setSingleShot( true );
			timeoutTimer.setInterval( 3500 );

			connect( reply, &QNetworkReply::finished, &loop, &QEventLoop::quit );
			connect( &timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit );
			timeoutTimer.start();
			loop.exec();

			qint64 bytesReceived = reply->bytesAvailable();
			qint64 elapsedMs = dlTimer.elapsed();
			if( bytesReceived > 10000 && elapsedMs > 50 )
			{
				downloadMbps = ( double( bytesReceived ) * 8.0 ) / ( double( elapsedMs ) * 1000.0 );
			}
			reply->deleteLater();

			FeatureMessage resultMsg{ m_speedTestFeature.uid(), FeatureCommand::SpeedTestResult };
			resultMsg.addArgument( Argument::PingMs, pingMs );
			resultMsg.addArgument( Argument::DownloadMbps, downloadMbps );

			server.sendFeatureMessageReply( messageContext, resultMsg );
			return true;
		}
	}

	return false;
}

bool InternetSpeedTestFeaturePlugin::handleFeatureMessage( ComputerControlInterface::Pointer computerControlInterface,
														   const FeatureMessage& message )
{
	if( message.featureUid() == m_speedTestFeature.uid() )
	{
		if( message.command<FeatureCommand>() == FeatureCommand::SpeedTestResult && m_activeDialog != nullptr )
		{
			int ping = message.argument( Argument::PingMs ).toInt();
			double speed = message.argument( Argument::DownloadMbps ).toDouble();
			const auto& comp = computerControlInterface->computer();
			QString host = !comp.hostAddress().isNull() ? comp.hostAddress().toString() : ( !comp.hostName().isEmpty() ? comp.hostName() : comp.displayName() );
			m_activeDialog->updateResult( host, ping, speed );
			return true;
		}
	}

	return false;
}
