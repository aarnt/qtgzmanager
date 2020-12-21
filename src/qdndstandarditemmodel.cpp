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

#include "qdndstandarditemmodel.h"
#include "mainwindowimpl.h"
#include "package.h"
#include "uihelper.h"
#include "strconstants.h"

#include <QMessageBox>
#include <QMimeData>

QDnDStandardItemModel::QDnDStandardItemModel( QObject *parent )
	: QStandardItemModel(parent){}

QStringList QDnDStandardItemModel::mimeTypes() const {
	QStringList types;
	types << "application/vnd.text.list";
	return types;    
}

Qt::DropActions QDnDStandardItemModel::supportedDropActions() const {
  return (Qt::CopyAction) | (Qt::MoveAction);  
}

Qt::ItemFlags QDnDStandardItemModel::flags(QModelIndex index) const {
  Qt::ItemFlags defaultFlags = flags(index); 
	
	if (index.isValid())
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
	else 
		return Qt::ItemIsDropEnabled | defaultFlags;		
}

QMimeData* QDnDStandardItemModel::mimeData(const QModelIndexList &indexes) const{
  QMimeData *mimeData = new QMimeData();
  QByteArray encodedData;
  
  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  MainWindowImpl* w=0;
  for (QWidget *widget: QApplication::topLevelWidgets()) {
    if (widget->objectName() == "MainWindow") w = (MainWindowImpl*) widget;
  }  
  
  QDir d(w->getModelDir()->filePath(w->tvDir->currentIndex()));
	QString path = d.absolutePath() + QDir::separator();

  for(QModelIndex index: indexes){
    if ( (index.isValid()) && (index.column() == ctn_PACKAGE_NAME )){
      QString text = path + data(index, Qt::DisplayRole).toString();
      stream << text;      
    }    
  }  
  
  mimeData->setData("application/vnd.text.list", encodedData);
  return mimeData;
}

bool QDnDStandardItemModel::dropMimeData(const QMimeData *data, Qt::DropAction, 
  int, int, const QModelIndex &) {

  MainWindowImpl* w = 0;  
  for (QWidget *widget: QApplication::topLevelWidgets()) {
    if (widget->objectName() == "MainWindow") w = (MainWindowImpl*) widget;
  }  

  if (!data->hasFormat("application/vnd.text.list")) return false;

  QByteArray encodedData = data->data("application/vnd.text.list");
  QDataStream stream(&encodedData, QIODevice::ReadOnly);
  QStringList items;
  
  while (!stream.atEnd()) {
    QString text;
    stream >> text;
    items << text;
  }

  for (QString sourceFilePath: items){
    QString pkg = sourceFilePath.mid(sourceFilePath.lastIndexOf(QDir::separator()) + 1);
    QList<SelectedPackage> lsp = w->getSelectedPackage();
    SelectedPackage sp;

    for ( SelectedPackage l: lsp ){
      if ( l.getFileName() == pkg ){
        sp = l;
        break;
      }
    }

    QList<QStandardItem*> li2;
    QStandardItem *si;

    if (!lsp.isEmpty()){
      si = new QStandardItem(sourceFilePath);
      if ((sp.getIcon().pixmap(QSize(22,22)).toImage() == IconHelper::getIconSuperior().pixmap(QSize(22,22)).toImage()) ||
          (sp.getIcon().pixmap(QSize(22,22)).toImage() == IconHelper::getIconOtherVersion().pixmap(QSize(22,22)).toImage())	) {
        li2 = findItems(StrConstants::getTodoUpgradeText(), Qt::MatchStartsWith);

        if ((!li2.isEmpty()) &&
            (findItems(sourceFilePath, Qt::MatchRecursive).size()==0)
          &&(Package::isValid(sourceFilePath)))
          li2.at(0)->appendRow(si);
      }
      else if (sp.getIcon().pixmap(QSize(22,22)).toImage() == IconHelper::getIconInferior().pixmap(QSize(22,22)).toImage()) {
        li2 = findItems(StrConstants::getTodoDowngradeText(), Qt::MatchStartsWith);

        if ((!li2.isEmpty()) && (findItems(
            sourceFilePath, Qt::MatchRecursive).size()==0)
          &&(Package::isValid(sourceFilePath)))
          li2.at(0)->appendRow(si);
      }
      else if (sp.getIcon().pixmap(QSize(22,22)).toImage() ==
               IconHelper::getIconNotInstalled().pixmap(QSize(22,22)).toImage() ||
               sp.getIcon().pixmap(QSize(22,22)).toImage() ==
               IconHelper::getIconOtherArch().pixmap(QSize(22,22)).toImage()) {
        li2 = findItems(StrConstants::getTodoInstallText(), Qt::MatchStartsWith);

        if ((!li2.isEmpty()) && (findItems(
            sourceFilePath, Qt::MatchRecursive).size()==0)
          &&(Package::isValid(sourceFilePath)) && (Package::isSlackPackage(sourceFilePath)))
          li2.at(0)->appendRow(si);        
      }
      else if (sp.getIcon().pixmap(QSize(22,22)).toImage() == IconHelper::getIconInstalled().pixmap(QSize(22,22)).toImage()) {
        li2 = findItems(StrConstants::getTodoRemoveText(), Qt::MatchStartsWith);

        if ((!li2.isEmpty()) &&(findItems(
            sourceFilePath, Qt::MatchRecursive).size()==0)
          &&(Package::isValid(sourceFilePath))){
          li2.at(0)->appendRow(si);
        }
      }
    }    
  }

	if (!w->getTODOTreeView()->isExpanded(w->getTODOTreeView()->currentIndex())) w->getTODOTreeView()->expandAll();
			
	return true;		
}		
