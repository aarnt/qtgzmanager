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

#include "updater.h"
#include "updaterthread.h"
#include "settingsmanager.h"
#include "unixcommand.h"
#include "package.h"
#include <iostream>

#include <QCryptographicHash>

Updater::Updater(){
  m_updaterDir = getUpdaterDirectory(&m_slackwareVersion) + QDir::separator();
  m_updaterMirror = SettingsManager::getUpdaterMirror();

  if (m_updaterMirror.isEmpty()){
    if (UnixCommand::getSlackArchitecture().contains("arm", Qt::CaseInsensitive))
      m_updaterMirror=ctn_SLACKWAREARM_MIRROR;
    else
      m_updaterMirror=ctn_SLACKWARE_MIRROR_USA;
  }
}

Updater::~Updater(){
  removePartFiles();
}

int Updater::autoCheckUpdatesAvailable(){
  m_numberOfUpdates = -1;
  m_lastProblem = ectn_NONE;

  /*
   * Get the latest PACKAGES.TXT from mirror.
   * This function returns something different from "Problem ectn_NONE" if there was an error.
  */
  Problem pResult = getPackagesTxtFromMirror();
  if (pResult != ectn_NONE) return 0;

  //Build the QList with the information of each package that is able to be upgraded
  parsePackagesTxt();

  //Return the number of available patches!
  return (getNumberOfUpdatesAvailable());
}

//Remove any ".part" file at updaterDir
void Updater::removePartFiles(){
  QDir dir;
  dir.setPath(m_updaterDir);
  dir.setFilter(QDir::Files);
  QStringList sl;

  QFileInfoList list = dir.entryInfoList(sl << "*.part");

  for (int i = 0; i < list.size(); ++i) {
    QFileInfo fileInfo = list.at(i);
    dir.remove(fileInfo.filePath());
  }
}

Problem Updater::preInitialize(){
  Problem pResult = ectn_NONE;

  if (m_slackwareVersion == ctn_UNKNOWN_SLACKWARE_VERSION){
    pResult = ectn_UNKNOWN_SLACKWARE_VERSION;

    emit unknownSlackwareVersion();
    return pResult;
  }

  if (!UnixCommand::hasTheExecutable("ping")){
    pResult = ectn_NO_PING_BIN;
    emit noPingBinFound();
    return pResult;
  }

  if (!UnixCommand::hasTheExecutable("curl")){
    pResult = ectn_NO_CURL_BIN;
    emit noCurlBinFound();
    return pResult;
  }

  if (!UnixCommand::hasTheExecutable("gpg")){
    pResult = ectn_NO_GPG_BIN;
    emit noGpgBinFound();
    return pResult;
  }

  if (m_slackwareVersion == ""){
    pResult = ectn_VERSION_NOT_SUPPORTED;
    return pResult;
  }

  if (!hasInternetConnection()){
    pResult = ectn_NO_INTERNET;
    m_lastProblem = pResult;

    emit noInternetAvailable();
    return pResult;
  }

  m_lastProblem = pResult;
  return pResult;
}

Problem Updater::initialize(){
  m_numberOfUpdates = -1;
  Problem pResult = preInitialize();

  if (pResult == ectn_NONE){
    //The system has the Slackware GPG-KEY installed?
    if (!hasSlackGPGKeyInstalled()){
      pResult = installSlackGPGKey();
      if (pResult == ectn_SLACKWARE_MIRROR_DOWN){
        emit slackwareMirrorDown();

        return pResult;
      }
    }

    //Get the latest PACKAGES.TXT from mirror.
    //This function returns something different from "Problem ectn_NONE" if there was an error.
    pResult = getPackagesTxtFromMirror();
    if (pResult == ectn_SLACKWARE_MIRROR_DOWN){
      m_lastProblem=pResult;
      emit slackwareMirrorDown();

      return pResult;
    }
    else if (pResult == ectn_PACKAGE_NOT_AUTHENTIC){

      return pResult;
    }
    else if (pResult == ectn_UNDEFINED){
      m_lastProblem=pResult;
      emit undefinedError();

      return pResult;
    }

    //Build the QList with the information of each package that is able to be upgraded
    parsePackagesTxt();

    //Is there any package that can be upgraded?
    if (getNumberOfUpdatesAvailable() == 0){
      pResult = ectn_NO_NEW_PACKAGES;
    }
  }

  m_lastProblem = pResult;    
  return pResult;
}

Problem Updater::reinstallSlackGPGKey(){
  Problem res = ectn_NONE;

  res = getSlackGPGKeyFromMirror();

  if (res != ectn_NONE){
    if (res == ectn_SLACKWARE_MIRROR_DOWN)
      emit slackwareMirrorDown();
    else if (res == ectn_UNDEFINED)
      emit undefinedError();

    return res;
  }

  installSlackGPGKey();
  return res;
}

QString Updater::getListOfAvailableUpdates(){
  QString result("");

  if (m_lastProblem == ectn_NONE){    
    result += "<br><pre><ol>";

    for(Patch p: m_patchesList){
      result += "<li><h4>" + p.getFileName() + "</h4></li>";
    }

    result += "</ol></pre><br>";
  }

  return result;
}

QString Updater::getSlackArchitecture(){
  return UnixCommand::getSlackArchitecture();
}

QString Updater::getSlackVersion(QString architecture){
  QString result;

  if (architecture == "")
    result = ctn_UNKNOWN_SLACKWARE_VERSION;

  QFile slackVersionFile(ctn_SLACKWARE_VERSION_FILE);

  if (slackVersionFile.exists()){
    slackVersionFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QString data = slackVersionFile.readAll();
    slackVersionFile.close();

    if (data.indexOf(ctn_SLACKWARE_15_0_VERSION_NAME) != -1){
      if (architecture.indexOf(QRegExp(ctn_ARCH32)) != -1)
        result = ctn_SLACKWARE_15_0;
      else if (architecture.indexOf(QRegExp(ctn_ARCH64)) != -1)
        result = ctn_SLACKWARE64_15_0;
      else if (architecture.contains(ctn_ARCHARM, Qt::CaseInsensitive))
        result = ctn_ARMEDSLACK_15_0;
      else result = ctn_UNKNOWN_SLACKWARE_VERSION;
    }

    else if (data.indexOf(ctn_SLACKWARE_14_2_VERSION_NAME) != -1){
      if (architecture.indexOf(QRegExp(ctn_ARCH32)) != -1)
        result = ctn_SLACKWARE_14_2;
      else if (architecture.indexOf(QRegExp(ctn_ARCH64)) != -1)
        result = ctn_SLACKWARE64_14_2;
      else if (architecture.contains(ctn_ARCHARM, Qt::CaseInsensitive))
        result = ctn_ARMEDSLACK_14_2;
      else result = ctn_UNKNOWN_SLACKWARE_VERSION;
    }
   
    else if (data.indexOf(ctn_SLACKWARE_14_1_VERSION_NAME) != -1){
      if (architecture.indexOf(QRegExp(ctn_ARCH32)) != -1)
        result = ctn_SLACKWARE_14_1;
      else if (architecture.indexOf(QRegExp(ctn_ARCH64)) != -1)
        result = ctn_SLACKWARE64_14_1;
      else if (architecture.contains(ctn_ARCHARM, Qt::CaseInsensitive))
        result = ctn_ARMEDSLACK_14_1;
      else result = ctn_UNKNOWN_SLACKWARE_VERSION;
    }

    else if (data.indexOf(ctn_SLACKWARE_14_0_VERSION_NAME) != -1){
      if (architecture.indexOf(QRegExp(ctn_ARCH32)) != -1)
        result = ctn_SLACKWARE_14_0;
      else if (architecture.indexOf(QRegExp(ctn_ARCH64)) != -1)
        result = ctn_SLACKWARE64_14_0;
      else if (architecture.contains(ctn_ARCHARM, Qt::CaseInsensitive))
        result = ctn_ARMEDSLACK_14_0;
      else result = ctn_UNKNOWN_SLACKWARE_VERSION;
    }

    else if (data.indexOf(ctn_SLACKWARE_13_37_VERSION_NAME) != -1){
      if (architecture.indexOf(QRegExp(ctn_ARCH32)) != -1)
        result = ctn_SLACKWARE_13_37;
      else if (architecture.indexOf(QRegExp(ctn_ARCH64)) != -1)
        result = ctn_SLACKWARE64_13_37;
      else if (architecture.contains(ctn_ARCHARM, Qt::CaseInsensitive))
        result = ctn_ARMEDSLACK_13_37;
      else result = ctn_UNKNOWN_SLACKWARE_VERSION;
    }

    else if (data.indexOf(ctn_SLACKWARE_13_1_VERSION_NAME) != -1){
      if (architecture.indexOf(QRegExp(ctn_ARCH32)) != -1)
        result = ctn_SLACKWARE_13_1;
      else if (architecture.indexOf(QRegExp(ctn_ARCH64)) != -1)
        result = ctn_SLACKWARE64_13_1;
      else if (architecture.contains(ctn_ARCHARM, Qt::CaseInsensitive))
        result = ctn_ARMEDSLACK_13_1;
      else result = ctn_UNKNOWN_SLACKWARE_VERSION;
    }

    else if (data.indexOf(ctn_SLACKWARE_13_0_VERSION_NAME) != -1){
      if (architecture.indexOf(QRegExp(ctn_ARCH32)) != -1)
        result = ctn_SLACKWARE_13_0;
      else if (architecture.indexOf(QRegExp(ctn_ARCH64)) != -1)
        result = ctn_SLACKWARE64_13_0;
      else result = ctn_UNKNOWN_SLACKWARE_VERSION;
    }

    else if (data.indexOf(ctn_SLACKWARE_12_2_VERSION_NAME) != -1){
      if (architecture.indexOf(QRegExp(ctn_ARCH32)) != -1)
        result = ctn_SLACKWARE_12_2;
      else if (architecture.contains(ctn_ARCHARM, Qt::CaseInsensitive))
        result = ctn_ARMEDSLACK_12_2;
    }
    else if (data.indexOf(ctn_SLACKWARE_12_1_VERSION_NAME) != -1) result = ctn_SLACKWARE_12_1;

    else if (data.indexOf(ctn_SLACKWARE_12_0_VERSION_NAME) != -1) result = ctn_SLACKWARE_12_0;

    else if (data.indexOf(ctn_SLACKWARE_11_0_VERSION_NAME) != -1){
      if (architecture.contains(ctn_ARCHARM, Qt::CaseInsensitive))
        result = ctn_ARMEDSLACK_11_0;
    }

    else result = ctn_UNKNOWN_SLACKWARE_VERSION;
  }

  else result = ctn_UNKNOWN_SLACKWARE_VERSION;

  return result;
}

QString Updater::getUpdaterDirectory(QString *version){
  QString defaultDir = SettingsManager::instance()->getDefaultDirectory();
  QString updaterDir = SettingsManager::instance()->getUpdaterDirectory();
  QDir dirUpdaterDir(updaterDir);

  QString slackVersion = getSlackVersion(getSlackArchitecture());

  if (version != 0){
    *version = slackVersion;
    if (*version == ctn_UNKNOWN_SLACKWARE_VERSION)
      return "";
  }
  else if (slackVersion == ctn_UNKNOWN_SLACKWARE_VERSION)
    return "";

  QString newUpdaterDir = defaultDir + QDir::separator() + ctn_PATCHES_DIR_PREFIX + slackVersion;

  if (slackVersion.isEmpty()) return "";
  else if (updaterDir == defaultDir || !dirUpdaterDir.exists() || updaterDir != newUpdaterDir){
    return createUpdaterDirectory(slackVersion);
  }
  else{    
    return updaterDir;
  }  
}

QString Updater::createUpdaterDirectory(QString pSlackwareVersion){
  QString updaterDir("");
  QDir baseDir("/tmp/");
  updaterDir = ctn_PATCHES_DIR_PREFIX + pSlackwareVersion;

  if (!baseDir.exists(updaterDir))
    baseDir.mkdir(updaterDir);

  SettingsManager::setUpdaterDirectory(baseDir.path() + QDir::separator() + updaterDir);
  return baseDir.path() + QDir::separator() + updaterDir;
}

bool Updater::isAuthenticPackage( QString packageName ){
  return UnixCommand::isAuthenticPackage(packageName);
}

QString Updater::runCurlCommand(QString commandToRun, CurlExec typeOfCurlExecution){
  QString res;

  if (typeOfCurlExecution == ectn_BLOCKING_EXEC){
    res=UnixCommand::runCurlCommand(commandToRun);
  }
  else if (typeOfCurlExecution == ectn_NON_BLOCKING_EXEC){
    CurlThread *w = new CurlThread;

    connect(w, SIGNAL(curlDownloadProgressChanged(int)),
            this, SIGNAL(curlDownloadProgressChanged(int)),
            Qt::DirectConnection);

    w->setCommandToRun(commandToRun);
    w->start();
    w->wait();

    res=w->getResult();
  }
  return res;
}

Problem Updater::getPackageFromMirror( QString subdirectory, QString packageName ){
  Problem result = ectn_NONE;

  QFile oldFile(m_updaterDir + packageName);
  if (oldFile.exists()){
    return result;
  }

  emit packageToDownloadChanged(packageName);

  subdirectory = QDir::separator() + subdirectory.remove(0, 2);    

  QString ftpCommand = ctn_FTP_COMMAND_STRING.arg(
        m_updaterMirror + m_slackwareVersion + subdirectory + "/" + packageName).arg(
        m_updaterDir + packageName + ".part");


  QString res = runCurlCommand(ftpCommand, ectn_NON_BLOCKING_EXEC);

  if (res.contains(ctn_CURL_CONNECTION_ERROR))
    return (ectn_SLACKWARE_MIRROR_DOWN);
  else if (res.contains(ctn_CURL_COULDNT_RESOLVE_HOST))
    return (ectn_NO_INTERNET);
  else if (!res.isEmpty()){
    m_lastErrorMessage = res;
    return (ectn_UNDEFINED);
  }

  emit signatureOfPackageToDownloadChanged(packageName + ctn_GPG_SIGNATURE_EXTENSION);

  ftpCommand = ctn_FTP_COMMAND_STRING.arg(m_updaterMirror + m_slackwareVersion +
                                                   subdirectory + "/" + packageName + ctn_GPG_SIGNATURE_EXTENSION).arg(
        m_updaterDir + packageName + ctn_GPG_SIGNATURE_EXTENSION);

  res = UnixCommand::runCurlCommand(ftpCommand);

  if (res.contains(ctn_CURL_CONNECTION_ERROR))
    return (ectn_SLACKWARE_MIRROR_DOWN);
  else if (!res.isEmpty()){
    QFile fileToRemove(m_updaterDir + packageName + ".part");
    if (fileToRemove.exists()) fileToRemove.remove();
    m_lastErrorMessage = res;
    return (ectn_UNDEFINED);
  }

  QFile newPackage(m_updaterDir + packageName + ".part");
  newPackage.rename(m_updaterDir + packageName);
  bool isAuthentic = isAuthenticPackage(m_updaterDir + packageName);

  emit packageSignatureCheckingChanged(isAuthentic);

  if (!isAuthentic){
    QFile fileToRemove(m_updaterDir + packageName);
    if (fileToRemove.exists()) fileToRemove.remove();

    QFile signatureToRemove(m_updaterDir + packageName + ctn_GPG_SIGNATURE_EXTENSION);
    if (signatureToRemove.exists()) signatureToRemove.remove();

    result = ectn_PACKAGE_NOT_AUTHENTIC;
  }

  return result;  
}

QString Updater::readPackageListFile(QString filePath){
  QFile packageListFile(filePath);
  packageListFile.open(QIODevice::ReadOnly | QIODevice::Text);
  QString result = packageListFile.readAll();
  packageListFile.close();
  return result;
}

Problem Updater::getChecksumsSignatureFileFromMirror(){
  Problem result = ectn_NONE;

  QString signaturesFileName(m_updaterDir + QDir::separator() + ctn_CHECKSUMS_SIGNATURES_FILE);
  QFile oldChecksumsSignatureFile(signaturesFileName);
  if (oldChecksumsSignatureFile.exists()) oldChecksumsSignatureFile.remove();

  QString ftpCommand = ctn_FTP_COMMAND_STRING.arg(m_updaterMirror + m_slackwareVersion +
                                                   ctn_PATCHES_URL + "/" + ctn_CHECKSUMS_SIGNATURES_FILE).arg(
        signaturesFileName);

  QString res = runCurlCommand(ftpCommand);

  if (res.contains(ctn_CURL_CONNECTION_ERROR))
    return ectn_SLACKWARE_MIRROR_DOWN;
  else if (!res.isEmpty()){
    m_lastErrorMessage = res;
    return ectn_UNDEFINED;
  }

  if (oldChecksumsSignatureFile.exists())
    return result;
  else return ectn_UNDEFINED;
}

Problem Updater::getChecksumsFromMirror(){
  Problem result = ectn_NONE;

  QString oldChecksumsFileName(m_updaterDir + ctn_CHECKSUMS_FILE);
  QFileInfo oldChecksumsFile(oldChecksumsFileName);

  if (oldChecksumsFile.exists()){
    QString newChecksumsFileName = "/tmp/updater_CHECKSUMS_" +
                          QDateTime::currentDateTime().toString("dd_MM_yyyy_hh_mm_ss");

    emit checksumsToDownloadChanged();

    QString ftpCommand = ctn_FTP_COMMAND_STRING.arg(m_updaterMirror + m_slackwareVersion +
                                                     ctn_PATCHES_URL + "/" + ctn_CHECKSUMS_FILE).arg(
          newChecksumsFileName);

    QString res = runCurlCommand(ftpCommand);

    if (res.contains(ctn_CURL_CONNECTION_ERROR))
      return ectn_SLACKWARE_MIRROR_DOWN;
    else if (!res.isEmpty()){
      m_lastErrorMessage = res;
      return ectn_UNDEFINED;
    }

    QString newHash =
        QCryptographicHash::hash(readPackageListFile(newChecksumsFileName).toLatin1(),
                                 QCryptographicHash::Sha1);
    QString oldHash =
        QCryptographicHash::hash(readPackageListFile(oldChecksumsFileName).toLatin1(),
                                 QCryptographicHash::Sha1);

    if (newHash != oldHash){
      QDir dir(m_updaterDir);
      dir.remove(ctn_CHECKSUMS_FILE);
      QFile newFile(newChecksumsFileName);
      newFile.copy(m_updaterDir + ctn_CHECKSUMS_FILE);
      newFile.remove();

      Problem ret = getChecksumsSignatureFileFromMirror();

      if (ret == ectn_NONE){
        if (isAuthenticPackage(m_updaterDir + ctn_CHECKSUMS_FILE)){
          result = ectn_NONE;
        }
        else result = ectn_PACKAGE_NOT_AUTHENTIC;
      }
      else if (ret == ectn_UNDEFINED){
        result = ectn_UNDEFINED;
      }
    }
    else{
      QFile newFile(newChecksumsFileName);
      newFile.remove();

      Problem ret = getChecksumsSignatureFileFromMirror();

      if (ret == ectn_NONE){
        if (isAuthenticPackage(m_updaterDir + ctn_CHECKSUMS_FILE)){
          result = ectn_NONE;
        }
        else result = ectn_PACKAGE_NOT_AUTHENTIC;
      }
      else if (ret == ectn_UNDEFINED){
        result = ectn_UNDEFINED;
      }
    }
  }
  else{
    emit checksumsToDownloadChanged();

    QString ftpCommand = ctn_FTP_COMMAND_STRING.arg(m_updaterMirror + m_slackwareVersion +
                                                     ctn_PATCHES_URL + "/" + ctn_CHECKSUMS_FILE).arg(
          m_updaterDir + ctn_CHECKSUMS_FILE);

    QString res = runCurlCommand(ftpCommand);
    if (res.contains(ctn_CURL_CONNECTION_ERROR))
      return ectn_SLACKWARE_MIRROR_DOWN;
    else if (!res.isEmpty()){
      m_lastErrorMessage = res;
      return ectn_UNDEFINED;
    }

    Problem ret = getChecksumsSignatureFileFromMirror();

    if (ret == ectn_NONE){
      if (isAuthenticPackage(m_updaterDir + ctn_CHECKSUMS_FILE)){
        result = ectn_NONE;
      }
      else result = ectn_PACKAGE_NOT_AUTHENTIC;
    }
    else if (ret == ectn_UNDEFINED){
      result = ectn_UNDEFINED;
    }
  }

  return result;
}

Problem Updater::getPackagesTxtFromMirror(){
  bool bHasChanges = false;
  Problem result = ectn_NONE;

  QString ftpCommand("");
  QString oldPackagesTxtFileName(m_updaterDir + ctn_PACKAGES_LIST);
  QFileInfo oldPackagesTxtFile(oldPackagesTxtFileName);

  //Garbage collect from /tmp/
  QDir tmp("/tmp/");
  QStringList nameFilters;
  nameFilters << "updater_PACKAGELIST_*";
  nameFilters << "updater_CHECKSUMS_*";
  QStringList filesToRemove = tmp.entryList(nameFilters, QDir::Files);

  for(QString fileToRemove: filesToRemove){
    QFile f("/tmp/" + fileToRemove);
    f.remove();
  }

  if (oldPackagesTxtFile.exists()){
    QString newPackagesTxtFileName = "/tmp/updater_PACKAGELIST_" +
                          QDateTime::currentDateTime().toString("dd_MM_yyyy_hh_mm_ss");

    emit packageListToDownloadChanged();

    ftpCommand = ctn_FTP_COMMAND_STRING.arg(m_updaterMirror + m_slackwareVersion +
                                             ctn_PATCHES_URL + "/" + ctn_PACKAGES_LIST).arg(newPackagesTxtFileName);

    QString output = runCurlCommand(ftpCommand);

    if (output.contains(ctn_CURL_CONNECTION_ERROR))
      return (ectn_SLACKWARE_MIRROR_DOWN);
    else if (!output.isEmpty()){
      m_lastErrorMessage = output;
      return ectn_UNDEFINED;
    }

    QString newHash =
        QCryptographicHash::hash(readPackageListFile(newPackagesTxtFileName).toLatin1(),
                                 QCryptographicHash::Sha1);
    QString oldHash =
        QCryptographicHash::hash(readPackageListFile(oldPackagesTxtFileName).toLatin1(),
                                 QCryptographicHash::Sha1);

    if (newHash != oldHash){
      QDir dir(m_updaterDir);
      dir.remove(ctn_PACKAGES_LIST);
      QFile newFile(newPackagesTxtFileName);
      newFile.copy(m_updaterDir + ctn_PACKAGES_LIST);
      newFile.remove();
      bHasChanges = true;
    }
    else{
      QFile newFile(newPackagesTxtFileName);
      newFile.remove();
    }
  }
  else {
    emit packageListToDownloadChanged();

    QString ftpCommand = ctn_FTP_COMMAND_STRING.arg(m_updaterMirror + m_slackwareVersion +
                                                     ctn_PATCHES_URL + "/" + ctn_PACKAGES_LIST).arg(
          oldPackagesTxtFileName);  

    QString output = runCurlCommand(ftpCommand);

    if (output.contains(ctn_CURL_CONNECTION_ERROR))
      return (ectn_SLACKWARE_MIRROR_DOWN);
    else if (!output.isEmpty()){
      m_lastErrorMessage = output;
      return ectn_UNDEFINED;
    }

    bHasChanges = true;
  }

  if (bHasChanges){
    Problem ret = getChecksumsFromMirror();

    if (ret != ectn_NONE) return ret;
    else{
      /*
        Well, here we have PACKAGES.TXT and the authentic CHECKSUMS.MD5 file.
        We ONLY need to check if PACKAGES.TXT md5 signature is correct.
      */
      QString hashInChecksumsFile = getMD5HashFromChecksumsFile(ctn_PACKAGES_LIST);

      QFile target(oldPackagesTxtFileName);
      target.open(QIODevice::ReadOnly | QIODevice::Text);
      QString content = target.readAll();
      target.close();

      QString fileHash = QCryptographicHash::hash(content.toUtf8(), QCryptographicHash::Md5).toHex();
      bool theyAreDifferent = fileHash != hashInChecksumsFile;
      emit packageListChecksumChanged(!theyAreDifferent);

      if (theyAreDifferent){
        result = ectn_PACKAGE_NOT_AUTHENTIC; //Ooopps... we'd better not use those files.
      }
    }
  }

  return result;
}

QString Updater::getMD5HashFromChecksumsFile(QString targetFile){
  QString hash("");
  QFile checksumsFile(m_updaterDir + QDir::separator() + ctn_CHECKSUMS_FILE);
  char buf[512];

  if (checksumsFile.exists()) {
    checksumsFile.open(QIODevice::ReadOnly | QIODevice::Text);

    do{
      qint64 lineLength = checksumsFile.readLine(buf, sizeof(buf));
      if (lineLength == -1) break;

      QString lineRead(buf);
      if (lineRead.indexOf(targetFile) != -1){
        hash = lineRead.left(ctn_SIZE_OF_CHECKSUM_HASH);
        break;
      }
    }
    while(true);

    checksumsFile.close();
  }

  return hash;
}

void Updater::parsePackagesTxt(){
  m_patchesList.clear();
  QFile packageListFile(m_updaterDir + ctn_PACKAGES_LIST);
  packageListFile.open(QIODevice::ReadOnly | QIODevice::Text);
  QString pkgName("");
  QString pkgLocation("");
  QString pkgDescription("");
  char buf[512];

  do{
    qint64 lineLength = packageListFile.readLine(buf, sizeof(buf));
    if (lineLength == -1) break;

    QString lineRead(buf);
    if (lineRead.indexOf("PACKAGE NAME:") != -1){
      pkgName = lineRead.right(lineRead.size() - 13).trimmed();
    }
    else if (lineRead.indexOf("PACKAGE LOCATION:") != -1){
      pkgLocation = lineRead.right(lineRead.size() - 17).trimmed();
    }
    else if (pkgName != "" && (lineRead.indexOf(Package::getBaseName(pkgName)) != -1)) {
      int size = Package::getBaseName(pkgName).size() + 1;
      pkgDescription += lineRead.right(lineRead.size() - size);
    }
    else if (lineLength == 1 && pkgLocation != "" && pkgName != "" && pkgDescription != ""){
      pkgDescription = Package::makeURLClickable(pkgDescription);
      Patch p(pkgLocation, pkgName, pkgDescription);
      m_patchesList.append(p);

      pkgLocation = "";
      pkgName = "";
      pkgDescription = "";
    }
  }
  while(true);

  packageListFile.close();

  //Make the list show only the packages available for update (that weren't downloaded).
  QMutableListIterator<Patch> p(m_patchesList);
  while(p.hasNext()){
    Patch item = p.next();
    Result status = Package::getStatus(item.getFileName());

    QFile patchFile(m_updaterDir + QDir::separator() + item.getFileName());
    if (status.getClassification() == ectn_NOT_INSTALLED ||
        status.getClassification() == ectn_INSTALLED ||
        status.getClassification() == ectn_INFERIOR_VERSION ||
        patchFile.exists())
      p.remove();
  }

  emit numberOfUpdatesChanged(m_patchesList.count());
}

Problem Updater::getSlackGPGKeyFromMirror(){
  Problem result = ectn_NONE;

  emit slackwareSignatureToDownloadChanged();

  QString ftpCommand = ctn_FTP_COMMAND_STRING.arg(
        m_updaterMirror + m_slackwareVersion + QDir::separator() + ctn_SLACKWARE_GPG_KEY).arg(
        m_updaterDir + ctn_SLACKWARE_GPG_KEY);

  QString res = runCurlCommand(ftpCommand);

  if (res.contains(ctn_CURL_CONNECTION_ERROR))
    return ectn_SLACKWARE_MIRROR_DOWN;
  else if (!res.isEmpty()){
    m_lastErrorMessage=res;
    return ectn_UNDEFINED;
  }

  return result;
}

Problem Updater::installSlackGPGKey(){
  Problem res = ectn_NONE;

  QString gpgKeyPath = m_updaterDir + ctn_SLACKWARE_GPG_KEY;
  QFile keyFile(gpgKeyPath);
  if (!keyFile.exists()){
    res = getSlackGPGKeyFromMirror();
    if (res != ectn_NONE)
      return res;
  }

  emit slackwareSignatureToInstallChanged();
  UnixCommand::installSlackGPGKey(gpgKeyPath);

  return res;
}

bool Updater::hasInternetConnection(){
  return UnixCommand::hasInternetConnection();
}

bool Updater::hasSlackGPGKeyInstalled(){
  emit verifySlackwareSignatureChanged();
  return UnixCommand::hasSlackGPGKeyInstalled(m_slackwareVersion);
}

int Updater::getNumberOfUpdatesAvailable(){
  if (m_numberOfUpdates == -1){
    if (m_lastProblem == ectn_NONE) m_numberOfUpdates = m_patchesList.count();
  }

  return m_numberOfUpdates;
}

int Updater::downloadAllUpdates(){
  int updatesDownloaded = 0;

  for( Patch p: m_patchesList ){
    Problem ret = getPackageFromMirror(p.getSubdirectory(), p.getFileName());

    if (ret == ectn_NONE){
      //If there were no problems with this package...
      updatesDownloaded++;
    }
    else if (ret == ectn_SLACKWARE_MIRROR_DOWN || ret == ectn_NO_INTERNET){
      emit noInternetAvailable();
    }
    else if (ret == ectn_UNDEFINED){
      emit undefinedError();
    }
  }

  return updatesDownloaded;
}

//Test functions -------------------------------------------------------

QString Updater::testParsePackagesTxt(){
  parsePackagesTxt();
  QString output("<pre>");

  for(Patch p: m_patchesList){
    output += p.getDescription() + "<br/><br/>";
  }

  output += "</pre>";
  return output;
}

void Updater::testGetPackagesTxtFromMirror(){
  getPackagesTxtFromMirror();
}

void Updater::testHasSlackGPGKeyInstalled(){
  if (hasSlackGPGKeyInstalled()){
    std::cout << "Slackware's GPG-KEY is installed!" << std::endl;
  }
  else{
    std::cout << "Slackware's GPG-KEY is NOT installed." << std::endl;
  }
}

void Updater::testInstallSlackGPGKey(){
  installSlackGPGKey();
}

void Updater::testHasInternetConnection(){
  if(hasInternetConnection()){
    std::cout << "Internet Connection is ON!" << std::endl;
  }
  else{
    std::cout << "Internet Connection is off!" << std::endl;
  }
}

void Updater::testGetPackageFromMirror(){
  if (getPackageFromMirror("./patches/packages", "irssi-0.8.15-i486-1_slack13.0.txz")){
    std::cout << "The package was retreived successfully!" << std::endl;
  }
  else{
    std::cout << "It was not possible to get the package." << std::endl;
  }
}

void Updater::testGetSixthPackageFromPackageList(){
  parsePackagesTxt();
  if (m_patchesList.size() > 6){
    std::cout << "Package Name: " << m_patchesList.at(5).getFileName().toLatin1().data() << std::endl;
    getPackageFromMirror(m_patchesList.at(5).getSubdirectory(), m_patchesList.at(5).getFileName());
  }
}

void Updater::testDownloadAllUpdates(){
  downloadAllUpdates();
}
