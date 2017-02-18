/*
   Drawpile - a collaborative drawing program.

   Copyright (C) 2017 Calle Laakkonen

   Drawpile is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Drawpile is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include "localserver.h"
#include "remoteserver.h"
#include "multiserver.h"
#include "database.h"
#include "trayicon.h"
#include "singleinstance.h"
#include "authdialog.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QThread>
#include <QMessageBox>
#include <QMutex>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDir>
#include <QSystemTrayIcon>
#include <QSettings>
#include <QMenu>

namespace server {
namespace gui {

bool startServer()
{
	QString errorMessage;
	MultiServer *server=nullptr;

	// A database is always used with the GUI server
	QDir dbDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
	dbDir.mkpath(".");
	QString dbFile = QFileInfo(dbDir, "guiserver.db").absoluteFilePath();

	// The server must be initialized in its own thread
	QMutex mutex;
	auto threadInitFunc = [dbFile, &errorMessage, &server, &mutex]() {
		Database *db = new Database;
		if(!db->openFile(dbFile)) {
			errorMessage = QApplication::tr("Couldn't open database");
			mutex.unlock();
			return;
		}

		server = new MultiServer(db);
		db->setParent(server);

		mutex.unlock();
	};

	// Start a new thread for the server
	qDebug() << "Starting server thread";
	QThread *serverThread = new QThread;
	auto initConnetion = serverThread->connect(serverThread, &QThread::started, threadInitFunc);
	mutex.lock();
	serverThread->start();

	// Wait for initialization to complete
	mutex.lock();
	mutex.unlock();
	QObject::disconnect(initConnetion);
	qDebug() << "Server thread initialized";

	if(!errorMessage.isEmpty()) {
		QMessageBox::critical(nullptr, QApplication::tr("Drawpile Server"), errorMessage);
		return false;
	}

	Q_ASSERT(server);

	// The local server connector to use
	LocalServer *localServer = new LocalServer(server);

	QObject::connect(localServer, &LocalServer::serverError, [](const QString &error) {
		QMessageBox::warning(nullptr, QApplication::tr("Drawpile Server"), error);
	});

	// Create the system tray menu
	if(QSettings().value("ui/trayicon", true).toBool()) {
		TrayIcon::showTrayIcon();
	}

	QObject::connect(server, &MultiServer::userCountChanged, TrayIcon::instance(), &TrayIcon::setNumber);
	QObject::connect(localServer, &LocalServer::serverStateChanged, TrayIcon::instance(), &TrayIcon::setServerOn);

	// Open the main window
	MainWindow::setDefaultInstanceServer(localServer);
	MainWindow::showDefaultInstance();

	return true;
}

bool startRemote(const QString &address)
{
	QUrl url(address);
	if(!url.isValid()) {
		QMessageBox::critical(nullptr, QApplication::tr("Drawpile Server"), QApplication::tr("Invalid URL"));
		return false;
	}

	AuthDialog::init();

	RemoteServer *remote = new RemoteServer(url);

	MainWindow *win = new MainWindow(remote);
	remote->setParent(win);

	win->show();
	return true;
}

bool start() {
	// Set up command line arguments
	QCommandLineParser parser;

	parser.setApplicationDescription("Standalone server for Drawpile (graphical mode)");
	parser.addHelpOption();

	// --gui (this is just for the help text)
	QCommandLineOption guiOption(QStringList() << "gui", "Run the graphical version.");
	parser.addOption(guiOption);

	// remote <address>
	QCommandLineOption remoteOption(QStringList() << "remote", "Remote admin mode", "address");
	parser.addOption(remoteOption);

	// Parse
	parser.process(*QCoreApplication::instance());

	if(parser.isSet(remoteOption)) {
		// Remote admin mode
		return startRemote(parser.value(remoteOption));

	} else {
		// Normal server mode
		SingleInstance *guard = new SingleInstance;
		if(!guard->tryStart()) {
			qWarning("Another instance is already running");
			return false;
		}
		QObject::connect(qApp, &QApplication::aboutToQuit, guard, &SingleInstance::deleteLater);

#ifdef Q_OS_MAC
		qApp->setQuitOnLastWindowClosed(false);
#endif

		return startServer();
	}
}

}
}
