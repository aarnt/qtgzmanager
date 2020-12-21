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

#include "qdnddirmodel.h"

#include <QApplication>
#include <QMimeData>
#include <QMessageBox>

QDnDDirModel::QDnDDirModel(QObject *parent): QFileSystemModel(parent){
  setReadOnly(false);
  setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
}

QStringList QDnDDirModel::mimeTypes() const {
	QStringList types;
	types << "application/vnd.text.list";
	return types;    
}

Qt::DropActions QDnDDirModel::supportedDropActions() const {
  return (Qt::CopyAction) | (Qt::MoveAction);  
}

Qt::ItemFlags QDnDDirModel::flags(QModelIndex index) const {
  Qt::ItemFlags defaultFlags = flags(index); 
	
	if (index.isValid())
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
	else 
		return Qt::ItemIsDropEnabled | defaultFlags;		
}
  
bool QDnDDirModel::dropMimeData(const QMimeData *data, Qt::DropAction action, 
                                int, int, const QModelIndex &parent) {

  QApplication::changeOverrideCursor(QCursor(Qt::ArrowCursor));

  if (action == Qt::IgnoreAction) return true;
  if (!data->hasFormat("application/vnd.text.list")) return false;
  QByteArray encodedData = data->data("application/vnd.text.list");
  QDataStream stream(&encodedData, QIODevice::ReadOnly);
  QStringList newItems;

  while (!stream.atEnd()) {
    QString text;
    stream >> text;
    newItems << text;
  }

	int res;
	if (newItems.size() > 1) 
		res = QMessageBox::question(0, 
			tr("Confirmation"), 
      tr("Are you sure you want to move these files?"),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::No);
	else if (newItems.size() == 1)
		res = QMessageBox::question(0, 
				tr("Confirmation"), 
				tr("Are you sure you want to move this file?"),  
				QMessageBox::Yes | QMessageBox::No,
				QMessageBox::No);
  else
    return false;
	
	if (res == QMessageBox::No) return false;
  
  QDir d(filePath(parent)); 				  
	QString path = d.absolutePath() + QDir::separator();
	
	//Let's move the files...
	QFileInfo fi;

  for(QString str: newItems){
		fi = QFileInfo(str);
		bool r = QFile::copy(str, path + fi.fileName());
		if (r) QFile::remove(str);
  }

  return true;		
}
