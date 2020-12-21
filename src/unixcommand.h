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
#ifndef UNIXCOMMAND_H
#define UNIXCOMMAND_H

#include <QObject>
#include <QProcess>
#include <QTime>
#include <QRandomGenerator>
#include <unistd.h>
#include "package.h"

const QString ctn_XZ_TAR_ERROR("/usr/bin/tar: This does not look like a tar archive\n"
                               "xz: (stdin): File format not recognized\n"
                               "/usr/bin/tar: Child returned status 1\n"
                               "/usr/bin/tar: Error is not recoverable: exiting now\n");

const QString ctn_GZ_TAR_ERROR("/usr/bin/tar: This does not look like a tar archive\n\n"
                               "gzip: stdin: not in gzip format\n"
                               "/usr/bin/tar: Child returned status 1\n"
                               "/usr/bin/tar: Error is not recoverable: exiting now\n");

const QString ctn_COMMAND_NOT_FOUND("command not found");
const QString ctn_MAKELZMBIN("make-lzm");
const QString ctn_RPM2TGZBIN("rpm2tgz");
const QString ctn_RPM2TXZBIN("rpm2txz");
const QString ctn_TGZ2LZMBIN("tgz2lzm");
const QString ctn_TXZ2SBBIN("txz2sb");

const QString ctn_PACKAGES_WITH_SAME_CONTENT("The packages have the same content!");
const QString ctn_AUTOMATIC("automatic");

enum LZMCommand { ectn_MAKELZM, ectn_TGZ2LZM, ectn_TXZ2SB };

//Forward class declarations.
class QString;
class QStringList;

class UnixCommand : public QObject{
  Q_OBJECT

  private:    
    QString m_readAllStandardOutput;
    QString m_readAllStandardError;
    QString m_errorString;
    QProcess *m_process;
    static QFile *m_temporaryFile;

  public:
    UnixCommand(QObject *parent);

    //Delegations from Package class (due to QProcess use)
    static QString runCommand(const QString& commandToRun);
    static QString runCurlCommand(const QString& commandToRun);
    static QString discoverBinaryPath(const QString&);
    static void installSlackGPGKey(const QString& gpgKeyPath);
    static bool isSpkgInstalled();

    static QByteArray getPackageInformation(const QString &pkgName, bool installed);
    static QByteArray getPackageContents(const QString &pkgName);
    static QString getSlackArchitecture();

    static bool isSlackPackage( const QString& filePath );
    static bool isAuthenticPackage(const QString& packageName);
    static bool hasInternetConnection();
    static bool hasSlackGPGKeyInstalled(QString slackVersion);
    static bool isTextFile( const QString& fileName ); //fileName is Path + Name

    static bool isKtsussVersionOK();
    static bool hasTheExecutable( const QString& exeName );

    static bool isRootRunning(){
      int uid = geteuid();
      return (uid == 0); //Returns TRUE if root is running QTGZ
    }

    static QFile* getTemporaryFile(){
      //QTime time = QTime::currentTime();
      quint32 gen = QRandomGenerator::global()->generate();
      m_temporaryFile = new QFile(ctn_TEMP_ACTIONS_FILE + QString::number(gen));
      m_temporaryFile->open(QIODevice::ReadWrite|QIODevice::Text);
      m_temporaryFile->setPermissions(QFile::ExeOwner|QFile::ReadOwner);

      return m_temporaryFile;
    }

    static void removeTemporaryActionFile(){
      if (m_temporaryFile != 0){
        m_temporaryFile->close();
        m_temporaryFile->remove();
        delete m_temporaryFile;
      }
    }

    static void removeTemporaryFiles();

    static QString executeDiffToEachOther( QString pkg1, QString pkg2 );
    static QString executeDiffToInstalled( QString pkg, QString installedPackage );
    static QString getPkgInstallCommand();
    static QString getPkgUpgradeCommand();
    static QString getPkgRemoveCommand();
    static QString getPkgReinstallCommand();

    void executePackageActions(const QStringList& commandList);
    void transformTGZinLZM(const QStringList& commandList, LZMCommand commandUsed);   
    void transformRPMinTGZ(const QStringList& commandList);
    void transformRPMinTXZ(const QStringList& commandList);

    QString readAllStandardOutput();
    QString readAllStandardError();
    QString errorString();

  public slots:
    void processReadyReadStandardOutput();
    void processReadyReadStandardError();

  signals:
    void started();
    void readyReadStandardOutput();
    void finished ( int, QProcess::ExitStatus );
    void readyReadStandardError();
};

#endif // UNIXCOMMAND_H
