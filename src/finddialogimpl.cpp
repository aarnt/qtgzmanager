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

#include "finddialogimpl.h"
#include "packagecontroller.h"
#include "unixcommand.h"
#include "wmhelper.h"
#include "tvpackagesitemdelegate.h"
#include "strconstants.h"
#include "uihelper.h"
#include <iostream>

#include <QObject>
#include <QAction>
#include <QMutex>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>

FindDialogImpl::FindDialogImpl( QWidget *parent )
 : QDialog( parent ),
   m_packagePath(""),
   m_iconFile(":/resources/images/binary.png"),
   m_mutex(new QMutex()),
   m_stopGUIProcessing(false){

  setAttribute(Qt::WA_DeleteOnClose);
  setupUi(this);
  setMinimumWidth( 600 );
  setMinimumHeight( 400 );

  twFindResults->header()->setDefaultAlignment( Qt::AlignCenter );
  twFindResults->setContextMenuPolicy(Qt::CustomContextMenu);

  actionOpenFile = new QAction(m_iconFile, tr("Open file"), this);
  actionEditFile = new QAction(QIcon(":/resources/images/editfile.png"), tr("Edit file"), this);
  actionOpenFile->setIconVisibleInMenu(true);
  actionEditFile->setIconVisibleInMenu(true);

  m_qstandardItemModel = 0;
  bStop->setCursor(Qt::ArrowCursor);

  connect( bClose, SIGNAL(pressed()), this, SLOT(accept()) );
	connect( bStop, SIGNAL(pressed()), this, SLOT(stopFind()) );
	connect( bFind, SIGNAL(pressed()), this, SLOT(execFind()) );

  connect( twFindResults, SIGNAL(clicked(const QModelIndex)),
          twFindResults, SIGNAL(activated(const QModelIndex)));

  twFindResults->installEventFilter(this);

  if (SettingsManager::instance()->getShowPackageTooltip())
    twFindResults->setItemDelegate(new tvPackagesItemDelegate(twFindResults));

  twFindResults->setStyleSheet( StrConstants::getTreeViewCSS(SettingsManager::getFindTreeWidgetFontSize()) );
  show();
}

FindDialogImpl::~FindDialogImpl(){
  delete m_mutex;
  delete m_qstandardItemModel;
}

void FindDialogImpl::setPackagePath(const QString &newValue){
  if (!newValue.isEmpty()){
    m_packagePath = newValue;
    m_isInstalledPackage =
        (Package::getStatus(getPackageName()).getClassification() == ectn_INSTALLED);
  }
}

QString FindDialogImpl::getPackageName(){
  if (m_packagePath.isEmpty()) return "";
  else{
    QFileInfo info(m_packagePath);
    return info.fileName();
  }
}

void FindDialogImpl::setFontSize(const int fontSize){
  setStyleSheet("QCheckBox, QLabel, QTableWidget, QHeaderView, QPushButton {"
                "    font-family: \"Verdana\";"
                "    font-size: " + QString::number(fontSize+1) + "px;"
                "}"
                "QTabWidget {"
                "    border: 1px solid gray;"
                "    font-family: \"Verdana\";"
                "    font-size: " + QString::number(fontSize) + "px;"
                "}");
}

void FindDialogImpl::setSearchPlace(SearchPlace sp){
  m_searchPlace = sp;

  if (m_searchPlace == ectn_INSIDE_INSTALLED_PACKAGES){
    setWindowTitle( tr("Find a file in installed packages") );
    lblFileToFind->setText( tr("File to find") );
    twFindResults->setHeaderLabel( tr("0 files found") );

    connect( twFindResults, SIGNAL( itemActivated(QTreeWidgetItem*, int)), this,
      SLOT( positionInInstalledPkgList (QTreeWidgetItem *)));
    connect( twFindResults, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(execContextMenutwFindResults(QPoint)));
    connect( twFindResults, SIGNAL(itemDoubleClicked (QTreeWidgetItem*, int)),
             this, SLOT(canOpenFile(QTreeWidgetItem*)));
    connect(actionOpenFile, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(actionEditFile, SIGNAL(triggered()), this, SLOT(editFile()));
    ckbExactMatch->setCheckState(Qt::Unchecked);
  }
  else if (m_searchPlace == ectn_INSIDE_DIRECTORY){
    Q_ASSERT( m_targetDir != "" );

    setWindowTitle( tr("Find a package in directory \"%1\"").arg(m_targetDir) );
    lblFileToFind->setText( tr("Package to find") );
    twFindResults->setHeaderLabel( tr("0 packages found") );

    connect( twFindResults, SIGNAL( itemActivated ( QTreeWidgetItem *, int ) ), this,
      SLOT( positionInPkgList (QTreeWidgetItem *)));
    ckbExactMatch->setCheckState(Qt::Unchecked);
  }
  else if (m_searchPlace == ectn_INSIDE_QSTDITEMMODEL){
    Q_ASSERT( m_targetPackage != "" );

    if (m_targetDir.isEmpty())
      setWindowTitle(tr("Find a file in package \"%1\"").arg(m_targetPackage));
    else
      setWindowTitle(tr("Find a file in package \"%1\"").arg(m_targetPackage) + " (" + m_targetDir + ")");

    lblFileToFind->setText(tr("File to find") );
    twFindResults->setHeaderLabel(tr("0 files found") );

    connect( twFindResults, SIGNAL( itemActivated ( QTreeWidgetItem *, int ) ), this,
      SLOT( positionInPkgFileList (QTreeWidgetItem *)));
    connect( twFindResults, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(execContextMenutwFindResults(QPoint)));
    connect( twFindResults, SIGNAL(itemDoubleClicked (QTreeWidgetItem*, int)),
             this, SLOT(canOpenFile(QTreeWidgetItem*)));
    connect(actionOpenFile, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(actionEditFile, SIGNAL(triggered()), this, SLOT(editFile()));

    ckbExactMatch->setCheckState(Qt::Unchecked);
  }
}

void FindDialogImpl::setTargetDir(const QString &targetDir){
  m_targetDir = targetDir;
}

void FindDialogImpl::setTargetPackage(const QString &targetPackage){
  m_targetPackage = targetPackage;
}

void FindDialogImpl::setQStandardItemModel( const QStandardItemModel *sim, QStandardItem *sourceItem ){
  if (!sim) return;
  m_qstandardItemModel = new QStandardItemModel(this);
  QStandardItem *root = m_qstandardItemModel->invisibleRootItem();

  if (sourceItem != 0){
    QString aux = QDir::separator() + sourceItem->text();
    QString path1 = m_targetDir;

    //Maybe we should complete this directory...
    if (path1.indexOf(aux, 0) > 0){
      path1 = path1.remove(-aux.length(), aux.length());
      QStringList spath1 = path1.split("/", Qt::SkipEmptyParts);

      foreach(QString res, spath1){
        root->appendRow(new QStandardItem(res));
        root = root->child(0,0);
        root->setAccessibleDescription("directory");
      }
    }

    root->appendRow(sourceItem->clone());
    root = root->child(0,0);
  }

  for(int c=0;
      sourceItem==0 ? c < sim->rowCount(sim->invisibleRootItem()->index()) :
      c < sim->rowCount(sourceItem->index());
      c++){
    QStandardItem *it;
    if (sourceItem == 0)
      it = sim->item(c, 0);
    else
      it = sourceItem->child(c, 0);

    QStandardItem *clone = it->clone();
    clone->setAccessibleDescription(it->accessibleDescription());

    if (it->hasChildren())
      _copyChildItem(it, clone);

    root->appendRow(clone);
  }
}

bool FindDialogImpl::eventFilter(QObject *obj, QEvent *event){
  if (obj->objectName() == "twFindResults" && event->type() == QEvent::KeyPress){
    QTreeWidget *tv = qobject_cast<QTreeWidget *>(obj);
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) canOpenFile(tv->selectedItems()[0]);
  }

  return false;
}

void FindDialogImpl::showEvent(QShowEvent *){
	QDialog::show();
	leFileToFind->setFocus();
}

void FindDialogImpl::resetDialog(){
	leFileToFind->setText("");
  if (m_searchPlace == ectn_INSIDE_INSTALLED_PACKAGES) twFindResults->setHeaderLabel( tr("0 files found") );
  else if (m_searchPlace == ectn_INSIDE_DIRECTORY || m_searchPlace == ectn_INSIDE_QSTDITEMMODEL)
    twFindResults->setHeaderLabel( tr("0 packages found") );

  m_mapPkgFileList.clear();
  twFindResults->clear();
	leFileToFind->setFocus();
}

void FindDialogImpl::closeEvent(QCloseEvent * ce){
  if (bClose->isEnabled()){
    ce->accept();
  }
  else ce->ignore();
}

void FindDialogImpl::terminated(){
	setEnableFindButton(true);
  twFindResults->setHeaderLabel( tr("Search was canceled by user") );
}

void FindDialogImpl::setEnableFindButton(bool enableFindButton){
	bStop->setEnabled(!enableFindButton);
	bFind->setEnabled(enableFindButton);
  bClose->setEnabled(enableFindButton);
}

void FindDialogImpl::execFind(){
  if (!leFileToFind->hasFocus() && !bFind->hasFocus()) return;

  twFindResults->clear();
  qApp->processEvents();

  QString search(leFileToFind->text());

  if (m_searchPlace == ectn_INSIDE_INSTALLED_PACKAGES){
    if ( (search.isEmpty()) || (search.trimmed().isEmpty()) ) return;
    if ( search.trimmed().length() < MIN_LENGTH_SEARCH_STRING ){
      QMessageBox::warning( this, tr("Attention"),
                           tr("You should especify at least a %1 character word.").arg(MIN_LENGTH_SEARCH_STRING) );
      return;
    }
  }

  search = Package::parseSearchString(search, ckbExactMatch->checkState()!=0);
  setEnableFindButton(false);
	twFindResults->setHeaderLabels( QStringList( tr("Searching...")) );
	leFileToFind->setFocus();	

  m_ri = new CPUIntensiveComputing(this);

  m_tf = new ThreadFind();
  m_tf->setStringToSearch(search);
  m_tf->setSearchPlace(m_searchPlace);
  m_tf->setTargetDir(m_targetDir);
  m_tf->setQStandardItemModel(m_qstandardItemModel);

  //connect(m_tf, SIGNAL(terminated()), this, SLOT(terminated()));
  connect(m_tf, SIGNAL(finished()), this, SLOT(finishedSearch()));
  m_tf->start();
}

void FindDialogImpl::finishedSearch(){
  QStringList slItems;
	QList<QTreeWidgetItem *> items;
  QString search = Package::parseSearchString(leFileToFind->text(), ckbExactMatch->checkState()!=0);
  QMap<QString, QStringList> m;

  m = m_tf->getResultMap();
  if (m_searchPlace == ectn_INSIDE_QSTDITEMMODEL) m_mapPkgFileList = m;

  int findCount=0;
	QTreeWidgetItem *item, *parent;

  if (m_searchPlace == ectn_INSIDE_INSTALLED_PACKAGES){
    m_mutex->lock();
    foreach( QString k, m.keys() ){
      foreach( QString pkg, m.value(k) ){
        QFileInfo fi(pkg);
        if ( ((!fi.exists()) || (fi.isFile())) && ( fi.fileName().contains(QRegExp(search, Qt::CaseInsensitive))))
          findCount ++;
        else continue;

        QList<QTreeWidgetItem*> li = (twFindResults->findItems( k,
                                                                Qt::MatchRecursive|Qt::MatchExactly, 0 ) );
        if (li.count() == 0){
          slItems.clear();
          parent = new QTreeWidgetItem( (QTreeWidgetItem*) 0, slItems << k, 0 );

          if (FrozenPkgListSingleton::instance()->indexOf(Package::getBaseName(parent->text(0))) != -1)
            parent->setIcon(0, IconHelper::getIconFrozen());
          else parent->setIcon(0, IconHelper::getIconUnFrozen());

          items.append(parent);
        }
        else parent = li[0];

        twFindResults->addTopLevelItems(items);
        slItems.clear();
        item = new QTreeWidgetItem( parent, slItems << pkg , 0 );
        item->setIcon( 0, m_iconFile );
        items.append(item);
      }

      QCoreApplication::processEvents();
      if (m_stopGUIProcessing){
        m_mutex->unlock();
        stopGUIProcessing();
        return;
      }
    }
    m_mutex->unlock();
  }

  else if (m_searchPlace == ectn_INSIDE_DIRECTORY){
    m_mutex->lock();
    foreach( QString k, m.keys() ){
      foreach( QString pkg, m.value(k) ){
        if ( (pkg.contains(QRegExp(search, Qt::CaseInsensitive)))){
          findCount ++;
        }
        else continue;

        QList<QTreeWidgetItem*> li = (twFindResults->findItems(k, Qt::MatchRecursive, 0 ) );
        if (li.count() == 0)
        {
          slItems.clear();
          parent = new QTreeWidgetItem( (QTreeWidgetItem*) 0, slItems << k, 0 );
          parent->setIcon(  0, IconHelper::getIconFolder());
          items.append(parent);
        }
        else parent = li[0];

        twFindResults->addTopLevelItems(items);
        slItems.clear();
        item = new QTreeWidgetItem( parent, slItems << pkg , 0 );

        Result res = Package::getStatus(pkg);

        switch (res.getClassification()) {
        case ectn_RPM:
          item->setIcon( 0, IconHelper::getIconRPM());
          break;
        case ectn_FROZEN :
          item->setIcon( 0, IconHelper::getIconFrozen());
          break;

        case ectn_INTERNAL_ERROR :
          item->setIcon( 0, IconHelper::getIconInternalError());
          break;

        case ectn_INFERIOR_VERSION :
          item->setIcon( 0, IconHelper::getIconInferior());
          break;

        case ectn_SUPERIOR_VERSION:
          item->setIcon( 0, IconHelper::getIconSuperior());
          break;

        case ectn_OTHER_VERSION:
          item->setIcon( 0, IconHelper::getIconOtherVersion());
          break;

        case ectn_OTHER_ARCH:
          item->setIcon( 0, IconHelper::getIconOtherArch());
          break;

        case ectn_INSTALLED :
          item->setIcon( 0, IconHelper::getIconInstalled());
          break;

        case ectn_NOT_INSTALLED :
          item->setIcon( 0, IconHelper::getIconNotInstalled());
          item->setForeground( 0, Qt::darkGray);

          break;

        default:
          item->setIcon( 0, IconHelper::getIconInternalError());
        }
      }

      QCoreApplication::processEvents();
      if (m_stopGUIProcessing){
        m_mutex->unlock();
        stopGUIProcessing();
        return;
      }
    }
    m_mutex->unlock();
  }
  else if (m_searchPlace == ectn_INSIDE_QSTDITEMMODEL){
    m_mutex->lock();
    foreach( QString k, m.keys() ){
      foreach( QString pkg, m.value(k) ){
        if ( (pkg.contains(QRegExp(search, Qt::CaseInsensitive)))){
          findCount ++;
        }
        else continue;

        QList<QTreeWidgetItem*> li = (twFindResults->findItems(k, Qt::MatchRecursive, 0 ) );
        if (li.count() == 0)
        {
          slItems.clear();
          parent = new QTreeWidgetItem( (QTreeWidgetItem*) 0, slItems << k, 0 );
          parent->setIcon( 0, IconHelper::getIconFolder() );
          items.append(parent);
        }
        else parent = li[0];

        twFindResults->addTopLevelItems(items);

        slItems.clear();
        item = new QTreeWidgetItem( parent, slItems << pkg , 0 );
        item->setIcon( 0, m_iconFile );
      }

      QCoreApplication::processEvents();
      if (m_stopGUIProcessing){
        m_mutex->unlock();
        stopGUIProcessing();
        return;
      }
    }
    m_mutex->unlock();
  }
  if ( findCount == 1 ){
    if (m_searchPlace == ectn_INSIDE_INSTALLED_PACKAGES || m_searchPlace == ectn_INSIDE_QSTDITEMMODEL)
      twFindResults->setHeaderLabel( tr("1 file found") );
    else if (m_searchPlace == ectn_INSIDE_DIRECTORY) twFindResults->setHeaderLabel( tr("1 package found") );
  }
  else if ( findCount > 1 ){
    if (m_searchPlace == ectn_INSIDE_INSTALLED_PACKAGES || m_searchPlace == ectn_INSIDE_QSTDITEMMODEL)
      twFindResults->setHeaderLabel( tr("%1 files found").arg( findCount ) );
    else if (m_searchPlace == ectn_INSIDE_DIRECTORY)
      twFindResults->setHeaderLabel( tr("%1 packages found").arg( findCount ) );
  }
  else if ( findCount == 0 ){
    if (twFindResults->headerItem()->text(0) != tr("Search was canceled by user") &&
        (m_searchPlace == ectn_INSIDE_INSTALLED_PACKAGES || m_searchPlace == ectn_INSIDE_QSTDITEMMODEL))
      twFindResults->setHeaderLabel( tr("0 files found") );
    else if (twFindResults->headerItem()->text(0) != tr("Search was canceled by user") &&
             m_searchPlace == ectn_INSIDE_DIRECTORY)
      twFindResults->setHeaderLabel( tr("0 packages found") );
  }

  twFindResults->expandAll();
  if ( findCount != 0 ){
    twFindResults->setFocus();
    twFindResults->invisibleRootItem()->child(0)->setSelected(true);

    if (m_searchPlace == ectn_INSIDE_INSTALLED_PACKAGES)
      positionInInstalledPkgList( twFindResults->invisibleRootItem()->child(0) );
  }

	delete m_ri;
  if (!m_tf.isNull()) m_tf->freeGarbage();
  delete m_tf;
  m.clear();
	setEnableFindButton(true);
}

void FindDialogImpl::stopFind(){
  if (m_searchPlace == ectn_INSIDE_INSTALLED_PACKAGES){
    if (m_tf->isRunning()){
      m_tf->wait(1000);
      m_tf->terminate();
      m_tf->wait(1000);
    }
    else m_stopGUIProcessing = true;
  }
  else m_stopGUIProcessing = true;
}

void FindDialogImpl::stopGUIProcessing(){
  m_stopGUIProcessing = false;
  delete m_ri;
  if (!m_tf.isNull())
    m_tf->freeGarbage();
  delete m_tf;
  setEnableFindButton(true);
  twFindResults->setHeaderLabel( tr("Search was canceled by user") );
  twFindResults->expandAll();
}

//In response to a click at an installed package, we emit a signal
void FindDialogImpl::positionInInstalledPkgList(QTreeWidgetItem* item){
  if ((twFindResults->invisibleRootItem()->childCount()>0)){
		if (item->parent() != 0)
      emit installedPkgSelectedInFind(item->parent()->text(0));
		else
      emit installedPkgSelectedInFind(item->text(0));
	}
}

//In response to a click at a package inside a directory, we emit a signal
void FindDialogImpl::positionInPkgList(QTreeWidgetItem* item){
  if ((twFindResults->invisibleRootItem()->childCount()>0)){
    if (item->parent() != 0){
      emit packageInsideDirSelectedInFind(item->parent()->text(0), item->text(0));
    }
  }
}

//In response to a click at an installed package, we emit a signal
void FindDialogImpl::positionInPkgFileList (QTreeWidgetItem *item){
  if ((twFindResults->invisibleRootItem()->childCount()>0)){
    if (item->icon(0).pixmap(QSize(22,22)).toImage() !=
        IconHelper::getIconFolder().pixmap(QSize(22,22)).toImage()){

      QStringList slmi = m_mapPkgFileList.value(item->parent()->text(0));
      int i = slmi.indexOf(item->text(0));
      QString directory = item->parent()->text(0);
      emit fileInsidePkgSelectedInFind(slmi.at(i), directory);
    }
  }
}

void FindDialogImpl::keyPressEvent(QKeyEvent* ke){
	if (ke->key() == Qt::Key_Escape) {
    if ((m_tf) && (m_tf->isRunning())) stopFind();
    else if(m_mutex->tryLock())
      close();
    else
      m_stopGUIProcessing = true;
	}
  else if (ke->key() == Qt::Key_F3 && ke->modifiers() == Qt::ShiftModifier &&
           twFindResults->invisibleRootItem()->childCount() >0)
    iterateOverFoundItems(ectn_ITERATE_BACKWARDS);
  else if (ke->key() == Qt::Key_F3 &&
           twFindResults->invisibleRootItem()->childCount() >0)
    iterateOverFoundItems(ectn_ITERATE_AFTERWARDS);
  else if (ke->key() == Qt::Key_F && ke->modifiers() == Qt::ControlModifier){
    leFileToFind->clear();
    leFileToFind->setFocus();
  }
  else QDialog::keyPressEvent(ke);
}

void FindDialogImpl::iterateOverFoundItems(IterationMode im){
  twFindResults->setFocus();
  if (im == ectn_ITERATE_AFTERWARDS){
    QModelIndex mi = twFindResults->indexBelow(twFindResults->currentIndex());
    if (mi.isValid())
      twFindResults->setCurrentIndex(mi);
    else{
      QTreeWidgetItem *it = twFindResults->invisibleRootItem();
      if (it->childCount() > 0){
        twFindResults->setCurrentItem(it->child(0));
      }
    }
  }
  else if (im == ectn_ITERATE_BACKWARDS){
    QModelIndex mi = twFindResults->indexAbove(twFindResults->currentIndex());
    if (mi.isValid())
      twFindResults->setCurrentIndex(mi);
    else{      
      QTreeWidgetItem *it = twFindResults->invisibleRootItem();
      if (it->childCount() > 0){
        int lastItem = twFindResults->invisibleRootItem()->childCount()-1;
        QTreeWidgetItem *it2 = twFindResults->invisibleRootItem()->child(lastItem);
        if (it2->childCount() > 0)
          twFindResults->setCurrentItem(it2->child(it2->childCount()-1));
        else
          twFindResults->setCurrentItem(it->child(lastItem));
      }
    }
  }
}

void FindDialogImpl::keyReleaseEvent(QKeyEvent* ke){
	if ((ke->key() != Qt::Key_Shift) && (ke->key() != Qt::Key_Control) && 
    (twFindResults->hasFocus() &&
     (ke->key() == Qt::Key_Down || ke->key() == Qt::Key_Up ||
      ke->key() == Qt::Key_PageDown || ke->key() == Qt::Key_PageUp)))
  {
    if (m_searchPlace == ectn_INSIDE_INSTALLED_PACKAGES)
      positionInInstalledPkgList( twFindResults->currentItem() );
    else if (m_searchPlace == ectn_INSIDE_DIRECTORY)
      positionInPkgList( twFindResults->currentItem() );
    else if (m_searchPlace == ectn_INSIDE_QSTDITEMMODEL)
      positionInPkgFileList( twFindResults->currentItem() );
	}
	else ke->ignore();
}

void FindDialogImpl::execContextMenutwFindResults(QPoint p){
  if (m_searchPlace == ectn_INSIDE_INSTALLED_PACKAGES){
    if (twFindResults->selectedItems().count() == 1 &&
        twFindResults->currentItem()->icon(0).pixmap(QSize(22,22)).toImage() ==
        m_iconFile.pixmap(QSize(22,22)).toImage() &&
        QFile(twFindResults->currentItem()->text(0)).exists()==true){

      QMenu menu(this);
      QPoint pt2 = twFindResults->mapToGlobal(p);
      pt2.setY(pt2.y() + twFindResults->header()->height());

      positionInInstalledPkgList(twFindResults->currentItem());
      menu.addAction(actionOpenFile);
      if(!UnixCommand::isRootRunning()) menu.addAction(actionEditFile);
      menu.exec(pt2);
    }
  }
  else if (m_searchPlace == ectn_INSIDE_QSTDITEMMODEL){
    if (twFindResults->selectedItems().count() == 1 &&
        twFindResults->currentItem()->icon(0).pixmap(QSize(22,22)).toImage() ==
        m_iconFile.pixmap(QSize(22,22)).toImage()){

      QFile file(twFindResults->currentItem()->parent()->text(0) + QDir::separator() +
            twFindResults->currentItem()->text(0));

      QMenu menu(this);
      QPoint pt2 = twFindResults->mapToGlobal(p);
      pt2.setY(pt2.y() + twFindResults->header()->height());

      if ((m_isInstalledPackage && file.exists()) ||
          !m_isInstalledPackage ||
          (m_isInstalledPackage && !file.exists())){
        positionInPkgFileList(twFindResults->currentItem());
        menu.addAction(actionOpenFile);

        if (!UnixCommand::isRootRunning() && m_isInstalledPackage &&
            file.exists() && UnixCommand::isTextFile(file.fileName()))
          menu.addAction(actionEditFile);

        menu.exec(pt2);
      }
    }
  }
}

void FindDialogImpl::canOpenFile(QTreeWidgetItem*){
  if (m_searchPlace == ectn_INSIDE_QSTDITEMMODEL){
    if(twFindResults->selectedItems().count() == 1 &&
       twFindResults->currentItem()->icon(0).pixmap(QSize(22,22)).toImage() ==
       m_iconFile.pixmap(QSize(22,22)).toImage()){

      QFile file(twFindResults->currentItem()->parent()->text(0) + QDir::separator() +
            twFindResults->currentItem()->text(0));

      if (m_isInstalledPackage && file.exists())
        WMHelper::openFile(twFindResults->currentItem()->parent()->text(0) + QDir::separator() +
                            twFindResults->currentItem()->text(0));
      else if (!m_isInstalledPackage || (m_isInstalledPackage && !file.exists()))
        WMHelper::openFile(twFindResults->currentItem()->parent()->text(0) + QDir::separator() +
                            twFindResults->currentItem()->text(0), m_packagePath);
    }
  }
  else if (m_searchPlace == ectn_INSIDE_INSTALLED_PACKAGES){
    if (twFindResults->selectedItems().count() == 1 &&
        twFindResults->currentItem()->icon(0).cacheKey() == m_iconFile.cacheKey() &&
        QFile(twFindResults->currentItem()->text(0)).exists()==true){

      WMHelper::openFile(twFindResults->selectedItems()[0]->text(0));
    }
  }
}

void FindDialogImpl::openFile(){
  if (m_searchPlace == ectn_INSIDE_INSTALLED_PACKAGES)
    WMHelper::openFile(twFindResults->selectedItems()[0]->text(0));
  else if (m_searchPlace == ectn_INSIDE_QSTDITEMMODEL){
    QFile file(twFindResults->currentItem()->parent()->text(0) + QDir::separator() +
          twFindResults->currentItem()->text(0));

    if (m_isInstalledPackage && file.exists())
      WMHelper::openFile(twFindResults->currentItem()->parent()->text(0) + QDir::separator() +
                            twFindResults->currentItem()->text(0));
    else
      WMHelper::openFile(twFindResults->currentItem()->parent()->text(0) + QDir::separator() +
                            twFindResults->currentItem()->text(0), m_packagePath);
  }
}

void FindDialogImpl::editFile(){
  if (m_searchPlace == ectn_INSIDE_INSTALLED_PACKAGES)
    WMHelper::editFile(twFindResults->selectedItems()[0]->text(0));
  else if (m_searchPlace == ectn_INSIDE_QSTDITEMMODEL)
    WMHelper::editFile(twFindResults->currentItem()->parent()->text(0) + QDir::separator() +
                            twFindResults->currentItem()->text(0));
}

/*
 * ThreadFind methods
*/

ThreadFind::ThreadFind(){
  m_qstandardItemModel = 0;
}

ThreadFind::~ThreadFind(){  
  delete m_qstandardItemModel;
}

void ThreadFind::setQStandardItemModel( const QStandardItemModel *sim ){
  if (!sim) return;
  m_qstandardItemModel = new QStandardItemModel(this);

  QStandardItem *root = m_qstandardItemModel->invisibleRootItem();
  for(int c=0; c < sim->rowCount(sim->invisibleRootItem()->index()); c++){
    QStandardItem *it = sim->item(c, 0);
    QStandardItem *clone = it->clone();

    if (it->hasChildren())
      _copyChildItem(it, clone);

    root->appendRow(clone);
  }
}

void ThreadFind::run(){
  if (m_searchPlace == ectn_INSIDE_INSTALLED_PACKAGES)
    m_map = PackageController::findFile(m_stringToSearch);
  else if (m_searchPlace == ectn_INSIDE_QSTDITEMMODEL)
    m_map = PackageController::findFile(m_stringToSearch, m_qstandardItemModel);
  else if (m_searchPlace == ectn_INSIDE_DIRECTORY)
    m_map = PackageController::findPackage(m_stringToSearch, m_targetDir);
}

void ThreadFind::freeGarbage(){
  foreach(QString k, m_map.keys()){
    QStringList sl = m_map.value(k);
    sl.clear();
  }

  m_map.clear();
  if (m_qstandardItemModel)
    m_qstandardItemModel->clear();
}

QMap<QString, QStringList>& ThreadFind::getResultMap(){
	return m_map;
}

void _copyChildItem(QStandardItem *item, QStandardItem *clone){
  for(int c=0; c < item->rowCount(); c++){
    QStandardItem *it = item->child(c, 0);
    QStandardItem *copy = it->clone();

    clone->appendRow(copy);
    if (it->hasChildren())
      _copyChildItem(it, copy);
  }
}
