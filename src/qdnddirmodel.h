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

#ifndef QDNDDIRMODEL_H
#define QDNDDIRMODEL_H

#include <QDirModel>
#include <QFileSystemModel>
#include <QIcon>
#include <QFileIconProvider>

#include "uihelper.h"

class QDnDDirModel : public QFileSystemModel{
  Q_OBJECT

  public:
  	QDnDDirModel(QObject *parent);
    QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;
    Qt::ItemFlags flags(const QModelIndex index) const;
    bool dropMimeData(const QMimeData *data, 
      Qt::DropAction action, int row, int column, const QModelIndex &parent);
};

class QDnDDirModelIconProvider : public QFileIconProvider{
  private:
    QString m_initialDir;

  public:
    QDnDDirModelIconProvider(QString &initialDir):QFileIconProvider(), m_initialDir(initialDir){}
    virtual QIcon icon ( const QFileInfo & info ) const {
      QFileInfo fi(m_initialDir);
      if (info.absolutePath() == fi.absolutePath() &&
          info.fileName() == fi.fileName() &&
          info.filePath() == fi.filePath())
      {
        return IconHelper::getIconFavorites();
      }
      else if (info.isDir()){
        return IconHelper::getIconFolder();
      }
      else return QFileIconProvider::icon(info);
    }
};

#endif
