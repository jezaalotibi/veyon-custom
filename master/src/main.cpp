/*
 * main.cpp - startup routine for Veyon Master Application
 *
 * Copyright (c) 2004-2026 Tobias Junghans <tobydox@veyon.io>
 *
 * This file is part of Veyon - https://veyon.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QSplashScreen>

#include "DocumentationFigureCreator.h"
#include "VeyonMaster.h"
#include "MainWindow.h"
#include "PlatformCoreFunctions.h"
#include "PlatformPluginInterface.h"
#include "PlatformSessionFunctions.h"


int main( int argc, char** argv )
{
	VeyonCore::setupApplicationParameters();

	QApplication app( argc, argv );
	app.connect( &app, &QApplication::lastWindowClosed, &QApplication::quit );

	VeyonCore core( &app, VeyonCore::Component::Master, QStringLiteral("Master") );

	const auto sessionId = VeyonCore::platform().sessionFunctions().currentSessionId();
	const QString serverName = QStringLiteral("VeyonMaster_SingleInstance_%1")
								.arg( sessionId >= 0 ? sessionId : 0 );

	QLocalSocket socket;
	socket.connectToServer( serverName );
	if( socket.waitForConnected( 800 ) )
	{
		socket.write( "ACTIVATE\n" );
		socket.waitForBytesWritten( 800 );
		socket.disconnectFromServer();
		return 0;
	}

	QLocalServer::removeServer( serverName );
	auto localServer = new QLocalServer( &app );
	localServer->listen( serverName );

#ifdef VEYON_DEBUG
	if( qEnvironmentVariableIsSet( "VEYON_MASTER_CREATE_DOC_FIGURES") )
	{
		DocumentationFigureCreator().run();
		return 0;
	}
#endif

	QSplashScreen splashScreen( QPixmap( QStringLiteral(":/master/splash.png") ) );
	splashScreen.show();

	if( MainWindow::initAuthentication() == false ||
			MainWindow::initAccessControl() == false )
	{
		return -1;
	}

	VeyonMaster masterCore( &core );
	auto mainWindow = masterCore.mainWindow();

	QObject::connect( localServer, &QLocalServer::newConnection, [localServer, mainWindow]() {
		auto clientSocket = localServer->nextPendingConnection();
		if( clientSocket )
		{
			QObject::connect( clientSocket, &QLocalSocket::readyRead, [clientSocket, mainWindow]() {
				const auto data = clientSocket->readAll();
				if( data.contains( "ACTIVATE" ) && mainWindow )
				{
					if( mainWindow->isMinimized() )
					{
						mainWindow->showNormal();
					}
					mainWindow->show();
					mainWindow->raise();
					mainWindow->activateWindow();
					VeyonCore::platform().coreFunctions().raiseWindow( mainWindow, false );
				}
			} );
			QObject::connect( clientSocket, &QLocalSocket::disconnected, clientSocket, &QLocalSocket::deleteLater );
		}
	} );

	// hide splash-screen as soon as main-window is shown
	splashScreen.finish( mainWindow );

	mainWindow->show();

	return core.exec();
}

