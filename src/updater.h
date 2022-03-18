/*=
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

#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QString>
#include <QStringList>

const int ctn_SIZE_OF_CHECKSUM_HASH = 32;
const QString ctn_SLACKWARE_VERSION_FILE = "/etc/slackware-version";

const QString ctn_SLACKWARE_15_0_VERSION_NAME  = "Slackware 15.0";
const QString ctn_SLACKWARE_14_2_VERSION_NAME  = "Slackware 14.2";
const QString ctn_SLACKWARE_14_1_VERSION_NAME  = "Slackware 14.1";
const QString ctn_SLACKWARE_14_0_VERSION_NAME  = "Slackware 14.0";
const QString ctn_SLACKWARE_13_37_VERSION_NAME = "Slackware 13.37.0";
const QString ctn_SLACKWARE_13_1_VERSION_NAME  = "Slackware 13.1.0";
const QString ctn_SLACKWARE_13_0_VERSION_NAME  = "Slackware 13.0.0.0.0";
const QString ctn_SLACKWARE_12_2_VERSION_NAME  = "Slackware 12.2";
const QString ctn_SLACKWARE_12_1_VERSION_NAME  = "Slackware 12.1";
const QString ctn_SLACKWARE_12_0_VERSION_NAME  = "Slackware 12.0";
const QString ctn_SLACKWARE_11_0_VERSION_NAME  = "Slackware 11.0.0";

//Slackware supported architectures
const QString ctn_ARCH32  = "^i[4|5|6]86";
const QString ctn_ARCH64  = "^x86_64";
const QString ctn_ARCHARM = "arm";

const QString ctn_PATCHES_DIR_PREFIX = "patches_";
const QString ctn_UNKNOWN_SLACKWARE_VERSION = "UNKNOWN_VERSION";

//Slackware 64 bit versions supported
const QString ctn_SLACKWARE64_15_0  = "slackware64-15.0";
const QString ctn_SLACKWARE64_14_2  = "slackware64-14.2";
const QString ctn_SLACKWARE64_14_1  = "slackware64-14.1";
const QString ctn_SLACKWARE64_14_0  = "slackware64-14.0";
const QString ctn_SLACKWARE64_13_37 = "slackware64-13.37";
const QString ctn_SLACKWARE64_13_1  = "slackware64-13.1";
const QString ctn_SLACKWARE64_13_0  = "slackware64-13.0";

//Slackware 32 bit versions supported
const QString ctn_SLACKWARE_15_0  = "slackware-15.0";
const QString ctn_SLACKWARE_14_2  = "slackware-14.2";
const QString ctn_SLACKWARE_14_1  = "slackware-14.1";
const QString ctn_SLACKWARE_14_0  = "slackware-14.0";
const QString ctn_SLACKWARE_13_37 = "slackware-13.37";
const QString ctn_SLACKWARE_13_1  = "slackware-13.1";
const QString ctn_SLACKWARE_13_0  = "slackware-13.0";
const QString ctn_SLACKWARE_12_2  = "slackware-12.2";
const QString ctn_SLACKWARE_12_1  = "slackware-12.1";
const QString ctn_SLACKWARE_12_0  = "slackware-12.0";

//ArmedSlack versions supported
const QString ctn_ARMEDSLACK_15_0  = "slackwarearm-15.0";
const QString ctn_ARMEDSLACK_14_2  = "slackwarearm-14.2";
const QString ctn_ARMEDSLACK_14_1  = "slackwarearm-14.1";
const QString ctn_ARMEDSLACK_14_0  = "slackwarearm-14.0";
const QString ctn_ARMEDSLACK_13_37 = "armedslack-13.37";
const QString ctn_ARMEDSLACK_13_1  = "armedslack-13.1";
const QString ctn_ARMEDSLACK_12_2  = "armedslack-12.2";
const QString ctn_ARMEDSLACK_11_0  = "armedslack-11.0";

const QString ctn_PATCHES_URL = "/patches";
const QString ctn_PACKAGES_LIST = "PACKAGES.TXT";
const QString ctn_CHECKSUMS_FILE = "CHECKSUMS.md5";
const QString ctn_CHECKSUMS_SIGNATURES_FILE = "CHECKSUMS.md5.asc";

const QString ctn_SLACKWARE_MIRROR_USA = "http://mirrors.slackware.com/slackware/";
const QString ctn_SLACKWAREARM_MIRROR  = "http://slackware.uk/slackwarearm/";
//const QString ctn_SLACKWAREARM_MIRROR  = "ftp://ftp.slackware.org.uk/slackwarearm/";

const QString ctn_LIST_GPG_SLACKWARE_KEY     = "Key fingerprint = EC56 49DA 401E 22AB FA67  36EF 6A44 63C0 4010 2233";
const QString ctn_LIST_GPG_SLACKWARE_ARM_KEY = "Key fingerprint = 1552 7425 B232 9AC5 F11E  4824 29E6 F38E 4567 23FD";

const QString ctn_SLACKWARE_GPG_KEY = "GPG-KEY";
const QString ctn_GPG_SIGNATURE_EXTENSION = ".asc";

const QString ctn_PING_COMMAND_TEST = "ping -c 3 -i 1.5 google.com";
const QString ctn_PING_COMMAND_SUCCESS = "2 packets transmitted, 2 received, 0% packet loss";

const QString ctn_CURL_CONNECTION_ERROR = "Failed to connect to host";
const QString ctn_CURL_COULDNT_RESOLVE_HOST = "Couldn't resolve host";

const QString ctn_FTP_COMMAND_STRING = "curl -L --connect-timeout 20 --retry 2 --retry-delay 5 -# %1 -o %2";

enum Problem { ectn_NONE, ectn_NO_PING_BIN, ectn_NO_CURL_BIN, ectn_NO_GPG_BIN,
               ectn_NO_INTERNET, ectn_NO_NEW_PACKAGES, ectn_VERSION_NOT_SUPPORTED,
               ectn_PACKAGE_NOT_AUTHENTIC, ectn_SLACKWARE_MIRROR_DOWN, ectn_UNDEFINED,
               ectn_UNKNOWN_SLACKWARE_VERSION
             };

enum CurlExec { ectn_BLOCKING_EXEC, ectn_NON_BLOCKING_EXEC };

struct Patch{
private:
  QString m_subdirectory;
  QString m_fileName;
  QString m_description;

public:
  Patch(QString subdirectory, QString fileName, QString description){
    m_subdirectory = subdirectory;
    m_fileName = fileName;
    m_description = description;
  }

  bool operator==(const Patch t){
    return (t.getFileName() == m_fileName);
  }

  inline QString getSubdirectory() const { return m_subdirectory; }
  inline QString getFileName() const { return m_fileName; }
  inline QString getDescription() const { return m_description; }
};

class Updater : public QObject{
  Q_OBJECT

private:

  Problem m_lastProblem;
  QString m_lastErrorMessage;
  QString m_slackwareVersion;
  QString m_updaterDir;
  QString m_updaterMirror;
  QList<Patch> m_patchesList;
  int m_numberOfUpdates;

  static QString createUpdaterDirectory(QString pSlackwareVersion);

  bool hasInternetConnection();
  bool isAuthenticPackage( QString packageName );
  bool hasSlackGPGKeyInstalled();

  //These methods contact the Slackware mirror and MAY have trouble with internet connection.
  Problem getPackagesTxtFromMirror();
  Problem getPackageFromMirror( QString subdirectory, QString packageName );
  Problem getChecksumsSignatureFileFromMirror();
  Problem getChecksumsFromMirror();
  Problem getSlackGPGKeyFromMirror();
  Problem installSlackGPGKey();

  void parsePackagesTxt();
  void removePartFiles();
  QString readPackageListFile(QString filePath);
  QString getMD5HashFromChecksumsFile(QString targetFile);
  QString runCurlCommand(QString commandToRun, CurlExec typeOfCurlExecution = ectn_BLOCKING_EXEC);

public:

  static QString getSlackArchitecture();
  static QString getSlackVersion(QString architecture);
  static QString getUpdaterDirectory(QString *version=0);
  inline QString getLastErrorMessage(){ return m_lastErrorMessage; }

  Updater();
  ~Updater();

  //These methods contact the Slackware mirror and MAY have trouble with internet connection.
  int autoCheckUpdatesAvailable();

  Problem preInitialize();
  Problem initialize();
  Problem reinstallSlackGPGKey();

  QString getListOfAvailableUpdates();

  int getNumberOfUpdatesAvailable();
  int downloadAllUpdates();

  //Test functions ------------------------

  void testGetPackagesTxtFromMirror();
  QString testParsePackagesTxt();
  void testHasSlackGPGKeyInstalled();
  void testInstallSlackGPGKey();
  void testHasInternetConnection();
  void testGetPackageFromMirror();
  void testGetSixthPackageFromPackageList();  
  void testDownloadAllUpdates();

signals:

  void unknownSlackwareVersion();
  void noInternetAvailable();
  void noPingBinFound();
  void noCurlBinFound();
  void noGpgBinFound();

  void slackwareMirrorDown();
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
  void undefinedError();

  void curlDownloadProgressChanged(int);
};

#endif // UPDATER_H
