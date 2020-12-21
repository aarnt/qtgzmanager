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

#include "tabwidget.h"
#include "mainwindowimpl.h"
#include "strconstants.h"
#include <QTabBar>

void TabWidget::removeTab(int index){
  if (this->tabText(index).indexOf(StrConstants::getUpdaterTabTitle()) >= 0 &&
      MainWindowImpl::returnMainWindow()->isUpdaterRunning())
    return;
  else
    QTabWidget::removeTab(index);
}

void TabWidget::initTabBar(){
  tabBar()->setObjectName("tabBar");
  tabBar()->setTabButton(0, QTabBar::RightSide, 0);
  tabBar()->setTabButton(1, QTabBar::RightSide, 0);

  tabBar()->setStyleSheet(StrConstants::getTabBarCSS());
}
