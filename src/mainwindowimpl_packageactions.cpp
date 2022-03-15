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

#include "mainwindowimpl.h"
#include "packagecontroller.h"
#include "package.h"
#include "unixcommand.h"
#include "strconstants.h"
#include "searchlineedit.h"
#include "searchbar.h"
#include "uihelper.h"
#include "wmhelper.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

QFile* UnixCommand::m_temporaryFile;

void MainWindowImpl::onTabMoved(int from, int to){
  if( ((from == 0 && to != 0) || (from == 1 && to != 1)) ||
      ((to == 0 && from != 0) || (to == 1 && from != 1)) ){
    disconnect(m_tabBar, SIGNAL(tabMoved(int,int)), this, SLOT(onTabMoved(int, int)));
    m_tabBar->moveTab(to, from);
    connect(m_tabBar, SIGNAL(tabMoved(int,int)), this, SLOT(onTabMoved(int, int)));
  }
}

void MainWindowImpl::_tvTODOAdjustItemText(QStandardItem *item){
  int countSelected=0;

  for(int c=0; c < item->rowCount(); c++){
    if(tvTODO->selectionModel()->isSelected(item->child(c)->index()))
        countSelected++;
  }

  QString itemText = item->text();
  int slash = itemText.indexOf("/");
  int pos = itemText.indexOf(")");

  if (slash > 0){
    itemText.remove(slash, pos-slash);
  }

  pos = itemText.indexOf(")");
  itemText.insert(pos, "/" + QString::number(countSelected));
  item->setText(itemText);
}

void MainWindowImpl::tvTODOSelectionChanged (const QItemSelection&, const QItemSelection&){
  _tvTODOAdjustItemText(m_install);
  _tvTODOAdjustItemText(m_reinstall);
  _tvTODOAdjustItemText(m_upgrade);
  _tvTODOAdjustItemText(m_downgrade);
  _tvTODOAdjustItemText(m_remove);
}

void MainWindowImpl::_tvTODORowsChanged(const QModelIndex& parent){
  QStandardItem *item = m_modelTodo->itemFromIndex(parent);
  QString count = QString::number(item->rowCount());

  if (item == m_install){
    if (item->rowCount() > 0){
      m_install->setText(StrConstants::getTodoInstallText() + " (" + count + ")");
      _tvTODOAdjustItemText(m_install);
    }
    else m_install->setText(StrConstants::getTodoInstallText());
  }
  else if (item == m_reinstall){
    if (item->rowCount() > 0){
      m_reinstall->setText(StrConstants::getTodoReinstallText() + " (" + count + ")");
      _tvTODOAdjustItemText(m_reinstall);
    }
    else m_reinstall->setText(StrConstants::getTodoReinstallText());
  }
  else if (item == m_upgrade){
    if (item->rowCount() > 0){
      m_upgrade->setText(StrConstants::getTodoUpgradeText() + " (" + count + ")");
      _tvTODOAdjustItemText(m_upgrade);
    }
    else m_upgrade->setText(StrConstants::getTodoUpgradeText());
  }
  else if (item == m_downgrade){
    if (item->rowCount() > 0){
      m_downgrade->setText(StrConstants::getTodoDowngradeText() + " (" + count + ")");
      _tvTODOAdjustItemText(m_downgrade);
    }
    else m_downgrade->setText(StrConstants::getTodoDowngradeText());
  }
  else if (item == m_remove){
    if (item->rowCount() > 0){
      m_remove->setText(StrConstants::getTodoRemoveText() + " (" + count + ")");
      _tvTODOAdjustItemText(m_remove);
   }
    else m_remove->setText(StrConstants::getTodoRemoveText());
  }
}

void MainWindowImpl::tvTODORowsInserted(const QModelIndex& parent, int, int){
  _tvTODORowsChanged(parent);
}

void MainWindowImpl::tvTODORowsRemoved(const QModelIndex& parent, int, int){
  _tvTODORowsChanged(parent);
}

void MainWindowImpl::deleteActionFile(){
  QModelIndex mi = tvTODO->currentIndex();

  int res = QMessageBox::question ( this, tr ( "Confirmation" ),
                                    tr ( "Are you sure you want to remove this Action?" ),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No );

  if ( res == QMessageBox::Yes ){
    m_modelTodo->removeRow ( mi.row(), mi.parent() );
  }
}

void MainWindowImpl::deleteAllActionFiles(){
  QModelIndex mi = tvTODO->currentIndex();

  int res = QMessageBox::question ( this, tr ( "Confirmation" ),
                                    tr ( "Are you sure you want to remove all these Actions?" ),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No );

  if ( res == QMessageBox::Yes ){
    m_modelTodo->removeRows (0, m_modelTodo->rowCount(mi), mi);
  }
}

void MainWindowImpl::deleteSelectedActionFiles(){
  int res = QMessageBox::question ( this, tr ( "Confirmation" ),
                                tr ( "Are you sure you want to remove all these Actions?" ),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No );

  if (res == QMessageBox::Yes){
    for(int c=tvTODO->selectionModel()->selectedIndexes().count()-1; c>=0; c--){
      const QModelIndex mi = tvTODO->selectionModel()->selectedIndexes().at(c);
      if (m_modelTodo->itemFromIndex(mi)->parent() != 0){
        m_modelTodo->removeRow(mi.row(), mi.parent());
      }
    }
  }
}

void MainWindowImpl::execContextMenuTodoTreeView(QPoint point){
  if (tvTODO->selectionModel()->selectedRows().count()==0) return;
  QModelIndex mi = tvTODO->currentIndex();
  QMenu menu(this);

  //Which actions do we have to put? It depends on package status
  if ( (m_modelTodo->itemFromIndex(mi)->parent() == 0) && (m_modelTodo->rowCount(mi) > 0))
    menu.addAction(actionDelete_All_ActionFiles);
  else if (tvTODO->selectionModel()->selectedRows().count() == 1 &&
           m_modelTodo->itemFromIndex(mi)->parent() != 0)
    menu.addAction(actionDelete_ActionFile);
  else if (tvTODO->selectionModel()->selectedRows().count() > 1)
    menu.addAction(actionDelete_SelectedActionFiles);

  QPoint pt2 = tvTODO->mapToGlobal(point);
  pt2.setY(pt2.y() + tvTODO->header()->height());
  menu.exec(pt2);
}

void MainWindowImpl::clearTodoTreeView(){
  m_install->removeRows(0, m_install->rowCount());
  m_remove->removeRows(0, m_remove->rowCount());
  m_downgrade->removeRows(0, m_downgrade->rowCount());
  m_upgrade->removeRows(0, m_upgrade->rowCount());
  m_reinstall->removeRows(0, m_reinstall->rowCount());
  delete m_modelInstalledPackages;
  delete m_proxyModelInstalledPackages;
  initializeInstalledPackagesTreeView();
}

bool MainWindowImpl::hasPendingActions(){
  return (
      (m_reinstall->rowCount() != 0) ||
      (m_install->rowCount() != 0) ||
      (m_remove->rowCount() != 0) ||
      (m_downgrade->rowCount() != 0) ||
      (m_upgrade->rowCount() != 0));
}

QString MainWindowImpl::getHtmlListOfCancelledActions(){
  QString htmlList("");

  if (m_discardedPackageList.size() > 0){
    htmlList += "<pre>" + tr("The installation of the following packages was canceled:");
    htmlList += "<ul>";

    for(QString discardedPackage: m_discardedPackageList){
      htmlList += "<li>" + discardedPackage + "</li>";
    }

    htmlList += "</ul></pre><br>";
  }

  return htmlList;
}

QString MainWindowImpl::getStrSilentOutput(){
  if (SettingsManager::getUseSilentActionOutput())
    return " 1>/dev/null ";
  else
    return " ";
}

void MainWindowImpl::executePackageActions(){
  //If there are no means to run the actions, we must warn!
  if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
    QMessageBox::critical( 0, "QTGZManager", StrConstants::getErrorNoSuCommand());
    return;
  }
  else if (!SettingsManager::getUsePkgTools() && !UnixCommand::isSpkgInstalled()){
    QMessageBox::critical( 0, "QTGZManager", StrConstants::getErrorSpkgNotInstalled());
    return;
  }

  //Verify if QTGZ uses spkg and if it's still installed...
  if (SettingsManager::getUsePkgTools() == 0 &&
          !UnixCommand::isSpkgInstalled()){
      SettingsManager::setUsePkgTools(true);
  }

  //actionExit->setEnabled(false);
  actionSetup->setEnabled(false);

  QStringList out;
  bool changedToPkgTool=false;

  if (SettingsManager::getUsePkgTools() == 0){
    for (int r=0; r<m_downgrade->rowCount(); r++){
      QFileInfo fi(m_downgrade->child(r)->text());
      if(fi.path().contains(SettingsManager::getUpdaterDirectory()) ||
            fi.fileName().contains("glibc")){

            SettingsManager::setUsePkgTools(true);
            changedToPkgTool=true;
      }
    }

    for (int r=0; r<m_upgrade->rowCount(); r++){
      QFileInfo fi(m_upgrade->child(r)->text());
        if(fi.path().contains(SettingsManager::getUpdaterDirectory()) ||
              fi.fileName().contains("glibc")){

              SettingsManager::setUsePkgTools(true);
              changedToPkgTool=true;
        }
    }
  }

  for (int r=0; r<m_remove->rowCount(); r++){
    QFileInfo fi(m_remove->child(r)->text());
    out << "echo -e " << StrConstants::getExecutingCommand() << " \\\x27" << "cd " << fi.path() << "\\\x27 \n";
    out << "echo -e " << StrConstants::getExecutingCommand() << " \\\x27" <<
           UnixCommand::getPkgRemoveCommand() << " " << fi.fileName() << "\\\x27...\n";
    out << "cd \"" << fi.path() << "\" \n";
    out << UnixCommand::getPkgRemoveCommand() << " " << fi.fileName() << getStrSilentOutput() << "\n";
  }
  for (int r=0; r<m_downgrade->rowCount(); r++){
    QFileInfo fi(m_downgrade->child(r)->text());
    out << "echo -e " << StrConstants::getExecutingCommand() << " \\\x27" << "cd " << fi.path() << "\\\x27 \n";
    out << "echo -e " << StrConstants::getExecutingCommand() << " \\\x27" <<
           UnixCommand::getPkgUpgradeCommand() << " " << fi.fileName() << "\\\x27...\n";
    out << "cd \"" << fi.path() << "\" \n";
    out << UnixCommand::getPkgUpgradeCommand() << " " << fi.fileName() << getStrSilentOutput() << "\n";
  }
  for (int r=0; r<m_upgrade->rowCount(); r++){
    QFileInfo fi(m_upgrade->child(r)->text());
    out << "echo -e " << StrConstants::getExecutingCommand() << " \\\x27" << "cd " << fi.path() << "\\\x27 \n";
    out << "echo -e " << StrConstants::getExecutingCommand() << " \\\x27" <<
           UnixCommand::getPkgUpgradeCommand() << " " << fi.fileName() << "\\\x27...\n";
    out << "cd \"" << fi.path() << "\" \n";
    out << UnixCommand::getPkgUpgradeCommand() << " " << fi.fileName() << getStrSilentOutput() << "\n";
  }

  if (!m_ignoreCancelledActions){
    for (int r=0; r<m_install->rowCount(); r++){
      QFileInfo fi(m_install->child(r)->text());

      if (Package::isValid(fi.filePath()) && Package::isSlackPackage(fi.filePath())) {
        out << "echo -e " << StrConstants::getExecutingCommand() << " \\\x27" << "cd " << fi.path() << "\\\x27 \n";
        out << "echo -e " << StrConstants::getExecutingCommand() << " \\\x27" <<
               UnixCommand::getPkgInstallCommand() << " " << fi.fileName() << "\\\x27...\n";
        out << "cd \"" << fi.path() << "\" \n";
        out << UnixCommand::getPkgInstallCommand() << " " << fi.fileName() << getStrSilentOutput() << "\n";
      }
      else{
        //Append to Discarded package list...
        m_discardedPackageList << fi.filePath();
      }
    }
  }
  else{
    m_ignoreCancelledActions = false;

    for (int r=0; r<m_install->rowCount(); r++){
      QFileInfo fi(m_install->child(r)->text());
      out << "echo -e " << StrConstants::getExecutingCommand() << " \\\x27" << "cd " << fi.path() << "\\\x27 \n";
      out << "echo -e " << StrConstants::getExecutingCommand() << " \\\x27" <<
             UnixCommand::getPkgInstallCommand() << " " << fi.fileName() << "\\\x27...\n";
      out << "cd \"" << fi.path() << "\" \n";
      out << UnixCommand::getPkgInstallCommand() << " " << fi.fileName() << getStrSilentOutput() << "\n";
    }
  }

  for (int r=0; r<m_reinstall->rowCount(); r++){
    QFileInfo fi(m_reinstall->child(r)->text());
    out << "echo -e " << StrConstants::getExecutingCommand() << " \\\x27" << "cd " << fi.path() << "\\\x27 \n";
    out << "echo -e " << StrConstants::getExecutingCommand() << " \\\x27" <<
           UnixCommand::getPkgReinstallCommand() << " " << fi.fileName() << "\\\x27...\n";
    out << "cd \"" << fi.path() << "\" \n";
    out << UnixCommand::getPkgReinstallCommand() << " " << fi.fileName() << getStrSilentOutput() << "\n";
  }

  //Let's check if a kernel pkg is included in the list
  if (out.indexOf("kernel-") != -1)
  {
    //If so, we include a "lilo" command
    out << "lilo" << "\n";
  }

  textEdit->clear();
  textEdit->setHtml("");
  conditionalGotoNormalView();
  QString htmlListOfCancelledPackages(getHtmlListOfCancelledActions());

  if (!htmlListOfCancelledPackages.isEmpty() && (
      (m_install->rowCount() + m_upgrade->rowCount() + m_downgrade->rowCount() +
      m_reinstall->rowCount() + m_remove->rowCount()) == m_discardedPackageList.size())){

    m_discardedPackageList.clear();
    twTODO->setCurrentIndex(1);
    actionExecuteActions->setEnabled( false );
    clearTodoTreeView();
    QString msg("");
    msg += "<h4>" + tr("Please, wait while the actions are being executed...") + "</h4><br>";
    msg += htmlListOfCancelledPackages;
    msg += "<br><h4>" + tr("Finished executing package actions!") + "</h4><br>" + StrConstants::getForMoreInformation();
    textEdit->setHtml(msg);
    actionSetup->setEnabled(true);
    return;
  }

  _collectActionExecGarbage();
  m_unixCommand = new UnixCommand(this);

  QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput() ),
                   this, SLOT( actionsProcessReadOutput() ));
  QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                   this, SLOT( actionsProcessRaisedError() ));

  if (changedToPkgTool){
    SettingsManager::setUsePkgTools(false);
  }

  disconnect ( m_psw, SIGNAL(directoryChanged ( const QString &)),
            this, SLOT(fileSystemWatcher_installedPackagesDirectoryChanged ( const QString &) ));

  m_unixCommand->executePackageActions(out);
}

void MainWindowImpl::actionsProcessStarted(){
  conditionalGotoNormalView();

  twTODO->setCurrentIndex(1);
  QString msg;
  textEdit->clear();
  actionExecuteActions->setEnabled( false );
  msg = "<h4>" + tr("Please, wait while the actions are being executed...") + "</h4><br>";

  QString htmlListOfCancelledActions(getHtmlListOfCancelledActions());
  if (!htmlListOfCancelledActions.isEmpty()) msg += htmlListOfCancelledActions;

  _positionTextEditCursorAtEnd();
  textEdit->insertHtml(msg);
  textEdit->ensureCursorVisible();
}

void MainWindowImpl::_positionTextEditCursorAtEnd(){
  QTextCursor tc = textEdit->textCursor();
  tc.clearSelection();
  tc.movePosition(QTextCursor::End);
  textEdit->setTextCursor(tc);
}

void MainWindowImpl::actionsProcessReadOutput(){
  twTODO->setCurrentIndex(1);

  QString out(m_unixCommand->readAllStandardOutput());
  out = _removeStringBugs(out);
  out = out.replace(QRegExp("\n"), "<br>");

  _positionTextEditCursorAtEnd();
  textEdit->insertHtml("<pre>" + out + "<pre>");
  textEdit->ensureCursorVisible();
}

void MainWindowImpl::actionsProcessFinished(int, QProcess::ExitStatus){    
  m_discardedPackageList.clear();
  conditionalGotoNormalView();
  twTODO->setCurrentIndex(1);
  QString msg;
  m_lblStatus->setText("");

  if (textEdit->toPlainText().count("\n") > 2) {
    clearTodoTreeView();
    msg = "<br><h4>" + tr("Finished executing package actions!") + "</h4><br>" + StrConstants::getForMoreInformation();
  }
  else{
    msg = "<br><h4><font color=\"red\">" + tr("Package actions' execution were cancelled!") + "</font></h4><br>" +
        StrConstants::getForMoreInformation();
    actionExecuteActions->setEnabled( true );
  }

  reapplyInstalledPackagesFilter();
  _positionTextEditCursorAtEnd();
  textEdit->insertHtml(msg);
  textEdit->ensureCursorVisible();

  _positionInFirstMatch();

  delete m_unixCommand;
  m_unixCommand = 0;

  //actionExit->setEnabled(true);
  actionSetup->setEnabled(true);
  UnixCommand::removeTemporaryActionFile();

  connect ( m_psw, SIGNAL(directoryChanged ( const QString &)),
            this, SLOT(fileSystemWatcher_installedPackagesDirectoryChanged ( const QString &) ));
  refreshInstalledPackageTreeView();
  showPackagesInDirectory();
}

QString MainWindowImpl::_removeStringBugs(const QString str){
  QString newStr(str);

  //Removing the annoying kdesu 4.x "error" messages
  newStr.remove(QRegularExpression("qt.qpa.xcb+"));
  newStr.remove(QRegularExpression("^kdesu\\(.*$"));
  newStr.remove(QRegularExpression("^kbuildsycoca.*$"));
  newStr.remove(QRegularExpression("^Connecting to deprecated signal.*$"));
  newStr.remove(QRegularExpression("^QDBusConnection.*$"));
  newStr.remove(QRegularExpression("kdeinit4.*$"));
  newStr.remove(QRegularExpression("glibtop\\(c=.*$"));
  newStr.remove(QRegularExpression(": QXcbConnection:.*$"));

  //Removing Style error messages
  newStr.remove("QGtkstyle was unable to detect the current gtk+ theme.",
                Qt::CaseInsensitive);

  return newStr;
}

void MainWindowImpl::actionsProcessRaisedError(){
  twTODO->setCurrentIndex(1);
  QString out(m_unixCommand->readAllStandardError());

  out = _removeStringBugs(out);
  out = out.replace(QRegExp("\n"), "<br>");

  _positionTextEditCursorAtEnd();
  textEdit->insertHtml("<pre>" + out + "</pre>");
  textEdit->ensureCursorVisible();
}

void MainWindowImpl::tgz2lzmProcessStarted(){
  conditionalGotoNormalView();
  twTODO->setCurrentIndex(1);
  QString msg;
  textEdit->clear();
  actionExecuteActions->setEnabled( false );
  msg = "<h4>" + tr("Please, wait while the TGZ to LZM convertion is being executed...") + "</h4><br>";

  _positionTextEditCursorAtEnd();
  textEdit->insertHtml(msg);
  textEdit->ensureCursorVisible();
}

void MainWindowImpl::tgz2lzmProcessReadOutput(){
  twTODO->setCurrentIndex(1);
  QString out(m_unixCommand->readAllStandardOutput());
  out = out.replace(QRegExp("\n"), "<br>");

  _positionTextEditCursorAtEnd();
  textEdit->insertHtml("<pre>" + out + "</pre>");
  textEdit->ensureCursorVisible();
}

void MainWindowImpl::tgz2lzmProcessFinished(int, QProcess::ExitStatus){
  conditionalGotoNormalView();
  twTODO->setCurrentIndex(1);
  QString msg;
  m_lblStatus->setText("");

  msg = "<br><h4>" + tr("Finished executing TGZ to LZM convertion!") + "</h4><br>" + StrConstants::getForMoreInformation();

  _positionTextEditCursorAtEnd();
  textEdit->insertHtml(msg);
  textEdit->ensureCursorVisible();

  _positionInFirstMatch();

  delete m_unixCommand;
  m_unixCommand = 0;

  UnixCommand::removeTemporaryActionFile();
}

void MainWindowImpl::tgz2lzmProcessRaisedError(){
  twTODO->setCurrentIndex(1);
  QString out(m_unixCommand->readAllStandardError());

  out = _removeStringBugs(out);
  out = out.replace(QRegExp("\n"), "<br>");

  _positionTextEditCursorAtEnd();
  textEdit->insertHtml("<pre>" + out + "</pre><br><br>");
  textEdit->ensureCursorVisible();
}

void MainWindowImpl::rpm2tgzProcessStarted(){
  conditionalGotoNormalView();
  twTODO->setCurrentIndex(1);
  QString msg;
  textEdit->clear();
  actionExecuteActions->setEnabled( false );
  msg = "<h4>" + tr("Please, wait while the RPM to TGZ convertion is being executed...") + "</h4><br>";

  _positionTextEditCursorAtEnd();
  textEdit->insertHtml(msg);
  textEdit->ensureCursorVisible();
}

void MainWindowImpl::rpm2tgzProcessReadOutput(){
  twTODO->setCurrentIndex(1);
  QString out(m_unixCommand->readAllStandardOutput());
  out = out.replace(QRegExp("\n"), "<br>");

  _positionTextEditCursorAtEnd();
  textEdit->insertHtml("<pre>" + out + "</pre>");
  textEdit->ensureCursorVisible();
}

void MainWindowImpl::rpm2tgzProcessFinished(int, QProcess::ExitStatus){
  conditionalGotoNormalView();
  twTODO->setCurrentIndex(1);
  QString msg;
  m_lblStatus->setText("");
  msg = "<br><h4>" + tr("Finished executing RPM to TGZ convertion!") + "</h4><br>" + StrConstants::getForMoreInformation();

  _positionTextEditCursorAtEnd();
  textEdit->insertHtml(msg);
  textEdit->ensureCursorVisible();

  _positionInFirstMatch();

  delete m_unixCommand;
  m_unixCommand = 0;

  UnixCommand::removeTemporaryActionFile();
}

void MainWindowImpl::rpm2tgzProcessRaisedError(){
  twTODO->setCurrentIndex(1);
  QString out(m_unixCommand->readAllStandardError());

  out = _removeStringBugs(out);
  out = out.replace(QRegExp("\n"), "<br>");

  _positionTextEditCursorAtEnd();
  textEdit->insertHtml("<pre>" + out + "</pre>");
  textEdit->ensureCursorVisible();
}

void MainWindowImpl::transformTGZinLZM(){
  QStringList out;
  int tgzCount=0;
  QList<SelectedPackage> lsp = getSelectedPackage();

  for(SelectedPackage ami: lsp){
    if (ami.getIcon().pixmap(QSize(22,22)).toImage() != IconHelper::getIconRPM().pixmap(QSize(22,22)).toImage())
        tgzCount++;
  }

  if ( tgzCount == lsp.count() ){
    for(SelectedPackage sp: lsp){
      out << sp.getCompleteFileName();
    }

    m_unixCommand = new UnixCommand(this);

    QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( tgz2lzmProcessStarted()));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput() ),
                     this, SLOT( tgz2lzmProcessReadOutput() ));
    QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( tgz2lzmProcessFinished(int, QProcess::ExitStatus) ));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                     this, SLOT( tgz2lzmProcessRaisedError() ));

    if (UnixCommand::hasTheExecutable(ctn_MAKELZMBIN))
      m_unixCommand->transformTGZinLZM(out, ectn_MAKELZM);
    else if (UnixCommand::hasTheExecutable(ctn_TGZ2LZMBIN))
      m_unixCommand->transformTGZinLZM(out, ectn_TGZ2LZM);
    else if (UnixCommand::hasTheExecutable(ctn_TXZ2SBBIN))
      m_unixCommand->transformTGZinLZM(out, ectn_TXZ2SB);
  }
}

void MainWindowImpl::transformRPMinTGZ(const QString &rpmPackage){
  QStringList out;

  if (rpmPackage == ""){
    QList<SelectedPackage> lsp = getSelectedPackage();

    for(SelectedPackage sp: lsp){
      out << sp.getCompleteFileName();
    }
  }
  else
    out << rpmPackage;

  m_unixCommand = new UnixCommand(this);

  QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( rpm2tgzProcessStarted() ));
  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput() ),
                   this, SLOT( rpm2tgzProcessReadOutput() ));
  QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( rpm2tgzProcessFinished(int, QProcess::ExitStatus) ));
  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                   this, SLOT( rpm2tgzProcessRaisedError() ));

  if (UnixCommand::hasTheExecutable(ctn_RPM2TXZBIN))
    m_unixCommand->transformRPMinTXZ(out);
  else if (UnixCommand::hasTheExecutable(ctn_RPM2TGZBIN))
    m_unixCommand->transformRPMinTGZ(out);
}

void MainWindowImpl::insertInstallPackageAction(){
  if (m_unixCommand != 0) return;

  for(SelectedPackage sp: getSelectedPackage()){
    if ((m_modelTodo->findItems(sp.getCompleteFileName(), Qt::MatchRecursive).size()==0)
      && (Package::isValid(sp.getCompleteFileName())))
      m_install->appendRow(new QStandardItem(sp.getCompleteFileName()));
  }

  if (!tvTODO->isExpanded(tvTODO->currentIndex())) tvTODO->expandAll();
  if (twTODO->currentIndex()!=0) twTODO->setCurrentIndex(0);
  conditionalGotoNormalView();
}

void MainWindowImpl::insertDowngradePackageAction(){
  if (m_unixCommand != 0) return;

  for(SelectedPackage sp: getSelectedPackage()){
    if ((m_modelTodo->findItems(sp.getCompleteFileName(), Qt::MatchRecursive).size()==0)
      && (Package::isValid(sp.getCompleteFileName())))
      m_downgrade->appendRow(new QStandardItem(sp.getCompleteFileName()));
  }

  if (!tvTODO->isExpanded(tvTODO->currentIndex())) tvTODO->expandAll();
  if (twTODO->currentIndex()!=0) twTODO->setCurrentIndex(0);
  conditionalGotoNormalView();
}

void MainWindowImpl::insertRemovePackageAction(){
  if (m_unixCommand != 0) return;

  if (tvPackage->hasFocus()){
    for(SelectedPackage sp: getSelectedPackage()){
      if (((m_modelTodo->findItems(sp.getCompleteFileName(), Qt::MatchRecursive).size()==0))
        && (Package::isValid(sp.getCompleteFileName())))
        m_remove->appendRow(new QStandardItem(sp.getCompleteFileName()));
    }
  }
  else if (tvInstalledPackages->hasFocus()){
    for(SelectedPackage sp: getSelectedInstalledPackage()){
      if ((m_modelTodo->findItems(sp.getCompleteFileName(), Qt::MatchRecursive).size()==0))
        m_remove->appendRow(new QStandardItem(sp.getCompleteFileName()));
    }
  }

  if (!tvTODO->isExpanded(tvTODO->currentIndex())) tvTODO->expandAll();
  if (twTODO->currentIndex()!=0) twTODO->setCurrentIndex(0);
  conditionalGotoNormalView();
}

void MainWindowImpl::insertRemovePackageAction(QString packageName){
  if (m_modelTodo->findItems(ctn_INSTALLED_PACKAGES_DIR + packageName, Qt::MatchRecursive).size()==0)
      m_remove->appendRow(new QStandardItem(ctn_INSTALLED_PACKAGES_DIR + packageName));

  if (!tvTODO->isExpanded(tvTODO->currentIndex())) tvTODO->expandAll();
  if (twTODO->currentIndex()!=0) twTODO->setCurrentIndex(0);
  conditionalGotoNormalView();
}

void MainWindowImpl::insertUpgradePackageAction(){
  if (m_unixCommand != 0) return;
  for(SelectedPackage sp: getSelectedPackage()){
    if ((m_modelTodo->findItems(sp.getCompleteFileName(), Qt::MatchRecursive).size()==0)
      && (Package::isValid(sp.getCompleteFileName())))
      m_upgrade->appendRow(new QStandardItem(sp.getCompleteFileName()));
  }

  if (!tvTODO->isExpanded(tvTODO->currentIndex())) tvTODO->expandAll();
  if (twTODO->currentIndex()!=0) twTODO->setCurrentIndex(0);
  conditionalGotoNormalView();
}

void MainWindowImpl::insertReinstallPackageAction(){
  if (m_unixCommand != 0) return;

  for(SelectedPackage sp: getSelectedPackage()){
    if ((m_modelTodo->findItems(sp.getCompleteFileName(), Qt::MatchRecursive).size()==0)
      && (Package::isValid(sp.getCompleteFileName())))
      m_reinstall->appendRow(new QStandardItem(sp.getCompleteFileName()));
  }

  if (!tvTODO->isExpanded(tvTODO->currentIndex())) tvTODO->expandAll();
  if (twTODO->currentIndex()!=0) twTODO->setCurrentIndex(0);
  conditionalGotoNormalView();
}

void MainWindowImpl::freezePackage(){
  /*for(QModelIndex item: tvInstalledPackages->selectionModel()->selectedIndexes())
  {
    if ( item.column() == ctn_PACKAGE_NAME )
    {
      QModelIndex mi = m_proxyModelInstalledPackages->mapToSource( item );
      QStandardItem *si = m_modelInstalledPackages->item(mi.row(), mi.column());
      si->setText(si->text() + "_MUDEI!");
      qDebug() << "Selected package: " << si->text();
    }
  }*/

  int bkSize = m_frozenPkgList->count();
  for(QModelIndex item: tvInstalledPackages->selectionModel()->selectedIndexes())
  {
    if ( item.column() == ctn_PACKAGE_ICON )
    {
      QModelIndex mi = m_proxyModelInstalledPackages->mapToSource( item );
      QStandardItem *si = m_modelInstalledPackages->item(mi.row(), mi.column());
      si->setIcon(IconHelper::getIconFrozen());
      si->setText( "F" );
    }
    else if ( item.column() == ctn_PACKAGE_NAME )
    {
      qDebug() << "CHEGUEI!";
      QModelIndex mi2 = m_proxyModelInstalledPackages->mapToSource( item );
      QStandardItem *si2 = m_modelInstalledPackages->item(mi2.row(), mi2.column());
      QString pkg = si2->text();
      qDebug() << "Selected package: " << si2->text();
      pkg = Package::getBaseName(pkg);

      *m_frozenPkgList << pkg;
      //*m_frozenPkgList << Package::getBaseName( m_proxyModelInstalledPackages->data(item).toString());
    }
  }

  //refreshInstalledPackageTreeView();

  //Now we must rebuild the list of frozen packages in order to save it into the QSettings.
  m_frozenPkgList->save();
  if ( (m_modelPackage->rowCount()>0) && (bkSize != m_frozenPkgList->count()) ) showPackagesInDirectory();
}

void MainWindowImpl::unfreezePackage(){
  int bkSize = m_frozenPkgList->count();
  for(QModelIndex item: tvInstalledPackages->selectionModel()->selectedIndexes())
  {
    if ( item.column() == ctn_PACKAGE_ICON ){
      QModelIndex mi = m_proxyModelInstalledPackages->mapToSource( item );
      QStandardItem *si = m_modelInstalledPackages->item(mi.row(), mi.column());
      si->setIcon(IconHelper::getIconUnFrozen());
      si->setText("N");
    }
    else if ( item.column() == ctn_PACKAGE_NAME ){
      //qDebug() << "CHEGUEI!";
      QModelIndex mi2 = m_proxyModelInstalledPackages->mapToSource( item );
      QStandardItem *si2 = m_modelInstalledPackages->item(mi2.row(), mi2.column());
      QString pkg = si2->text();
      pkg = Package::getBaseName(pkg);
      //qDebug() << "Package to be removed " << pkg;
      m_frozenPkgList->removeAll(pkg);
    }
  }

  //refreshInstalledPackageTreeView();

  //Now we must rebuild the list of frozen packages in order to save it to the QSettings.
  m_frozenPkgList->save();
  if ( (m_modelPackage->rowCount()>0) && (bkSize != m_frozenPkgList->count()) ) showPackagesInDirectory();
}

void MainWindowImpl::_openSnapshotOfInstalledPackages(const QString snapshotFileName){
  qApp->processEvents();

  CPUIntensiveComputing *ri = new CPUIntensiveComputing;
  SnapshotList res = Package::processSnapshotOfInstalledPackageList(snapshotFileName);
  delete ri;
  QString out("");

  if (res.getDetails().count() > 0){
    for(QString pkg: res.getDetails()){
      out += pkg.append("\n");
    }
  }

  if (res.getNewPackagesList().count() > 0){
    //The list is not empty...
    QMessageBox msgBox;
    msgBox.setWindowTitle(snapshotFileName);

    if (res.getNewPackagesList().count() == 1){
      msgBox.setText(tr("There are 1 new package installed since this snapshot."));
      msgBox.setInformativeText(tr("Do you want to create remove actions for it?"));
    }
    else{
      msgBox.setText(tr("There are %1 new packages installed since this snapshot.").arg(res.getNewPackagesList().count()));
      msgBox.setInformativeText(tr("Do you want to create remove actions for them?"));
    }

    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::No);

    int result = msgBox.exec();
    //We call processEvents to avoid the dialog screen lag.
    qApp->processEvents();

    if (result == QMessageBox::Yes){
      for(QString pkg: res.getNewPackagesList()){
        insertRemovePackageAction(pkg);
      }
    }
    else if (result == QMessageBox::No){
      createTabSnapshotAnalisys(snapshotFileName, out);
    }
  }
  else if (res.getDetails().count() == 0){ //if newPackagesList is empty, we test DetailsList...
    QMessageBox::information(this, snapshotFileName, tr("There are no package changes since this snapshot!"));
  }
  else{ //There are no new installed packages, but other Details...
    createTabSnapshotAnalisys(snapshotFileName, out);
  }
}

void MainWindowImpl::openThisSnapshotOfInstalledPackages(){
  SelectedPackage sp=getSelectedPackage().at(0);
  _openSnapshotOfInstalledPackages(sp.getCompleteFileName());
}

void MainWindowImpl::openSnapshotOfInstalledPackages(){
  QString snapshotFileName = QFileDialog::getOpenFileName(this,
      tr("Open snapshot of installed packages"), m_defaultDirectory,
                                                          ("Snapshots Files (installed_packages_list*.txt)"));

  if (snapshotFileName == "") return;

  _openSnapshotOfInstalledPackages(snapshotFileName);
}

void MainWindowImpl::createTabSnapshotAnalisys(QString snapshotFileName, QString content){
  if (this->isHidden()) show();
  conditionalGotoNormalView();

  CPUIntensiveComputing ri;
  QString tabName(snapshotFileName);
  QString date;
  QString time;

  //installed_packages_list_16_11_2011_21_23_13.txt
  int p=tabName.indexOf("list_");
  if (p > 0){
    date = tabName.mid(p+5, 2) + "/" + tabName.mid(p+8, 2) + "/" + tabName.mid(p+11, 4);
    time = tabName.mid(p+16, 2) + ":" + tabName.mid(p+19, 2) + ":" + tabName.mid(p+22, 2);
  }
  tabName = "snasphot " + date + " " + time;

  QString caption = "<font face=\"helvetica\"><h3>" + tr("Changes since %1").arg(date + " - " + time) + "</h3>";
  QString header = "<font face=\"helvetica\"><div align=\"center\"><table width=\"98%\" border=\"0\" >";
    header += "<tr><td colspan=\"3\">";
    header += "<center>" + caption + "</center></td><tr><td></td>";

  QStringList lines=content.split("\n", Qt::SkipEmptyParts);

  int n=2;
  QString altColor = "\"#E5E5E5\"";  //"\"#D4DED9\"";
  QString html("");

  for(QString line: lines){
    if (line.at(0) != '=')
    {
      int div = line.indexOf("[");
      if (div < 0) continue;

      QString bgColor("");
      QString fontColor("");

      if(n % 2 == 0){
        bgColor = altColor;
        fontColor = "<font color=#000000 face=\"helvetica\">";
      }
      else{
        bgColor = ""; //"#FBFFFC";
        fontColor = "<font face=\"helvetica\">";
      }

      QString pkg = line.mid(0, div-2).trimmed();

      html += "<tr bgcolor=" + bgColor + ">" + "<td valign=\"middle\"><>" + fontColor + pkg + "</></td>" +
          "<td valign=\"middle\" align=\"center\"><>" + fontColor + line.mid(div).trimmed() +
          "</font></></td>" + "<td valign=\"middle\" align=\"center\"><>" + fontColor +
          Package::getModificationDate(pkg) + "</></td></tr>";
    }
    else if(lines.count() > 2)
    {
      //Sumary here
      html += "<tr/><tr><td colspan=\"3\" align=\"center\"><h4>" + line.remove(0,2) + "</h4></td></tr>";
    }

    n++;
  }

  createGenericTab(tabName, header + html);
}

void MainWindowImpl::createTabPkgDiff(const QString pkg, const QString installedPkg, const QString diff ){
  QString html;
  html += "<font face=\"helvetica\"><div align=\"center\"><table width=\"98%\" border=\"0\">";
  html += "<tr><td><h3>" + pkg +
          "</h3></td><td><h3>" + installedPkg + "</h3></td></tr>"
          "<tr><td></td><td></td></tr>";

  //We must first parse this diff content to produce human-readable text
  QStringList ldiff = diff.split("\n");

  int n=2;

  QString altColor = "\"#E5E5E5\""; //"\"#D4DED9\"";
  QString leftColor = "\"#164833\"";
  QString rightColor = ""; //"\"#1B1C1F\"";
  QString bgColor("");

  for(QString s: ldiff){
    if ((!s.isEmpty()) && (s.at(0) != '<') && (s.at(0) != '>')) continue;
    if (!s.isEmpty() && s.at(0) == '<'){
      if (n % 2 == 0){
        bgColor = altColor;
        leftColor = "#000000";
      }
      else{
        bgColor = ""; //"#FBFFFC";
        leftColor = "";
      }

      html += "<tr bgcolor=" + bgColor + "><td valign=\"middle\"><><font color=" + leftColor + ">" +
        s.mid(2) + "</></td><td valign=\"middle\"></td></tr>";
    }
    else if (!s.isEmpty() && s.at(0) == '>') {
      if (n % 2 == 0){
        bgColor = altColor;
        rightColor = "#000000";
      }
      else{
        bgColor = ""; //"#FBFFFC";
        rightColor = "";
      }

      html += "<tr bgcolor=" + bgColor + "><td td valign=\"middle\"></td><td valign=\"middle\">"
        "<><font color=" + rightColor + ">" + s.mid(2) + "</></td></tr>";
    }
    n++;
  }

  html += "</table></div>";
  QString aux;
  if (installedPkg.endsWith(ctn_TGZ_PACKAGE_EXTENSION) || installedPkg.endsWith(ctn_TXZ_PACKAGE_EXTENSION))
    aux = pkg + " DIFF";
  else aux = pkg + " DIFF(inst)";

  createGenericTab(aux, html);
}

/*
 * Extracts the base file name from an absolute file name.
 */
QString MainWindowImpl::_extractBaseFileName(const QString &fileName)
{
  QString baseFileName(fileName);

  if (fileName.endsWith('/'))
  {
    baseFileName.remove(baseFileName.size()-1, 1);
  }

  return baseFileName.right(baseFileName.size() - baseFileName.lastIndexOf('/') -1);
}

void MainWindowImpl::createTabPkgFileList(const QString pkgName, const QStringList fileList){
  conditionalGotoNormalView();

  QWidget *tabPkgFileList = new QWidget(this);
  QGridLayout *gridLayoutX = new QGridLayout ( tabPkgFileList );
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );
  QStandardItemModel *modelPkgFileList = new QStandardItemModel(this);
  QStandardItemModel *fakeModelPkgFileList = new QStandardItemModel(this);
  QTreeView *tvPkgFileList = new QTreeView(tabPkgFileList);
  tvPkgFileList->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tvPkgFileList->setDropIndicatorShown(false);
  tvPkgFileList->setAcceptDrops(false);
  tvPkgFileList->header()->setSortIndicatorShown(false);
  tvPkgFileList->header()->setSectionsClickable(false);
  tvPkgFileList->header()->setSectionsMovable(false);
  tvPkgFileList->setFrameShape(QFrame::NoFrame);
  tvPkgFileList->setFrameShadow(QFrame::Plain);
  tvPkgFileList->setObjectName("tvPkgFileList");
  tvPkgFileList->setStyleSheet( StrConstants::getTreeViewCSS(SettingsManager::getPkgListFontSize()) );

  SearchBar *searchBar = new SearchBar(this);

  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChangedEx(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosedEx()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNextEx()));
  connect(searchBar, SIGNAL(findPrevious()), this, SLOT(searchBarFindPreviousEx()));

  gridLayoutX->addWidget(searchBar, 1, 0, 1, 1);

  modelPkgFileList->setSortRole(0);
  modelPkgFileList->setColumnCount(0);

  gridLayoutX->addWidget ( tvPkgFileList, 0, 0, 1, 1 );
  QStandardItem* fakeRoot = fakeModelPkgFileList->invisibleRootItem();
  QStandardItem* root = modelPkgFileList->invisibleRootItem();
  QStandardItem *lastDir, *item, *lastItem=root, *parent;
  bool first=true;
  lastDir = root;

  QString fullPath;
  bool isSymLinkToDir = false;

  for( QString file: fileList )
  {
    QFileInfo fi ( file );  
    bool isDir = file.endsWith('/');
    isSymLinkToDir = false;
    QString baseFileName = _extractBaseFileName(file);

    //Let's test if it is not a symbolic link to a dir
    if(!isDir)
    {
      QFileInfo fiTestForSymLink(file);
      if(fiTestForSymLink.isSymLink())
      {
        QFileInfo fiTestForDir(fiTestForSymLink.symLinkTarget());
        isSymLinkToDir = fiTestForDir.isDir();
      }
    }

    if (isDir)
    {
      if ( first == true )
      {
        item = new QStandardItem ( IconHelper::getIconFolder(), baseFileName );
        item->setAccessibleDescription("directory " + item->text());
        fakeRoot->appendRow ( item );
      }
      else
      {
       fullPath = PackageController::showFullPathOfItem(lastDir->index());
       //if ( file.indexOf ( lastDir->text() ) != -1 ){
       if ( file.contains ( fullPath )) {
         item = new QStandardItem ( IconHelper::getIconFolder(), baseFileName);
         item->setAccessibleDescription("directory " + item->text());
         lastDir->appendRow ( item );
       }
       else
       {
         parent = lastItem->parent();
         if (parent != 0) fullPath = PackageController::showFullPathOfItem(parent->index());

         do
         {
           if ( parent == 0 || file.contains ( fullPath )) break;
           parent = parent->parent();
           if (parent != 0) fullPath = PackageController::showFullPathOfItem(parent->index());
         }
         while ( parent != fakeRoot );

         item = new QStandardItem ( IconHelper::getIconFolder(), baseFileName );
         item->setAccessibleDescription("directory " + item->text());

         if ( parent != 0 )
         {
           //std::cout << item->text().toLatin1().data() << " is son of " << fullPath.toLatin1().data() << std::endl;
           parent->appendRow ( item );
         }
         else
         {
           //std::cout << item->text().toLatin1().data() << " is son of <FAKEROOT>" << std::endl;
           fakeRoot->appendRow ( item );
         }
       }
      }
      lastDir = item;
    }
    else if (isSymLinkToDir)
    {
      item = new QStandardItem ( IconHelper::getIconFolder(), baseFileName);
      item->setAccessibleDescription("directory " + item->text());
      parent = lastDir;
      if (parent != 0) fullPath = PackageController::showFullPathOfItem(parent->index());

      do
      {
        //if (parent != 0) std::cout << "Testing if symlink" << file.toLatin1().data() << " contains " << fullPath.toLatin1().data() << std::endl;
        if ( parent == 0 || file.contains ( fullPath )) break;
        parent = parent->parent();
        if (parent != 0) fullPath = PackageController::showFullPathOfItem(parent->index());
      }
      while ( parent != fakeRoot );

      if (parent != 0)
      {
        parent->appendRow ( item );
      }
      else
      {
        fakeRoot->appendRow ( item );
      }
    }
    else
    {
      item = new QStandardItem ( IconHelper::getIconBinary(), baseFileName );
      item->setAccessibleDescription("file " + item->text());
      parent = lastDir;
      if (parent != 0) fullPath = PackageController::showFullPathOfItem(parent->index());

      do
      {
        if ( parent == 0 || file.contains(fullPath)) break;
        parent = parent->parent();
        if (parent != 0) fullPath = PackageController::showFullPathOfItem(parent->index());
      }
      while ( parent != fakeRoot );

      parent->appendRow ( item );
    }

    lastItem = item;
    first = false;
  }

  tabPkgFileList->setStatusTip(pkgName);
  QFileInfo info(pkgName);
  QString tabName(info.fileName());

  root = fakeRoot;
  fakeModelPkgFileList->sort(0);
  modelPkgFileList = fakeModelPkgFileList;
  tvPkgFileList->setModel(modelPkgFileList);
  tvPkgFileList->header()->setDefaultAlignment( Qt::AlignCenter );
  modelPkgFileList->setHorizontalHeaderLabels( QStringList() <<
                                               tr("Content of \"%1\" ( Ctrl+W to close this tab )").arg(tabName));

  QList<QStandardItem*> lit = modelPkgFileList->findItems( "/", Qt::MatchStartsWith | Qt::MatchRecursive );

  for( QStandardItem* it: lit ){
    QFileInfo fi( it->text() );
    if ( fi.isFile() == false ){
      QString s( it->text() );
      s.remove(s.size()-1, 1);
      s = s.right(s.size() - s.lastIndexOf('/') -1);
      it->setText( s );
    }
  }

  tabName = "&"+ tabName;
  int tindex = twTODO->addTab( tabPkgFileList, QApplication::translate (
      "MainWindow", tabName.toUtf8()) );
  twTODO->setTabText(twTODO->indexOf(tabPkgFileList), QApplication::translate(
      "MainWindow", tabName.toUtf8()) );

  QWidget *w = m_tabBar->tabButton(tindex, QTabBar::RightSide);
  w->setToolTip(tr("Close tab"));
  w->setObjectName("toolButton");
  twTODO->setCurrentIndex ( tindex );

  tvPkgFileList->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(tvPkgFileList, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(execContextMenuPkgFileList(QPoint)));
  connect(tvPkgFileList, SIGNAL(clicked (const QModelIndex&)),
          this, SLOT(showFullPathOfObject(const QModelIndex&)));
  connect(tvPkgFileList, SIGNAL(doubleClicked (const QModelIndex&)),
          this, SLOT(openFileOrDirectory(const QModelIndex&)));
  connect(tvPkgFileList, SIGNAL(pressed (const QModelIndex&)),
          tvPkgFileList, SIGNAL(clicked (const QModelIndex&)));
  connect(tvPkgFileList, SIGNAL(activated(const QModelIndex)), tvPkgFileList,
          SIGNAL(clicked(const QModelIndex)));
}

void MainWindowImpl::createGenericTab(QString tabName, QString tabContents){
  if (this->isHidden())
    show();

  conditionalGotoNormalView();

  //First, verify if tab already exists (with the EXACT content)
  for (int c=2; c<twTODO->count(); c++)
    if (twTODO->tabText(c) == tabName){
      twTODO->setCurrentIndex(c);
      QTextBrowser *tb = twTODO->currentWidget()->findChild<QTextBrowser*>("textBrowser");
      if (tb){
        QString aux = tabContents;
        QString old = tb->toPlainText();
        QRegExp re("<.*>", Qt::CaseInsensitive, QRegExp::RegExp2);
        re.setMinimal(true);

        aux.remove(re);
        aux.remove("\n");
        old.remove("\n");
        aux.replace("&lt;", "<");

        if (old == aux) return;
        else{
          //if the contents are not equal, let's remove the tab and go ahead...
          twTODO->removeTab(c);
        }
      }
      else return;
    }

  QWidget *tabObject = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout ( tabObject );
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );

  QTextBrowser *text = new QTextBrowser(tabObject);
  MyHighlighter *highlighter = new MyHighlighter(text, "");
  text->setObjectName("textBrowser");
  text->setReadOnly(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  text->setOpenExternalLinks(true);
  gridLayoutX->addWidget(text, 0, 0, 1, 1);

  SearchBar *searchBar = new SearchBar(this);
  text->setHtml(tabContents);

  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChanged(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosed()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNext()));
  connect(searchBar, SIGNAL(findPrevious()), this, SLOT(searchBarFindPrevious()));

  gridLayoutX->addWidget(searchBar, 1, 0, 1, 1);
  gridLayoutX->addWidget(new SyntaxHighlighterWidget(this, highlighter));

  text->show();

  int tindex = twTODO->addTab( tabObject, QApplication::translate (
      "MainWindow", tabName.toUtf8()) );
  twTODO->setTabText(twTODO->indexOf(tabObject), QApplication::translate(
      "MainWindow", tabName.toUtf8()) );

  QWidget *w = m_tabBar->tabButton(tindex, QTabBar::RightSide);
  w->setToolTip(tr("Close tab"));
  w->setObjectName("toolButton");
  twTODO->setCurrentIndex ( tindex );

  text->setFocus();
}

void MainWindowImpl::takeSnapshotOfInstalledPackages(DumpInstalledPackageListOptions options){
  conditionalGotoNormalView();

  twTODO->setCurrentIndex(1);
  QString msg;
  textEdit->clear();
  msg = "<h4>" + tr("Please, wait while the snapshot of installed packages is being generated...") + "</h4><br>";
  textEdit->insertHtml(msg);
  QString fileName = Package::dumpInstalledPackageList(options);
  msg = "<h4>" + tr("Finished generating the snapshot of installed packages!") + "</h4><br>" +
      "<br>" + tr("The name of the snapshot is: \"%1\"").arg(fileName) + "<br><br>" +
      StrConstants::getForMoreInformation();

  textEdit->insertHtml(msg);
  textEdit->ensureCursorVisible();
}
