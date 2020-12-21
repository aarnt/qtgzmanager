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

#ifndef WMHELPER_H
#define WMHELPER_H

#include <QString>

enum EditOptions { ectn_EDIT_AS_ROOT, ectn_EDIT_AS_NORMAL_USER };

const QString ctn_OCTOPISUDO(QStringLiteral("/usr/lib/qtgzmanager/qtgzmanager-sudo"));
const QString ctn_OCTOPISUDO_PARAMS(QStringLiteral("-d"));
const QString ctn_NO_SU_COMMAND("none");

const QString ctn_ROOT_SH("/bin/sh -c");

const QString ctn_KDE_DESKTOP(QStringLiteral("kwin"));
const QString ctn_KDE_X11_DESKTOP(QStringLiteral("kwin_x11"));
const QString ctn_KDE_WAYLAND_DESKTOP(QStringLiteral("kwin_wayland"));
const QString ctn_KDE_EDITOR(QStringLiteral("kwrite"));
const QString ctn_KDE_FILE_MANAGER(QStringLiteral("kfmclient"));
const QString ctn_KDE_TERMINAL(QStringLiteral("konsole"));
const QString ctn_KDE4_OPEN(QStringLiteral("kde-open"));
const QString ctn_KDE5_OPEN(QStringLiteral("kde-open5"));
const QString ctn_KDE4_FILE_MANAGER(QStringLiteral("dolphin"));
const QString ctn_KDE4_EDITOR(QStringLiteral("kate"));

const QString ctn_TDESU(QStringLiteral("tdesu"));
const QString ctn_TDE_DESKTOP(QStringLiteral("twin"));
const QString ctn_TDE_EDITOR(QStringLiteral("kwrite"));
const QString ctn_TDE_FILE_MANAGER(QStringLiteral("kfmclient"));
const QString ctn_TDE_TERMINAL(QStringLiteral("konsole"));

const QString ctn_RXVT_TERMINAL(QStringLiteral("urxvt"));

const QString ctn_GNOME_DESKTOP(QStringLiteral("mutter"));
const QString ctn_GNOME_EDITOR(QStringLiteral("gedit"));
const QString ctn_GNOME_FILE_MANAGER(QStringLiteral("nautilus"));
const QString ctn_GNOME_TERMINAL(QStringLiteral("gnome-terminal"));

const QString ctn_XDG_OPEN(QStringLiteral("xdg-open"));

const QString ctn_XFCE_DESKTOP(QStringLiteral("xfdesktop"));
const QString ctn_XFCE_EDITOR(QStringLiteral("mousepad"));
const QString ctn_XFCE_EDITOR_ALT(QStringLiteral("leafpad"));
const QString ctn_XFCE_FILE_MANAGER(QStringLiteral("thunar"));
const QString ctn_XFCE_TERMINAL(QStringLiteral("xfce4-terminal"));

const QString ctn_OPENBOX_DESKTOP(QStringLiteral("openbox"));
const QString ctn_LXDE_DESKTOP(QStringLiteral("lxsession"));
const QString ctn_LXDE_TERMINAL(QStringLiteral("lxterminal"));
const QString ctn_LXDE_FILE_MANAGER(QStringLiteral("pcmanfm"));

const QString ctn_LUMINA_DESKTOP(QStringLiteral("lumina-desktop"));
const QString ctn_LUMINA_EDITOR(QStringLiteral("lumina-textedit"));
const QString ctn_LUMINA_FILE_MANAGER(QStringLiteral("lumina-fm"));
const QString ctn_LUMINA_OPEN(QStringLiteral("lumina-open"));

const QString ctn_LXQT_DESKTOP(QStringLiteral("lxqt-session"));
const QString ctn_LXQT_TERMINAL(QStringLiteral("qterminal"));
const QString ctn_LXQT_FILE_MANAGER(QStringLiteral("pcmanfm-qt"));
const QString ctn_LXQT_EDITOR(QStringLiteral("juffed"));

const QString ctn_MATE_DESKTOP(QStringLiteral("mate-session"));
const QString ctn_MATE_EDITOR(QStringLiteral("mate-open"));
const QString ctn_MATE_FILE_MANAGER(QStringLiteral("caja"));
const QString ctn_MATE_TERMINAL(QStringLiteral("mate-terminal"));

const QString ctn_CINNAMON_DESKTOP(QStringLiteral("cinnamon-session"));
const QString ctn_CINNAMON_EDITOR(QStringLiteral("gedit"));
const QString ctn_CINNAMON_FILE_MANAGER(QStringLiteral("nemo"));
const QString ctn_CINNAMON_TERMINAL(QStringLiteral("gnome-terminal"));

/* This class exposes some services of the underlying Window Manager being used */
class WMHelper
{
public:
  static bool isKDERunning();
  static bool isTDERunning();
  static bool isGNOMERunning();
  static bool isXFCERunning();
  static bool isOPENBOXRunning();
  static bool isLXDERunning();
  static bool isLXQTRunning();
  static bool isMATERunning();
  static bool isCinnamonRunning();
  static bool isLuminaRunning();

  static QString getKDEOpenHelper();
  static QString getOctopiSudoCommand();
  static QString getXFCEEditor();
  static QString getSUCommand();
  static QString getSUTool();

  static void openFile( const QString& fileName, const QString& package="" ); //fileName is Path + Name
  static void editFile( const QString& fileName, EditOptions opt = ectn_EDIT_AS_ROOT); //fileName is Path + Name
  static void openDirectory( const QString& dirName );
  static void openTerminal( const QString& dirName );
};

#endif // WMHELPER_H
