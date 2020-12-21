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

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>

class QTabBar;

//A redefined QTabWidget to override some methods.
class TabWidget : public QTabWidget{
  Q_OBJECT

  public:
    TabWidget(QWidget *parent=0):QTabWidget(parent){}

    QTabBar* getTabBar() const { return this->tabBar(); }

    void replaceTabBar(QTabBar *tabBar) { QTabWidget::setTabBar(tabBar); }

    void initTabBar();

    void removeTab(int index);
};

#endif // TABWIDGET_H
