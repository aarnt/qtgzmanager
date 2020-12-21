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

#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include "ui_setupdialog.h"
#include <QDialog>

class SetupDialog : public QDialog, public Ui_SetupDialog
{
  Q_OBJECT

private:
  bool m_once;

  void initialize();
  void initButtonBox();
  void initCheckBoxes();
  void initComboPrivilege();
  void initFontSlider();
  void initGroupBox();

  void insertMirrorsInTable(QTextStream *stream, int row=0);
  void initMirrorTableWidget();

  void testMirrors();

protected:
  virtual void paintEvent(QPaintEvent *);

public:
  explicit SetupDialog(QWidget *parent = 0);
  void setFontSize(const int newValue);

signals:

private slots:

  void toggleSpinBoxHighlightedSearchItems(bool);
  void restoreDefaults(QAbstractButton*);
  void currentTabChanged(int tabIndex);
  virtual void accept();

};

#endif // SETUPDIALOG_H
