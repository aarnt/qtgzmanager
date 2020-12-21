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

#include "updaterthread.h"
#include "unixcommand.h"

#include <QTextStream>
#include <QTimer>

UpdaterThread::UpdaterThread(){
  m_updater = new Updater();

  connect(m_updater, SIGNAL(unknownSlackwareVersion()),
          this, SIGNAL(unknownSlackwareVersion()));

  connect(m_updater, SIGNAL(verifySlackwareSignatureChanged()),
          this, SIGNAL(verifySlackwareSignatureChanged()));

  connect(m_updater, SIGNAL(slackwareSignatureToDownloadChanged()),
          this, SIGNAL(slackwareSignatureToDownloadChanged()));

  connect(m_updater, SIGNAL(slackwareSignatureToInstallChanged()),
          this, SIGNAL(slackwareSignatureToInstallChanged()));

  connect(m_updater, SIGNAL(packageListToDownloadChanged()),
          this, SIGNAL(packageListToDownloadChanged()));

  connect(m_updater, SIGNAL(checksumsToDownloadChanged()),
          this, SIGNAL(checksumsToDownloadChanged()));

  connect(m_updater, SIGNAL(packageListChecksumChanged(bool)),
          this, SIGNAL(packageListChecksumChanged(bool)));

  connect(m_updater, SIGNAL(numberOfUpdatesChanged(int)),
          this, SIGNAL(numberOfUpdatesChanged(int)));

  connect(m_updater, SIGNAL(packageToDownloadChanged(QString)),
          this, SIGNAL(packageToDownloadChanged(QString)));

  connect(m_updater, SIGNAL(signatureOfPackageToDownloadChanged(QString)),
          this, SIGNAL(signatureOfPackageToDownloadChanged(QString)));

  connect(m_updater, SIGNAL(packageSignatureCheckingChanged(bool)),
          this, SIGNAL(packageSignatureCheckingChanged(bool)));

  connect(m_updater, SIGNAL(noInternetAvailable()),
          this, SIGNAL(noInternetAvailable()));

  connect(m_updater, SIGNAL(noPingBinFound()),
          this, SIGNAL(noPingBinFound()));

  connect(m_updater, SIGNAL(noCurlBinFound()),
          this, SIGNAL(noCurlBinFound()));

  connect(m_updater, SIGNAL(noGpgBinFound()),
          this, SIGNAL(noGpgBinFound()));

  connect(m_updater, SIGNAL(slackwareMirrorDown()),
          this, SIGNAL(slackwareMirrorDown()));

  connect(m_updater, SIGNAL(curlDownloadStatusChanged(int)),
          this, SIGNAL(curlDownloadStatusChanged(int)));

  connect(m_updater, SIGNAL(curlDownloadProgressChanged(int)),
          this, SIGNAL(curlDownloadProgressChanged(int)));

  connect(m_updater, SIGNAL(undefinedError()),
          this, SIGNAL(undefinedError()));
}

UpdaterThread::~UpdaterThread(){
  if (m_updater != 0)
    delete m_updater;
}

void UpdaterThread::run(){
  Problem result = m_updater->initialize();

  int numberOfUpdatesAvailable = m_updater->getNumberOfUpdatesAvailable();

  if (result == ectn_NONE){
    if (numberOfUpdatesAvailable > 0){
      m_numberOfDownloadedUpdates = m_updater->downloadAllUpdates();
    }
  }
}

void AutoCheckUpdatesAvailableThread::run(){
  m_updater = new Updater();

  emit checkNumberOfUpdatesChanged(m_updater->autoCheckUpdatesAvailable());
}

void ReinstallSlackGPGKeyThread::run(){
  m_updater = new Updater();

  connect(m_updater, SIGNAL(unknownSlackwareVersion()),
          this, SIGNAL(unknownSlackwareVersion()));

  connect(m_updater, SIGNAL(verifySlackwareSignatureChanged()),
          this, SIGNAL(verifySlackwareSignatureChanged()));

  connect(m_updater, SIGNAL(slackwareSignatureToDownloadChanged()),
          this, SIGNAL(slackwareSignatureToDownloadChanged()));

  connect(m_updater, SIGNAL(slackwareSignatureToInstallChanged()),
          this, SIGNAL(slackwareSignatureToInstallChanged()));

  connect(m_updater, SIGNAL(noInternetAvailable()),
          this, SIGNAL(noInternetAvailable()));

  connect(m_updater, SIGNAL(noPingBinFound()),
          this, SIGNAL(noPingBinFound()));

  connect(m_updater, SIGNAL(noCurlBinFound()),
          this, SIGNAL(noCurlBinFound()));

  connect(m_updater, SIGNAL(noGpgBinFound()),
          this, SIGNAL(noGpgBinFound()));

  connect(m_updater, SIGNAL(curlDownloadStatusChanged(int)),
          this, SIGNAL(curlDownloadStatusChanged(int)));

  connect(m_updater, SIGNAL(undefinedError()),
          this, SIGNAL(undefinedError()));

  Problem result = m_updater->preInitialize();

  if (result == ectn_NONE)
    m_updater->reinstallSlackGPGKey();
}

void CurlThread::run(){
  tempFile = UnixCommand::getTemporaryFile();
  tempFile->setPermissions(QFile::WriteOwner | QFile::ReadOwner | QFile::ExeOwner);

  QTextStream out(tempFile);
  out << m_commandToRun;
  tempFile->flush();
  tempFile->close();

  m_curlProcess = new QProcess;

  connect(m_curlProcess, SIGNAL(readyReadStandardOutput()),
          this, SLOT(onCurlReadyReadStandardOutput()), Qt::DirectConnection);
  connect(m_curlProcess, SIGNAL(readyRead()),
          this, SLOT(onCurlReadyReadStandardOutput()));
  connect(m_curlProcess, SIGNAL(readyReadStandardError()),
          this, SLOT(onCurlReadyReadStandardOutput()), Qt::DirectConnection);
  connect(m_curlProcess, SIGNAL(finished(int)), this, SLOT(onCurlDownloadFinished(int)), Qt::DirectConnection);

#if QT_VERSION >= 0x040600
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LANG", "us_EN");
  m_curlProcess->setProcessEnvironment(env);
#endif

  m_curlProcess->setProcessChannelMode(QProcess::MergedChannels);

  QTimer *pingTimer = new QTimer();
  connect(pingTimer, SIGNAL(timeout()), this, SLOT(onPingTimer()), Qt::DirectConnection);
  pingTimer->start(ctn_PING_TIMER_INTERVAL);

  QStringList params;
  params << "-c";
  params << tempFile->fileName();
  m_curlProcess->start("/bin/sh", params);
  m_curlProcess->waitForStarted(-1);

  exec();
  delete pingTimer;
}

/*
  Each ctn_PING_TIMER_INTERVAL miliseconds
  we check to see if our internet connection is OK.

  If it is not, we kill the CURL process!
*/
void CurlThread::onPingTimer(){
  if (!UnixCommand::hasInternetConnection()){
    m_curlProcess->kill();
  }
}

void CurlThread::onCurlReadyReadStandardOutput(){
  QString output = m_curlProcess->readAll();
  int p = output.indexOf(QRegExp("\\d{1,3}\\.\\d\%"));
  int p2 = output.indexOf(".");

  if (p < 0 || p2 < 0) return;
  output = output.mid(p, p2-p);

  emit curlDownloadProgressChanged(output.toInt());
}

void CurlThread::onCurlDownloadFinished(int exitCode){
  if (exitCode == 6 || m_curlProcess->exitStatus() == QProcess::CrashExit){
    m_result = ctn_CURL_COULDNT_RESOLVE_HOST;
    }
  else if (exitCode == 7){
    m_result = ctn_CURL_CONNECTION_ERROR;
  }

  m_curlProcess->close();
  tempFile->remove();
  quit();
}

CurlThread::~CurlThread(){
  if (m_curlProcess != 0)
    delete m_curlProcess;
}
