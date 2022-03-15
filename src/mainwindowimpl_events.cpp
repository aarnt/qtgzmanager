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

#include <iostream>
#include "searchbar.h"
#include "mainwindowimpl.h"
#include "unixcommand.h"
#include "strconstants.h"
#include "uihelper.h"

#include <QMessageBox>

//Helper function to verify if a file already exists in a copy operation (if so, it prefix "copy-of")
bool _copyFile(QString sourceDir, QString fileName, QString targetDir, bool cutOperation){
  QFile fileToCopy(sourceDir + QDir::separator() + fileName);
  QFile destinationFile(targetDir + QDir::separator() + fileName);
  int c=0;

  if (!cutOperation){
    while (destinationFile.exists()){
      if (c < 1)
        destinationFile.setFileName(targetDir + QDir::separator() + "copy-of-" + fileName);
      else
        destinationFile.setFileName(targetDir + QDir::separator() + "copy" +
                                    QString::number(c) + "-of-" + fileName);
      c++;
    }
  }

  return (fileToCopy.copy(destinationFile.fileName()));
}

/*
  This event was reimplemented to make sure tvDir's current directory is always
  visible among window resizes.
*/
void MainWindowImpl::resizeEvent(QResizeEvent *){
  if (!tvDir->rect().contains(tvDir->visualRect(tvDir->currentIndex())))
    tvDir->scrollTo(tvDir->currentIndex(), QAbstractItemView::PositionAtCenter);
}

/*
  This event was reimplemented for 3 reasons:

  1) Show a menu for closing the Lower View tabs (close and close all when more then 2 are opened).
  2) Enable the user to press Enter and open the file/directory selected in the package file list.
  3) Ensure the systemTray uses the correct ToolTip styleSheet.
*/
bool MainWindowImpl::eventFilter(QObject *obj, QEvent *event){
  if (event->type() == QEvent::ToolTip &&
      obj->objectName() == "titleDockDirectories"){

    //Only if one cannot see the entire directory name in the title dock, a tooltip will show
    removeStyleOfToolTip();
    int textPos = m_titleDockDirectories->cursorPositionAt(m_titleDockDirectories->rect().bottomLeft());

    if (textPos == 0) return true;
  }

  else if (obj->objectName() == "tabBar" && event->type() == QEvent::MouseButtonPress){
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

    for (int i=0; i<m_tabBar->count(); i++){
      if ( m_tabBar->tabRect(i).contains( mouseEvent->pos() ) ){
        m_clickedTab = i;
        break;
      }
    }
    if ((mouseEvent != 0) && ( mouseEvent->button() == Qt::RightButton ) &&
         (m_clickedTab != 0) && (m_clickedTab != 1)) {

      QMenu *menu = new QMenu(this);

      if (m_tabBar->count() > 3){
        menu->addAction(actionCloseClickedTab);
        menu->addAction(actionCloseAllTabs);
      }
      else
        menu->addAction(actionCloseClickedTab);

      menu->exec(mouseEvent->globalPos());
      m_clickedTab = -1;
    }
  }
  else if (obj->objectName() == "tvPkgFileList" && event->type() == QEvent::KeyPress){
    QTreeView *tv = static_cast<QTreeView *>(obj);
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

    if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
      openFileOrDirectory(tv->currentIndex());
  }

  else if (event->type() == QEvent::ToolTip &&
           SettingsManager::getShowPackageTooltip()){

    if (obj->inherits("QStandardItem"))
      qApp->setStyleSheet(StrConstants::getToolTipNormalCSS());

    else if (obj->inherits("QPushButton") ||
             obj->inherits("QToolButton") ||
             obj->inherits("QSystemTrayIcon") ||
             obj->inherits("QRadioButton") ||
             obj->inherits("QCheckBox") ||
             obj->inherits("QLabel") ||
             obj->inherits("QLineEdit") ||
             obj->objectName() == "toolButton")

      removeStyleOfToolTip();
  }

  return false;
}

void MainWindowImpl::keyReleaseEvent(QKeyEvent* ke){
  static int i=0;
  static int k=-9999;
  static int k_count=0;
  static QString cur_dir=m_modelDir->filePath(tvDir->currentIndex());

  if ((tvPackage->hasFocus()) && (ke->key() >= Qt::Key_A) && (ke->key() <= Qt::Key_Z)){
    QList<QStandardItem*> fi = m_modelPackage->findItems( ke->text(), Qt::MatchStartsWith, ctn_PACKAGE_NAME );
    if (fi.count() > 0){
      if ( (cur_dir != m_modelDir->filePath(tvDir->currentIndex()) ) ||
           (ke->key() != k) || (fi.count() != k_count) ) i=0;

      for(QStandardItem* si: fi){
        QModelIndex mi = si->index();
        mi = m_proxyModelPackage->mapFromSource(mi);
        if (!m_proxyModelPackage->hasIndex(mi.row(), mi.column())) fi.removeAll(si);
      }

      if (fi.count()==0) return;

      QList<SelectedPackage> sp = getSelectedPackage();
      if (sp.count() == 1) {
        int a=0;
        while (a<=fi.count()-1) {
          QStandardItem* si = fi[a];
          if (si->text() == sp[sp.count()-1].getFileName()) break;
          a++;
          i=a;
        }
        if((a+1)<=fi.count()-1) { a++; i=a; }
        else { a=i=0; }
      }

      tvPackage->selectionModel()->clear();
      QModelIndex mi = fi[i]->index();
      mi = m_proxyModelPackage->mapFromSource(mi);
      tvPackage->scrollTo(mi);

      QModelIndex maux = m_proxyModelPackage->index( mi.row(), ctn_PACKAGE_ICON );
      tvPackage->selectionModel()->setCurrentIndex(mi, QItemSelectionModel::Select);
      tvPackage->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);
      tvPackage->setCurrentIndex(mi);
      selectInstalledPackage();

      if ((i <= fi.count()-1)) i++;
      if (i == fi.count()) i = 0;
    }

    k = ke->key();
    k_count = fi.count();
    cur_dir=m_modelDir->filePath(tvDir->currentIndex());
  }

  else if ((tvInstalledPackages->hasFocus()) && (ke->key() >= Qt::Key_A) && (ke->key() <= Qt::Key_Z)) {
    QList<QStandardItem*> fi =
        m_modelInstalledPackages->findItems( ke->text(), Qt::MatchStartsWith, ctn_PACKAGE_NAME );

    if (fi.count() > 0){
      if ( (ke->key() != k) || (fi.count() != k_count) ) i=0;

      for(QStandardItem* si: fi){
        QModelIndex mi = si->index();
        mi = m_proxyModelInstalledPackages->mapFromSource(mi);
        if (!m_proxyModelInstalledPackages->hasIndex(mi.row(), mi.column())) fi.removeAll(si);
      }

      if (fi.count()==0) return;

      QList<SelectedPackage> sp = getSelectedInstalledPackage();
      if (sp.count() == 1) {
        int a=0;

        while (a<=fi.count()-1) {
          QStandardItem* si = fi[a];
          if (si->text() == sp[sp.count()-1].getFileName())
            break;
          a++;
          i=a;
        }

        if ((a+1)<=fi.count()-1){
          a++;
          i=a;
        }
        else a=i=0;
      }

      tvInstalledPackages->selectionModel()->clear();
      QModelIndex mi = fi[i]->index();
      mi = m_proxyModelInstalledPackages->mapFromSource(mi);
      tvInstalledPackages->scrollTo(mi, QAbstractItemView::PositionAtCenter);

      QModelIndex maux = m_proxyModelInstalledPackages->index(mi.row(), ctn_PACKAGE_ICON);
      tvInstalledPackages->selectionModel()->setCurrentIndex(mi, QItemSelectionModel::Select);
      tvInstalledPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);

      if ((i <= fi.count()-1)) i++;
      if (i == fi.count()) i = 0;
    }

    k = ke->key();
    k_count = fi.count();
    cur_dir = "";
  }

  else if ((ke->key() != Qt::Key_Shift) && (ke->key() != Qt::Key_Control) &&
           (tvPackage->hasFocus() &&
            (ke->key() == Qt::Key_Down ||
             ke->key() == Qt::Key_Up ||
             ke->key() == Qt::Key_PageDown ||
             ke->key() == Qt::Key_PageUp))) {
    tvPackage->scrollTo(tvPackage->currentIndex());
    selectInstalledPackage();
  }

  else if ((ke->key() == Qt::Key_Down || ke->key() == Qt::Key_Up||
            ke->key() == Qt::Key_PageDown || ke->key() == Qt::Key_PageUp))
  {
    QTreeView *t = twTODO->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
    if ( (t != 0) && (t->hasFocus())) {
      t->scrollTo( t->currentIndex() );
      showFullPathOfObject( t->currentIndex() );
    }
  }

  else ke->ignore();
}

void MainWindowImpl::keyPressEvent(QKeyEvent* ke){
  if ( ((ke->key() < Qt::Key_A) || (ke->key() > Qt::Key_Z)) && ((ke->key() != Qt::Key_Shift) &&
                                                                (ke->key() != Qt::Key_Control) &&
                                                                (tvPackage->hasFocus()))) {
    tvPackage->scrollTo(tvPackage->currentIndex());
    selectInstalledPackage();
  }

  else {
    QTreeView *t = twTODO->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
    if ( (t != 0)  && (t->hasFocus())) {
      t->scrollTo( t->currentIndex() );
      showFullPathOfObject( t->currentIndex() );
    }
  }

  if ( (tvPackage->hasFocus()) &&
       ((ke->key() == Qt::Key_L) && (ke->modifiers() == Qt::ControlModifier)) &&
       ( UnixCommand::hasTheExecutable(ctn_TXZ2SBBIN) ||
         UnixCommand::hasTheExecutable(ctn_TGZ2LZMBIN) ||
         UnixCommand::hasTheExecutable(ctn_MAKELZMBIN) )) {

    for(SelectedPackage ami: getSelectedPackage())
      if (ami.getIcon().pixmap(QSize(22,22)).toImage() ==
          IconHelper::getIconRPM().pixmap(QSize(22,22)).toImage()) return;

    QModelIndexList lmi = tvPackage->selectionModel()->selectedIndexes();
    if (lmi.size() > 0) transformTGZinLZM();
  }

  /*else if ((ke->modifiers() == (Qt::ShiftModifier)) && (ke->key() == Qt::Key_F3)){
    searchBarFindPrevious();
  }*/

  //This code simulates a directory rename event in tvDir.
  else if (ke->key() == Qt::Key_F2){
    if (!actionMaximizeLowerView->isChecked() && !tvDir->hasFocus()){
      QKeyEvent *ke = new QKeyEvent(QEvent::KeyPress, Qt::Key_F2, Qt::NoModifier);
      qApp->postEvent(tvDir, ke);
    }
  }

  /*else if (ke->key() == Qt::Key_F3){
    searchBarFindNext();
  }*/

  else if (ke->key() == Qt::Key_F6) openDirectory();

  else if (ke->key() == Qt::Key_F4) openTerminal();

  else if ((ke->key() == Qt::Key_F8) && (tvPackage->hasFocus())){
    showPackageInfo();
  }

  else if ((ke->key() == Qt::Key_F8) && (tvInstalledPackages->hasFocus())){
    showInstalledPackageInfo();
  }

  else if ((ke->key() == Qt::Key_Delete) && (tvPackage->hasFocus())){
    QModelIndexList lmi = tvPackage->selectionModel()->selectedIndexes();
    if (lmi.size() > 0) deleteFile();
  }

  else if ((ke->key() == Qt::Key_Delete) && (tvTODO->hasFocus())){
    //Here we have just one action selected
    if (tvTODO->selectionModel()->selectedRows().count() == 1){
      QModelIndex mi = tvTODO->currentIndex();
      int res=0;
      if ( (m_modelTodo->itemFromIndex(mi)->parent() == 0) && (m_modelTodo->rowCount(mi) > 0)){
        res = QMessageBox::question ( this, tr ( "Confirmation" ),
                                      tr ( "Are you sure you want to delete all these actions?" ),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No );

        if (res == QMessageBox::Yes) m_modelTodo->removeRows (0, m_modelTodo->rowCount(mi), mi);
      }
      else if ((m_modelTodo->itemFromIndex(mi)->parent() != 0)){
        res = QMessageBox::question(this, tr("Confirmation"),
                                    tr("Are you sure you want to delete this action?"),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);

        if (res == QMessageBox::Yes) m_modelTodo->removeRow(mi.row(), mi.parent());
      }
    }
    //Here we have more than one action selected
    else{
      deleteSelectedActionFiles();
    }
  }

  else if ((ke->key() == Qt::Key_Tab) && (ke->modifiers() == (Qt::ControlModifier)) &&
           !twTODO->hasFocus()){
    if (!actionMinimizeLowerView->isChecked()){
      if (twTODO->currentIndex() == twTODO->count()-1)
        twTODO->setCurrentIndex(0);
      else
        twTODO->setCurrentIndex(twTODO->currentIndex()+1);
    }
  }

  else if ((ke->key() == Qt::Key_E) &&
           (ke->modifiers() == (Qt::ControlModifier)) && actionExecuteActions->isEnabled() ){

    m_ignoreCancelledActions = true;
    executePackageActions();
  }

  else if ((ke->key() == Qt::Key_F) && (ke->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))){
    //Only let user finds a file if this is actually a package...
    if (getSelectedPackage().count() == 1){
      SelectedPackage sp = getSelectedPackage().at(0);
      if (sp.getIcon().pixmap(QSize(22,22)).toImage() !=
          IconHelper::getIconBinary().pixmap(QSize(22,22)).toImage())
        findFileInPackage();
    }
  }

  else if ((ke->key() == Qt::Key_D) && (ke->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
           && !actionMaximizeLowerView->isChecked()){
    gotoDirectory(SettingsManager::instance()->getDefaultDirectory());
  }

  else if ((ke->key() == Qt::Key_P) && (ke->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
           && !actionMaximizeLowerView->isChecked()){
    gotoDirectory(SettingsManager::instance()->getUpdaterDirectory());
  }

  else if ((ke->key() == Qt::Key_V) && (ke->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
           && !actionMaximizeLowerView->isChecked()){
    gotoDirectory(ctn_VAR_DIRECTORY);
  }

  else if ((ke->key() == Qt::Key_K) && (ke->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
           && !actionMaximizeLowerView->isChecked()){
    QDir s32(ctn_SLACKPKG_CACHE_DIR + "/" + "slackware");
    QDir s64(ctn_SLACKPKG_CACHE_DIR + "/" + "slackware64");
    QDir arm1(ctn_SLACKPKG_CACHE_DIR + "/" + "slackwarearm");
    QDir arm2(ctn_SLACKPKG_CACHE_DIR + "/" + "armedslack");

    if (s32.exists())
      gotoDirectory(s32.absolutePath());
    else if (s64.exists())
      gotoDirectory(s64.absolutePath());
    else if (arm1.exists())
      gotoDirectory(arm1.absolutePath());
    else if (arm2.exists())
      gotoDirectory(arm2.absolutePath());
    else
      gotoDirectory(ctn_SLACKPKG_CACHE_DIR);
  }

  else if ((ke->key() == Qt::Key_T) && (ke->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
           && !actionMaximizeLowerView->isChecked()){
    gotoDirectory(QDir::tempPath());
  }

  else if ((ke->key() == Qt::Key_S) && (ke->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
           && !actionMaximizeLowerView->isChecked()){
    gotoDirectory(ctn_SLAPTSRC_TEMP_DIR);
  }

  else if ((ke->key() == Qt::Key_S) && (ke->modifiers() == (Qt::ControlModifier | Qt::AltModifier))){
    takeSnapshotOfInstalledPackages(ectn_NO_MODIFIED_DATE);
  }

  else if ((ke->key() == Qt::Key_W) && (ke->modifiers() == Qt::ControlModifier)){
    if (!actionMinimizeLowerView->isChecked()) closeCurrentTab();
  }

  //Threre is a requested Cut operation...
  else if ((ke->key() == Qt::Key_X || ke->key() == Qt::Key_C) && (ke->modifiers() == Qt::ControlModifier) &&
           (tvPackage->hasFocus() && getSelectedPackage().count() > 0)){
    if (ke->key() == Qt::Key_X)
      cutPackages();
    else
      copyPackages();
  }

  else if ((ke->key() == Qt::Key_V) && (ke->modifiers() == Qt::ControlModifier) &&
           (!actionMaximizeLowerView->isChecked() && m_packagesClipboard.count() > 0)){
    pastePackages();
  }

  else if ((ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) &&
           (tvPackage->hasFocus() || tvInstalledPackages->hasFocus())){
    showPackageContent();
  }

  else if ((ke->key() == Qt::Key_Escape) && (tvDir->hasFocus())){
    tvPackage->setFocus();
  }

  else if ((ke->key() == Qt::Key_Escape) && (leFilterPackage->hasFocus())){
    if (!leFilterPackage->text().isEmpty())
        leFilterPackage->clear();
  }

  else if ((ke->key() == Qt::Key_Escape) && tvInstalledPackages->hasFocus()){
      actionHideRightView->setChecked(true);
      hideRightView(ectn_Save);
  }

  else if ((ke->key() == Qt::Key_Escape) && (leFilterInstalledPackages->hasFocus())){
      if (!leFilterInstalledPackages->text().isEmpty())
          leFilterInstalledPackages->clear();
      else{
          actionHideRightView->setChecked(true);
          hideRightView(ectn_Save);
      }
  }

  else ke->ignore();
}

void MainWindowImpl::_cutCopyPackages(){
  QString path;

  for(SelectedPackage pkg: getSelectedPackage()){
    m_packagesClipboard.add(pkg.getFileName());
    path=pkg.getPath();
  }

  m_packagesClipboard.setSourceDir(path);
}

void MainWindowImpl::cutPackages(){
  m_packagesClipboard.clear();
  m_packagesClipboard.setCutOperation(true);

  _cutCopyPackages();
}

void MainWindowImpl::copyPackages(){
  m_packagesClipboard.clear();
  m_packagesClipboard.setCutOperation(false);

  _cutCopyPackages();
}

void MainWindowImpl::pastePackages(){
  CPUIntensiveComputing *ri = new CPUIntensiveComputing();
  QString sourceDir = m_packagesClipboard.getSourceDir();
  QString targetDir = dockDirectories->windowTitle();

  for(QString pkg: m_packagesClipboard.getPackageList()){
    qApp->processEvents();
    QFile fileToCopy(sourceDir + QDir::separator() + pkg);

    if (_copyFile(sourceDir, pkg, targetDir, m_packagesClipboard.getCutOperation())){
      if(m_packagesClipboard.getCutOperation()){
        fileToCopy.remove();
      }
    }
    else{
      if (m_packagesClipboard.getCutOperation()){
        delete ri;
        QMessageBox::warning(this, tr("Moving file..."), tr("Could not move file %1 to %2.").arg(pkg).arg(targetDir));
        ri = new CPUIntensiveComputing();
      }
      else{
        delete ri;
        QMessageBox::warning(this, tr("Copying file..."), tr("Could not copy file %1 to %2.").arg(pkg).arg(targetDir));
        ri = new CPUIntensiveComputing();
      }
    }
  }

  m_packagesClipboard.clear();
  delete ri;
}

void MainWindowImpl::closeEvent(QCloseEvent *event){
  m_savedGeometry = this->saveGeometry();

  m_reallyWannaClose = !SettingsManager::getWindowCloseHidesApp();
  if ((m_reallyWannaClose) // || !SettingsManager::getWindowCloseHidesApp())
      && (hasPendingActions())){
    int res = QMessageBox::question(this, tr("Confirmation"),
                                    tr("There are actions waiting for execution!") + "\n" +
                                    tr("Are you sure you want to exit?"),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);

    if (res == QMessageBox::Yes)
      event->accept();
    else{
      event->ignore();
      if (isHidden()) show();
    }
  }
  else if(m_reallyWannaClose)
    event->accept();
  else
    event->ignore();

  if (event->isAccepted()){
    QByteArray windowSize=saveGeometry();
    SettingsManager::setWindowSize(windowSize);
    close();
  }
  else hide();

  //m_reallyWannaClose = false;
}
