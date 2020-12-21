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

#include "tvpackagesitemdelegate.h"
#include "package.h"
#include "mainwindowimpl.h"
#include "strconstants.h"
#include "uihelper.h"

#include <QToolTip>
#include <QFutureWatcher>
#include <QTreeWidget>
#include <QtConcurrent/QtConcurrentRun>

QPoint gPoint;
QFutureWatcher<QString> fw;
using namespace QtConcurrent;

QString showPackageInfo(QString s, bool b){
	return(Package::getInformation(s, b));
}

tvPackagesItemDelegate::tvPackagesItemDelegate(QObject *parent): QStyledItemDelegate(parent){
  m_PkgClassification = ectn_INSTALLED;
}

Classification tvPackagesItemDelegate::getPackageClassification(QString pkgName){
  return Package::getStatus(pkgName).getClassification();
}

bool tvPackagesItemDelegate::helpEvent ( QHelpEvent *event, QAbstractItemView*, 
		const QStyleOptionViewItem&, const QModelIndex &index ){

  if ( ((this->parent()->objectName() == "tvPackage") ||
        (this->parent()->objectName() == "tvInstalledPackages")) && (index.isValid()) ) {
		QTreeView* tvPackages = qobject_cast<QTreeView*>(this->parent());
		QSortFilterProxyModel *sfp = qobject_cast<QSortFilterProxyModel*>(tvPackages->model());		
		QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(sfp->sourceModel());		

    if (sim->rowCount() == 0) return false;

		QModelIndex ind = sfp->mapToSource(index);
		QStandardItem *si = sim->itemFromIndex(ind);
		SelectedPackage sp;   
    QFileInfo fi;

    if (this->parent()->objectName() == "tvPackage"){
      sp = SelectedPackage( MainWindowImpl::returnMainWindow()->getSelectedDirectory(), si->text() );
      fi = QFileInfo(sp.getCompleteFileName());
      if (fi.size() < ctn_MAX_PACKAGE_SIZE_FOR_TOOLTIP)
        m_PkgClassification = getPackageClassification(sp.getFileName());
    }
    else{      
      sp = SelectedPackage( ctn_PACKAGES_DIR, si->text() );
      QFileInfo fi(sp.getCompleteFileName());
      if (fi.size() < ctn_MAX_PACKAGE_SIZE_FOR_TOOLTIP)
        m_PkgClassification = getPackageClassification(sp.getFileName());
    }
		
		gPoint = tvPackages->mapToGlobal(event->pos()); 
		QFuture<QString> f;
    if (this->parent()->objectName() == "tvPackage"){
      fi = QFileInfo(sp.getCompleteFileName());
      if (fi.size() < ctn_MAX_PACKAGE_SIZE_FOR_TOOLTIP)
        f = run(showPackageInfo, sp.getCompleteFileName(), false);
      else return true;
    }
		else f = run(showPackageInfo, sp.getCompleteFileName(), true);

    if (fi.size() < ctn_MAX_PACKAGE_SIZE_FOR_TOOLTIP){
      fw.setFuture(f);
      connect(&fw, SIGNAL(finished()), this, SLOT(execToolTip()));
    }
	}
  else if (this->parent()->objectName() == "twFindResults" && index.isValid()) {
    FindDialogImpl *findDialog = qobject_cast<FindDialogImpl*>(this->parent()->parent());
    QTreeWidget* twFind = qobject_cast<QTreeWidget*>(this->parent());
    QTreeWidgetItem* twItem = twFind->itemAt(event->pos());

    gPoint = twFind->mapToGlobal(event->pos());
    QFuture<QString> f;

    if (findDialog->getSearchPlace() == ectn_INSIDE_DIRECTORY){
      if (twItem->icon(0).pixmap(QSize(22,22)).toImage() != IconHelper::getIconFolder().pixmap(22,22).toImage() ){
        QString packageDirectory = twItem->parent()->text(0);
        QString packageName = twItem->text(0);

        QFileInfo fi(packageDirectory + QDir::separator() + packageName);
        if (fi.size() < ctn_MAX_PACKAGE_SIZE_FOR_TOOLTIP){
          m_PkgClassification = getPackageClassification(packageName);
          f = run(showPackageInfo, packageDirectory + QDir::separator() + packageName, false);
          fw.setFuture(f);
          connect(&fw, SIGNAL(finished()), this, SLOT(execToolTip()));
        }
      }
    }
    else if (findDialog->getSearchPlace() == ectn_INSIDE_INSTALLED_PACKAGES){
      if (twItem->icon(0).pixmap(QSize(22,22)).toImage() == IconHelper::getIconUnFrozen().pixmap(22,22).toImage()
          || twItem->icon(0).pixmap(QSize(22,22)).toImage() == IconHelper::getIconFrozen().pixmap(22,22).toImage()){

        QString packageDirectory = ctn_INSTALLED_PACKAGES_DIR;
        QString packageName = twItem->text(0);

        m_PkgClassification = getPackageClassification(packageName);

        f = run(showPackageInfo, packageDirectory + QDir::separator() + packageName, true);
        fw.setFuture(f);
        connect(&fw, SIGNAL(finished()), this, SLOT(execToolTip()));
      }
    }
  }
  else if (this->parent()->objectName() == "tvTODO" && index.isValid()){
    QTreeView* tvTodo = qobject_cast<QTreeView*>(this->parent());
    QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tvTodo->model());

    if (sim->rowCount() == 0) return false;

    QStandardItem *si = sim->itemFromIndex(index);
    gPoint = tvTodo->mapToGlobal(event->pos());
    QFuture<QString> f;
    QFileInfo fi (si->text());

    if (si->text().indexOf(ctn_PACKAGES_DIR) == 0){
      m_PkgClassification = ectn_INSTALLED;
      f = run(showPackageInfo, si->text(), true);
    }
    else{
      if (fi.size() < ctn_MAX_PACKAGE_SIZE_FOR_TOOLTIP){
        m_PkgClassification = getPackageClassification(fi.fileName());
        f = run(showPackageInfo, si->text(), false);
      }
    }

    if (fi.size() < ctn_MAX_PACKAGE_SIZE_FOR_TOOLTIP){
      fw.setFuture(f);
      connect(&fw, SIGNAL(finished()), this, SLOT(execToolTip()));
    }
  }

  return true;
}

void tvPackagesItemDelegate::execToolTip(){
  if (fw.result().isEmpty())
    return;

  gPoint.setX(gPoint.x() + 25);
	gPoint.setY(gPoint.y() + 25);

  if (m_PkgClassification == ectn_INSTALLED || m_PkgClassification == ectn_OTHER_ARCH )
    qApp->setStyleSheet(StrConstants::getToolTipNormalCSS());
  else if (m_PkgClassification == ectn_OTHER_VERSION)
    qApp->setStyleSheet(StrConstants::getToolTipYellowCSS());
  else if (m_PkgClassification == ectn_INFERIOR_VERSION)
    qApp->setStyleSheet(StrConstants::getToolTipRedCSS());
  else if (m_PkgClassification == ectn_SUPERIOR_VERSION)
    qApp->setStyleSheet(StrConstants::getToolTipGreenCSS());
  else if (m_PkgClassification == ectn_NOT_INSTALLED)
    qApp->setStyleSheet(StrConstants::getToolTipBlankCSS());
  else if (m_PkgClassification == ectn_FROZEN)
    qApp->setStyleSheet(StrConstants::getToolTipBlueCSS());

  QToolTip::showText(gPoint, fw.result());
}
