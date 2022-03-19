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
#include "package.h"
#include "tvpackagesitemdelegate.h"
#include "argumentlist.h"
#include "strconstants.h"
#include "searchbar.h"

#include <QMessageBox>

void MainWindowImpl::execAutoCheckUpdatesAvailable(){
  m_autoCheckUpdatesThread = new AutoCheckUpdatesAvailableThread();

  connect(m_autoCheckUpdatesThread,
          SIGNAL(checkNumberOfUpdatesChanged(int)), this, SLOT(threadAutoCheckUpdatesAvailableReady(int)));

  m_autoCheckUpdatesThread->start(QThread::LowPriority);
}

void MainWindowImpl::threadAutoCheckUpdatesAvailableReady(int numberOfAvailableUpdates){
  if (numberOfAvailableUpdates > 0){
    QString message;

    if (numberOfAvailableUpdates == 1)
      message = tr("There is one new patch available!") + "\n\n" + tr("Do you want to download it now?");
    else
      message = tr("There are %1 new patches available!").arg(numberOfAvailableUpdates) +
          "\n\n" + tr("Do you want to download them now?");

    /* This is needed to prevent the hiden window (if it's hidden!)
       to close the application, when the questionBox is closed! */
    if (qApp->quitOnLastWindowClosed())
      qApp->setQuitOnLastWindowClosed(false);

    int rep = QMessageBox::question(this,
                                    tr("Confirmation"), message,
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::No);

    //And then, we return to the default setQuitOnLasWindowClosed state!
    qApp->setQuitOnLastWindowClosed(!SettingsManager::getWindowCloseHidesApp());

    if (rep == QMessageBox::Yes) startThreadUpdater();
  }
}

void MainWindowImpl::startThreadReinstallSlackGPGKey(){
  if (m_updaterThread && m_updaterThread->isRunning()) return;

  m_reinstallSlackGPGKeyThread = new ReinstallSlackGPGKeyThread();

  connect(m_reinstallSlackGPGKeyThread, SIGNAL(started()), this, SLOT(threadReinstallSlackGPGKeyStarted()));
  connect(m_reinstallSlackGPGKeyThread, SIGNAL(finished()), this, SLOT(threadReinstallSlackGPGKeyFinished()));

  connect(m_reinstallSlackGPGKeyThread, SIGNAL(unknownSlackwareVersion()),
          this, SLOT(threadUpdaterUnknownSlackwareVersion()));

  connect(m_reinstallSlackGPGKeyThread, SIGNAL(slackwareSignatureToDownloadChanged()),
          this, SLOT(threadUpdaterSlackwareSignatureToDownload()));
  connect(m_reinstallSlackGPGKeyThread, SIGNAL(slackwareSignatureToInstallChanged()),
          this, SLOT(threadUpdaterSlackwareSignatureToInstall()));

  connect(m_reinstallSlackGPGKeyThread, SIGNAL(noInternetAvailable()),
          this, SLOT(threadUpdaterNoInternetAvailable()));

  connect(m_reinstallSlackGPGKeyThread, SIGNAL(noPingBinFound()),
          this, SLOT(threadUpdaterNoPingBinFound()));

  connect(m_reinstallSlackGPGKeyThread, SIGNAL(noCurlBinFound()),
          this, SLOT(threadUpdaterNoCurlBinFound()));

  connect(m_reinstallSlackGPGKeyThread, SIGNAL(noGpgBinFound()),
          this, SLOT(threadUpdaterNoGpgBinFound()));

  connect(m_reinstallSlackGPGKeyThread, SIGNAL(slackwareMirrorDown()),
          this, SLOT(threadUpdaterSlackwareMirrorDown()));

  connect(m_reinstallSlackGPGKeyThread, SIGNAL(undefinedError()),
          this, SLOT(threadReinstallSlackGPGKeyUndefinedError()));

  m_reinstallSlackGPGKeyThread->start(QThread::LowPriority);
}

void MainWindowImpl::startThreadUpdater(){
  if (m_reinstallSlackGPGKeyThread && m_reinstallSlackGPGKeyThread->isRunning())
    return;
  else if (m_updaterThread && m_updaterThread->isRunning()){
    m_updaterThread->wait(1000);
    m_updaterThread->terminate();
    m_updaterThread->wait(1000);

    insertUpdaterText("<br><b><font color=\"red\">" +
                      tr("Slackware patches checking was cancelled!") + "</font></b><br>");

    actionUpdater->setText(tr("Check for Slackware patches..."));
    return;
  }

  actionUpdater->setText(tr("Cancel Slackware patches checking!"));

  m_updaterThread = new UpdaterThread();

  connect(m_updaterThread, SIGNAL(started()), this, SLOT(threadUpdaterStarted()));
  connect(m_updaterThread, SIGNAL(finished()), this, SLOT(threadUpdaterFinished()));

  connect(m_updaterThread, SIGNAL(unknownSlackwareVersion()),
          this, SLOT(threadUpdaterUnknownSlackwareVersion()));
  connect(m_updaterThread, SIGNAL(verifySlackwareSignatureChanged()),
          this, SLOT(threadUpdaterVerifySlackwareSignature()));
  connect(m_updaterThread, SIGNAL(slackwareSignatureToDownloadChanged()),
          this, SLOT(threadUpdaterSlackwareSignatureToDownload()));
  connect(m_updaterThread, SIGNAL(slackwareSignatureToInstallChanged()),
          this, SLOT(threadUpdaterSlackwareSignatureToInstall()));

  connect(m_updaterThread, SIGNAL(packageListToDownloadChanged()),
          this, SLOT(threadUpdaterPackageListToDownload()));
  connect(m_updaterThread, SIGNAL(checksumsToDownloadChanged()),
          this, SLOT(threadUpdaterChecksumsToDownload()));
  connect(m_updaterThread, SIGNAL(packageListChecksumChanged(bool)),
          this, SLOT(threadUpdaterPackageListChecksum(bool)));
  connect(m_updaterThread, SIGNAL(numberOfUpdatesChanged(int)),
          this, SLOT(threadUpdaterNumberOfUpdates(int)));

  connect(m_updaterThread, SIGNAL(packageToDownloadChanged(QString)),
          this, SLOT(threadUpdaterPackageToDownload(QString)));
  connect(m_updaterThread, SIGNAL(signatureOfPackageToDownloadChanged(QString)),
          this, SLOT(threadUpdaterSignatureOfPackageToDownload(QString)));
  connect(m_updaterThread, SIGNAL(packageSignatureCheckingChanged(bool)),
          this, SLOT(threadUpdaterPackageSignatureChecking(bool)));

  connect(m_updaterThread, SIGNAL(noInternetAvailable()),
          this, SLOT(threadUpdaterNoInternetAvailable()));

  connect(m_updaterThread, SIGNAL(noPingBinFound()),
          this, SLOT(threadUpdaterNoPingBinFound()));

  connect(m_updaterThread, SIGNAL(noCurlBinFound()),
          this, SLOT(threadUpdaterNoCurlBinFound()));

  connect(m_updaterThread, SIGNAL(noGpgBinFound()),
          this, SLOT(threadUpdaterNoGpgBinFound()));

  connect(m_updaterThread, SIGNAL(slackwareMirrorDown()),
          this, SLOT(threadUpdaterSlackwareMirrorDown()));

  connect(m_updaterThread, SIGNAL(curlDownloadStatusChanged(int)),
          this, SLOT(threadUpdaterCurlDownloadStatus(int)));

  connect(m_updaterThread, SIGNAL(undefinedError()),
          this, SLOT(threadUpdaterUndefinedError()));

  connect(m_updaterThread, SIGNAL(curlDownloadProgressChanged(int)),
          this, SLOT(threadUpdaterCurlDownloadStatus(int)));

  m_updaterThread->start(QThread::LowPriority);
  m_nowDownloading = 0;
}

void MainWindowImpl::createTabUpdater(){
  conditionalGotoNormalView();

  QTextBrowser *updaterOutput = getUpdaterOutput();
  if (updaterOutput != 0){
    updaterOutput->clear();
    return;
  }

  QWidget *tabUpdater = new QWidget(this);
  QGridLayout *gridLayoutX = new QGridLayout ( tabUpdater );
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );
  updaterOutput = new QTextBrowser(tabUpdater);
  updaterOutput->setObjectName("updaterOutput");
  updaterOutput->setReadOnly(true);
  updaterOutput->setFrameShape(QFrame::NoFrame);
  updaterOutput->setFrameShadow(QFrame::Plain);
  updaterOutput->setOpenLinks(false);
  gridLayoutX->addWidget ( updaterOutput, 0, 0, 1, 1 );

  connect(updaterOutput, SIGNAL(anchorClicked(QUrl)), this, SLOT(outputTextBrowserAnchorClicked(QUrl)));

  QString aux(StrConstants::getUpdaterTabTitle());

  int tindex = twTODO->addTab( tabUpdater, QApplication::translate (
      "MainWindow", aux.toUtf8()) );
  twTODO->setTabText(twTODO->indexOf(tabUpdater), QApplication::translate(
      "MainWindow", aux.toUtf8()) );
  twTODO->setCurrentIndex ( tindex );

  SearchBar *searchBar = new SearchBar(this);
  MyHighlighter *highlighter = new MyHighlighter(updaterOutput, "");

  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChanged(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosed()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNext()));
  connect(searchBar, SIGNAL(findPrevious()), this, SLOT(searchBarFindPrevious()));
  //connect(searchBar, SIGNAL(findNextButtonClicked()), this, SLOT(searchBarFindNext()));
  //connect(searchBar, SIGNAL(findPreviousButtonClicked()), this, SLOT(searchBarFindPrevious()));

  gridLayoutX->addWidget(searchBar, 1, 0, 1, 1);
  gridLayoutX->addWidget(new SyntaxHighlighterWidget(this, highlighter));

  QWidget *w = m_tabBar->tabButton(tindex, QTabBar::RightSide);
  w->setToolTip(tr("Close tab"));
  w->setObjectName("toolButton");
}

QTextBrowser* MainWindowImpl::getUpdaterOutput(){
  int c;

  for (c=2; c<twTODO->count(); c++){
    if (twTODO->tabText(c).indexOf(StrConstants::getUpdaterTabTitle()) > -1){
      twTODO->setCurrentIndex ( c );
      break;
    }
  }

  twTODO->setTabText(c, StrConstants::getUpdaterTabTitle());
  return (twTODO->widget(c)->findChild<QTextBrowser*>("updaterOutput"));
}

void MainWindowImpl::threadReinstallSlackGPGKeyStarted(){
  createTabUpdater();
  insertUpdaterText("<h4><b>" + tr("Please, wait while Slackware's GPG Key is installed.") + "</h4><br>");
}

void MainWindowImpl::threadReinstallSlackGPGKeyFinished(){
  conditionalGotoNormalView();

  delete m_reinstallSlackGPGKeyThread;
  m_reinstallSlackGPGKeyThread = 0;

  insertUpdaterText("<br><h4>" + tr("Finished installing Slackware's GPG Key.") + "</h4><br>" +
                    StrConstants::getForMoreInformation());
}

void MainWindowImpl::threadUpdaterStarted(){
  createTabUpdater();
  insertUpdaterText("<h4>" + tr("Please, wait while Slackware's official patches are being verified...") + "</h4><br>");
}

void MainWindowImpl::threadUpdaterFinished(){
  conditionalGotoNormalView();

  //html += QLatin1String("<tr><td><a href=\"goto:") + pkg + QLatin1String("\">") + pkg +
  QString updaterDir = "<a href=\"goto:patches_dir\"\\>" + Updater::getUpdaterDirectory();

  if (m_updaterThread->getNumberOfUpdates() == 1 &&
           m_updaterThread->getNumberOfDownloadedUpdates() == 1)
    insertUpdaterText("<br>" + tr("There is a new patch in directory %1").arg(updaterDir));
  else if (m_updaterThread->getNumberOfUpdates() > 1 &&
           m_updaterThread->getNumberOfDownloadedUpdates() == m_updaterThread->getNumberOfUpdates())
    insertUpdaterText("<br>" + tr("There are new patches in directory %1").arg(updaterDir));

  delete m_updaterThread;
  m_updaterThread = 0;

  insertUpdaterText("<br><h4><b>" + tr("Finished verifying Slackware's official patches!") + "</h4><br>" +
                    StrConstants::getForMoreInformation());

  actionUpdater->setText(tr("Check for Slackware patches..."));
}

void MainWindowImpl::threadUpdaterUnknownSlackwareVersion(){
  insertUpdaterText("<br><b><font color=\"red\">" + tr("Slackware version not supported!") + "</font></b><br>");
}

void MainWindowImpl::threadUpdaterNoInternetAvailable(){
  insertUpdaterText("<br><b><font color=\"red\">" + tr("Internet connection could not be stablished.") + "</font></b><br>");
}

void MainWindowImpl::threadUpdaterNoPingBinFound(){
  insertUpdaterText("<br><b><font color=\"red\">" + tr("Ping utility could not be found.") + "</font></b><br>");
}

void MainWindowImpl::threadUpdaterNoCurlBinFound(){
  insertUpdaterText("<br><b><font color=\"red\">" + tr("Curl utility could not be found.") + "</font></b><br>");
}

void MainWindowImpl::threadUpdaterNoGpgBinFound(){
  insertUpdaterText("<br><b><font color=\"red\">" + tr("Gpg utility could not be found.") + "</font></b><br>");
}

void MainWindowImpl::threadUpdaterSlackwareMirrorDown(){
  insertUpdaterText("<br><br><b><font color=\"red\">" +
                    tr("It was not possible to connect to the selected slackware mirror. Please, try another one.") +
                    "</font></b><br>");
}

void MainWindowImpl::threadUpdaterUndefinedError(){
  insertUpdaterText("<br><br><b><font color=\"red\">" +
                       m_updaterThread->getLastErrorMessage() + "</font></b>");
}

void MainWindowImpl::threadReinstallSlackGPGKeyUndefinedError(){
  insertUpdaterText("<br><br><b><font color=\"red\">" +
                       m_reinstallSlackGPGKeyThread->getLastErrorMessage() + "</font></b>");
}

void MainWindowImpl::threadUpdaterVerifySlackwareSignature(){
  insertUpdaterText(tr("Verifying if Slackware's GPG-KEY is installed...") + "<br>");
}

void MainWindowImpl::threadUpdaterSlackwareSignatureToDownload(){
  insertUpdaterText(tr("Downloading Slackware's GPG-KEY...") + "<br>");
}

void MainWindowImpl::threadUpdaterSlackwareSignatureToInstall(){
  insertUpdaterText(tr("Installing Slackware's GPG-KEY...") + "<br>");
}

void MainWindowImpl::threadUpdaterPackageListToDownload(){
  insertUpdaterText(tr("Downloading PACKAGES.TXT..."));
}

void MainWindowImpl::threadUpdaterChecksumsToDownload(){
  insertUpdaterText("<br>" + tr("Downloading CHECKSUMS.md5..."));
}

void MainWindowImpl::threadUpdaterPackageListChecksum(bool value){
  if (value){
    insertUpdaterText("<br><i>" + tr("PACKAGES.TXT MD5 hash is ok.") + "</i><br>");
  } else{
    insertUpdaterText("<br><i>" + tr("PACKAGES.TXT MD5 hash is wrong!") + "</i><br>");
  }
}

void MainWindowImpl::threadUpdaterNumberOfUpdates(int value){
  if (value == 1)
    insertUpdaterText("<br><br>" + tr("There is one new patch:"));
  else if (value > 1){
    QString s ("<br><br>" + tr("There are %1 new patches:").arg(value));
    insertUpdaterText(s);
  }
  else
    insertUpdaterText("<br>" + tr("There are no new patches available.") + "<br>");

  if (value >= 1)
    insertUpdaterText(m_updaterThread->getListOfAvailableUpdates());
}

void MainWindowImpl::threadUpdaterPackageToDownload(QString value){
  m_nowDownloading++;

  const QString s(tr("Downloading package %1...").arg(value) +
                  " (" + QString::number(m_nowDownloading) + "/" +
                  QString::number(m_updaterThread->getNumberOfUpdates()) + ")<br>");
  insertUpdaterText(s);
}

void MainWindowImpl::threadUpdaterCurlDownloadStatus(int curlDownloadStatus){
  int c=0;

  for (c=2; c<twTODO->count(); c++){
    if (twTODO->tabText(c).indexOf(StrConstants::getUpdaterTabTitle()) > -1){
      break;
    }
  }

  QString text = QString::number(curlDownloadStatus) + "%";
  twTODO->setTabText(c, StrConstants::getUpdaterTabTitle() + " (" + text + ")");
}

void MainWindowImpl::threadUpdaterSignatureOfPackageToDownload(QString value){
  QString s(tr("Downloading signature %1...").arg(value) +
            " (" + QString::number(m_nowDownloading) + "/" +
            QString::number(m_updaterThread->getNumberOfUpdates()) + ")<br>");

  insertUpdaterText(s);
}

void MainWindowImpl::threadUpdaterPackageSignatureChecking(bool value){
  if (value){
    insertUpdaterText("<i>" + tr("Package's signature is ok.") + "</i><br>");
  } else{
    insertUpdaterText("<i>" + tr("Package's signature is wrong!") + "</i><br>");
  }
}

void MainWindowImpl::insertUpdaterText(const QString &text){
  QTextBrowser *t = getUpdaterOutput();

  if (!t){
    createTabUpdater();
    t = getUpdaterOutput();
  }

  //First, we position QTextCursor at end...
  QTextCursor tc = t->textCursor();
  tc.clearSelection();
  tc.movePosition(QTextCursor::End);
  t->setTextCursor(tc);

  //Then, we may insert new text
  t->insertHtml(text);
  t->ensureCursorVisible();
}
