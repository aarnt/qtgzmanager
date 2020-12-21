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

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QString>
#include <QVariant>
#include <QSettings>

const QString ctn_ORGANIZATION("QTGZManager");
const QString ctn_APPLICATION("QTGZManager");
const QString ctn_KEY_WINDOW_CLOSE_HIDES_APP("Window_Close_Hides_App");
const QString ctn_KEY_WINDOW_SIZE("Window_Size");
const QString ctn_KEY_FROZEN_PACKAGES_LIST("Frozen_Packages");
const QString ctn_KEY_DEFAULT_DIRECTORY("Default_Directory");
const QString ctn_KEY_UPDATER_DIRECTORY("Updater_Directory");
const QString ctn_KEY_UPDATER_MIRROR("Updater_Mirror");
const QString ctn_KEY_AUTOMATIC_CHECK_UPDATES("Automatic_Check_Updates");
const QString ctn_KEY_FOUR_PANEL_ORGANIZING("Four_Panel_Organizing");
const QString ctn_KEY_PACKAGE_LIST_ORDERED_COL("PackageList_Ordered_Col");
const QString ctn_KEY_INSTALLED_PACKAGE_LIST_ORDERED_COL("InstalledPackageList_Ordered_Col");
const QString ctn_KEY_PACKAGE_LIST_SORT_ORDER("PackageList_Sort_Order");
const QString ctn_KEY_INSTALLED_PACKAGE_LIST_SORT_ORDER("InstalledPackageList_Sort_Order");
const QString ctn_KEY_SHOW_TOOLBAR("Show_ToolBar");
const QString ctn_KEY_SHOW_STATUSBAR("Show_StatusBar");
const QString ctn_KEY_SHOW_PACKAGE_TOOLTIP("Show_Package_Tooltip");
const QString ctn_KEY_START_ICONIFIED("Start_Iconified");
const QString ctn_KEY_HIGHLIGHTED_SEARCH_ITEMS("Limit_Hightlighted_Search_Items");
const QString ctn_KEY_FONT_SIZE_FACTOR("Font_Size_Factor");
const QString ctn_KEY_USE_PKGTOOLS("Use_PkgTools");
const QString ctn_KEY_USE_SILENT_ACTION_OUTPUT("Use_Silent_Action_Output");
const QString ctn_KEY_PRIVILEGE_ESCALATION_TOOL("Privilege_Escalation_Tool");

enum FourPanelOrganizing { ectn_HIDE_RIGHT=1, ectn_NORMAL=2, ectn_HIDE_BOTTON=4, ectn_MAXIMIZED=6 };

class QStringList;

class SettingsManager {

  private:

    static SettingsManager *m_pinstance;
    QSettings *m_SYSsettings;

    SettingsManager(const SettingsManager&);
    SettingsManager& operator= (const SettingsManager&);
    SettingsManager();
    ~SettingsManager();

    QSettings* getSYSsettings() { return m_SYSsettings; }

  public:

    static SettingsManager* instance();
    static QStringList getFrozenPkgList();
    static QString getDefaultDirectory();
    static QString getUpdaterDirectory();
    static QString getUpdaterMirror();
    static QString getPrivilegeEscalationTool();

    static int getFourPanelOrganizing();
    static int getPackageListOrderedCol();
    static int getInstalledPackageListOrderedCol();
    static int getPackageListSortOrder();
    static int getInstalledPackageListSortOrder();
    static int getHighlightedSearchItems();
    static bool getShowToolBar();
    static bool getShowStatusBar();
    static bool getShowPackageTooltip();
    static bool getStartIconified();
    static bool getUsePkgTools();
    static bool getUseSilentActionOutput();
    static bool getAutomaticCheckUpdates();
    static bool getWindowCloseHidesApp();
    static int getFontSizeFactor();
    static int getPackagesInDirFontSize(){ return 8 + getFontSizeFactor(); }
    static int getInstalledPackagesFontSize() { return 8 + getFontSizeFactor(); }
    static int getDirectoryFontSize() { return 8 + getFontSizeFactor(); }
    static int getTodoFontSize() { return 8 + getFontSizeFactor(); }
    static int getPkgListFontSize() { return 8 + getFontSizeFactor(); }
    static int getFindTreeWidgetFontSize() { return 8 + getFontSizeFactor(); }
    static int getToolTipFontSize() { return 13 + getFontSizeFactor(); }
    static int getMenuFontSize() { return 12 + getFontSizeFactor(); }

    static QByteArray getWindowSize();

    static void setFrozenPkgList(QStringList newValue);
    static void setDefaultDirectory(QString newValue);
    static void setUpdaterDirectory(QString newValue);
    static void setUpdaterMirror(QString newValue);
    static void setAutomaticCheckUpdates(bool newValue);
    static void setWindowCloseHidesApp(bool newValue);
    static void setPrivilegeEscalationTool(QString newValue);
    static void setFourPanelOrganizing(int newValue);
    static void setPackageListOrderedCol(int newValue);
    static void setInstalledPackageListOrderedCol(int newValue);
    static void setPackageListSortOrder(int newValue);
    static void setInstalledPackageListSortOrder(int newValue);
    static void setShowToolBar(bool newValue);
    static void setShowStatusBar(bool newValue);
    static void setShowPackageTooltip(bool newValue);
    static void setStartIconified(bool newValue);
    static void setHighlightedSearchItems(int newValue);
    static void setUsePkgTools(bool newValue);
    static void setUseSilentActionOutput(bool newValue);
    static void setFontSizeFactor(int newValue);
    static void setWindowSize(QByteArray newValue);

    static void moveUpdaterDirContents();
};

#endif // SETTINGSMANAGER_H
