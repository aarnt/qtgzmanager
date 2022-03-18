/*
* This file is part of QTGZManager, an open-source GUI for Slackware pkgtools.
* Copyright (C) 2006  Alexandre Albuquerque Arnt
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact e-mail : Alexandre Albuquerque Arnt <qtgzmanager@gmail.com>
* Program URL   : http://jtgzmanager.sf.net
*
*/
#include <iostream>
#include "argumentlist.h"
#include "mainwindowimpl.h"
#include "unixcommand.h"
#include "strconstants.h"
#include "wmhelper.h"
#include "settingsmanager.h"
#include "QtSolutions/qtsingleapplication.h"

#include <QMessageBox>
#include <QTranslator>
#include <QResource>

int main(int argc, char * argv[])
{
  ArgumentList *argList = new ArgumentList(argc, argv);
  QtSingleApplication app( StrConstants::getApplicationName(), argc, argv );

  //This sends a message just to awake the socket-based QtSinleApplication engine
  app.sendMessage("ping app...");

  if (app.isRunning())
    return 0;

  QTranslator appTranslator;
  appTranslator.load(":/resources/translations/qtgzmanager_" +
    QLocale::system().name());
  app.installTranslator(&appTranslator);

  if (argList->getSwitch("-help")){
    std::cout << StrConstants::getApplicationCliHelp().toLatin1().data() << std::endl;
    return(0);
  }
  else if (argList->getSwitch("-version")){
    std::cout << StrConstants::getApplicationName().toLatin1().data() << " " <<
                 StrConstants::getApplicationVersion().toLatin1().data() << "\n" << std::endl;
    return(0);
  }

	MainWindowImpl win;

  app.setActivationWindow(&win);

  if (argList->getSwitch("-no-patch-download"))
    win.setDisablePatchDownload();

	win.show(); 
  QResource::registerResource("./resources.qrc");
  app.setQuitOnLastWindowClosed(!SettingsManager::getWindowCloseHidesApp());

  return app.exec();
}
