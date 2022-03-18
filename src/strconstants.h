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

#ifndef STRCONSTANTS_H
#define STRCONSTANTS_H

#include "settingsmanager.h"

#include <QObject>
#include <QString>
#include <QDir>

class StrConstants{
public:
  static QString getApplicationName(){
    return "QTGZManager";
  }
  
  static QString getApplicationVersion(){
    return "1.1.0";
  }

  static QString getApplicationCliHelp(){
    QString str =
        "\n" + QObject::tr("QTGZManager help usage:") + "\n\n" +
        "-version: " + QObject::tr("show application version.") + "\n" +
        //"-style <Qt4-style>: " + QObject::tr("use a different Qt4 style (ex: -style gtk).") + "\n" +
        "-no-patch-download: " + QObject::tr("disable patch download from Slackware mirror.") +"\n";
    return str;
  }

  static QString getAbout(){
    return QObject::tr("About");
  }

  static QString getURL(){
    return QObject::tr("URL");
  }

  static QString getVersion(){
    return QObject::tr("Version");
  }

  static QString getQtVersion(){
    return "Qt " + QString(QT_VERSION_STR);
  }

  static QString getLicense(){
    return QObject::tr("License");
  }

  static QString getAttention(){
    return QObject::tr("Attention");
  }

  static QString getAutomaticSuCommand(){
    return QObject::tr("automatic");
  }

  static QString getPassword(){
    return QObject::tr("Password");
  }

  static QString getEnterAdministratorsPassword(){
    return QObject::tr(
          "Please, enter the administrator's password");
  }

  static QString getErrorNoSuCommand(){
    return
      QObject::tr("There are no means to get administrator's credentials.") + "\n" +
      QObject::tr("You'll need to run this application as root.");
  }

  static QString getErrorSpkgNotInstalled(){
    return QObject::tr("Spkg is not installed.");
  }

  static QString getExecutingCommand(){
    return QObject::tr("Executing command");
  }

  static QString getErrorAlreadyRunning(){
    return QObject::tr("There is already one instance of this application running!");
  }

  static QString getNeedsAppRestart(){
    return QObject::tr("Needs application restart to take effect");
  }

  static QString getWarnNeedsAppRestart(){
    return QObject::tr("These changes need application restart to take effect!");
  }

  static QString getForMoreInformation(){
    return "<small>" + QObject::tr("For more information about QTGZManager visit:") +
        " <a href=\"http://qtgzmanager.wordpress.com\" target=\"_blank\">" +
        "http://qtgzmanager.wordpress.com</a></small>";
  }

  static QString getUpdaterTabTitle(){
    return QObject::tr("Check for Slackware patches");
  }

  static QString getTodoInstallText(){
    return QObject::tr("To be installed");
  }

  static QString getTodoRemoveText(){
    return QObject::tr("To be removed");
  }

  static QString getTodoDowngradeText(){
    return QObject::tr("To be downgraded");
  }

  static QString getTodoUpgradeText(){
    return QObject::tr("To be upgraded");
  }

  static QString getTodoReinstallText(){
    return QObject::tr("To be reinstalled");
  }

  static QString getUserMirrorsFile(){
    return QDir::homePath() + QDir::separator() + ".config/QTGZManager/mirrors";
  }

  //Style Sheets ---------------------------------

  static QString getToolTipNormalCSS(){
    return QString("QToolTip {"
                   "background-color: #f7fcdc;"
                   "color: #000000;"
                   "border: 0.6px solid #337700;" //#effd72;"
                   "padding: 0.2px;"
                   "border-radius: 3px;"
                   "font-size: " + QString::number(SettingsManager::getToolTipFontSize()) + "px;}"
        );
  }

  static QString getToolTipBlankCSS(){
    return QString("QToolTip {"
                   "background-color: white;"
                   "color: #444444;"
                   "border: 0.5px solid #cccccc;"
                   "padding: 1.5px;"
                   "border-radius: 3px;"
                   "font-size: " + QString::number(SettingsManager::getToolTipFontSize()) + "px;}"
        );
  }

  static QString getToolTipRedCSS(){
    return QString("QToolTip {"
                   "background-color: #FF7A7A;" //"background-color: #FF8A8A;"
                   "color: #550000;"
                   "border: 0.6px solid #990000;"
                   "padding: 1.5px;"
                   "border-radius: 3px;"
                   "font-size: " + QString::number(SettingsManager::getToolTipFontSize()) + "px;}");
  }

  static QString getToolTipGreenCSS(){
    return QString("QToolTip {"
                   "background-color: #BDF4CB;"
                   "color: #000000;"
                   "border: 0.6px solid #006600;"
                   "padding: 1.5px;"
                   "border-radius: 3px;"
                   "font-size: " + QString::number(SettingsManager::getToolTipFontSize()) + "px;}"
                   );
  }

  static QString getToolTipBlueCSS(){
    return QString("QToolTip {"
                   "background-color: #CFECEC;"
                   "color: #000000;"
                   "border: 0.6px solid #3090C7;"
                   "padding: 1.5px;"
                   "border-radius: 3px;"
                   "font-size: " + QString::number(SettingsManager::getToolTipFontSize()) + "px;}"
                   );
  }

  static QString getToolTipYellowCSS(){
    return QString(" QToolTip {"
                   "background-color: #FFFF66;" //#FFCC00;"
                   "color: #000000;"
                   "border: 0.6px solid #FF9900;"
                   "padding: 1.5px;"
                   "border-radius: 3px;"
                   "font-size: " + QString::number(SettingsManager::getToolTipFontSize()) + "px;}"
                   );
  }

  static QString getToolTipRemovedCSS(){
    return QString(" QToolTip {"
                   "background-color: #FFFFDD;"
                   "color: #000000;"
                   "border: 0.6px solid #000000;"
                   "padding: 0.2px;"
                   "border-radius: 1px;"
                   "font-size: " + QString::number(SettingsManager::getToolTipFontSize()) + "px;}"
                   );
  }

  static QString getToolBarCSS(){
    return QString("QToolBar { border-bottom: 1px; border-top: 1px; } ");
    //               "QToolTip {}"
    //               );
  }

  static QString getFilterPackageNotFoundCSS(){
    return QString("QLineEdit{ color: white; "
                   "background-color: rgb(207, 135, 142);"
                   "border-color: rgb(206, 204, 197);}"
                   );
  }

  static QString getFilterPackageFoundCSS(){
    return QString("QLineEdit, SearchLineEdit{ color: black; "
                   "background-color: rgb(255, 255, 200);"
                   "border-color: rgb(206, 204, 197);}"
                   );
  }

  static QString getDockWidgetTitleCSS(){
    return QString("QDockWidget::title { "
                   "text-align: right;"
                   "background: transparent;"
                   "padding-right: 5px;}"
                   );
  }

  static QString getTabBarCSS(){
    return QString("QTabBar::close-button {"
                   "image: url(:/resources/images/window-close.png);"
                   "border-radius: 4px}"
                   "QTabBar::close-button:hover {"
                   "background-color: palette(light)}"
                   "QTabBar::close-button:pressed {"
                   "background-color: palette(mid)}"
                   );
  }

  static QString getTreeViewCSS(const int fontSize){
    return QString("QTreeView::branch:has-siblings:!adjoins-item {"
                   "   border-image: url(:/resources/styles/vline.png) 0;}"
                   "QTreeView::branch:has-siblings:adjoins-item {"
                   "    border-image: url(:/resources/styles/branch-more.png) 0;}"
                   "QTreeView::branch:!has-children:!has-siblings:adjoins-item {"
                   "   border-image: url(:/resources/styles/branch-end.png) 0;}"
                   "QTreeView::branch:has-children:!has-siblings:closed,"
                   "QTreeView::branch:closed:has-children:has-siblings {"
                   "       border-image: none;"
                   "        image: url(:/resources/styles/branch-closed_BW.png);}"
                   "QTreeView::branch:open:has-children:!has-siblings,"
                   "QTreeView::branch:open:has-children:has-siblings  {"
                   "       border-image: none;"
                   "       image: url(:/resources/styles/branch-open_BW.png);}"

                   "QTreeView {"
                       "selection-background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #6ea1f1, stop: 1 #567dbc);"
                       "show-decoration-selected: 1;"
                   "}"
                   "QTreeView::item {"
                       "border: 1px solid #d9d9d9;"
                       "border-top-color: transparent;"
                       "border-bottom-color: transparent;"
                   "}"
                   "QTreeView::item:hover {"
                       "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #e7effd, stop: 1 #cbdaf1);"
                       "border: 1px solid #bfcde4;"
                       "color: #000000;"
                   "}"
                   "QTreeView::item:selected {"
                       "border: 1px solid #567dbc;"
                   "}"
                   "QTreeView::item:selected:active{"
                       "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #6ea1f1, stop: 1 #567dbc)"
                   "}"
                   "QTreeView::item:selected:!active {"
                   "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #6b9be8, stop: 1 #577fbf)"
                   "}"
                   "QTreeView {"
                   "    font-family: \"Verdana\";"
                   "    font-size: " + QString::number(fontSize+5) + "px;"
                   "}"
                   );
  }
};

#endif // STRCONSTANTS_H
