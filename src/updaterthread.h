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

#ifndef UPDATERTHREAD_H
#define UPDATERTHREAD_H

#include "updater.h"
#include <QThread>
#include <QObject>
#include <QProcess>
#include <QFile>

const int ctn_PING_TIMER_INTERVAL(15000);

class UpdaterThread : public QThread {
  Q_OBJECT

private:
  int m_numberOfDownloadedUpdates;

protected:
  Updater *m_updater;

public:
  UpdaterThread();
  ~UpdaterThread();

  void run();

  inline QString getLastErrorMessage(){ return m_updater->getLastErrorMessage(); }
  inline int getNumberOfUpdates(){ return m_updater->getNumberOfUpdatesAvailable(); }
  inline int getNumberOfDownloadedUpdates(){ return m_numberOfDownloadedUpdates; }

  inline QString getListOfAvailableUpdates(){ return m_updater->getListOfAvailableUpdates(); }

signals:
  void unknownSlackwareVersion();
  void noInternetAvailable();
  void noPingBinFound();
  void noCurlBinFound();
  void noGpgBinFound();
  void slackwareMirrorDown();
  void undefinedError();

  void numberOfUpdatesChanged(int value);
  void verifySlackwareSignatureChanged();
  void slackwareSignatureToDownloadChanged();
  void slackwareSignatureToInstallChanged();

  void packageListToDownloadChanged();
  void checksumsToDownloadChanged();
  void packageListChecksumChanged(bool value);
  void packageToDownloadChanged(QString value);
  void signatureOfPackageToDownloadChanged(QString value);
  void packageSignatureCheckingChanged(bool value);

  void curlDownloadStatusChanged(int);
  void curlDownloadProgressChanged(int);
};

class AutoCheckUpdatesAvailableThread: public QThread {
  Q_OBJECT

private:
  Updater *m_updater;

public:
  void run();

signals:
  void checkNumberOfUpdatesChanged(int value);
};

class ReinstallSlackGPGKeyThread : public UpdaterThread {
  Q_OBJECT

public:
  void run();
};

class CurlThread : public QThread {
  Q_OBJECT

private:
  QFile *tempFile;
  QString m_commandToRun;
  QString m_result;
  QProcess *m_curlProcess;

public:
  inline void setCommandToRun(QString commandToRun){ m_commandToRun = commandToRun; }
  inline QString getResult(){ return m_result; }

  ~CurlThread();

protected:
  void run();

private slots:
  void onCurlReadyReadStandardOutput();
  void onCurlDownloadFinished(int);

  void onPingTimer();

signals:
  void curlDownloadProgressChanged(int);
};

#endif // UPDATERTHREAD_H
