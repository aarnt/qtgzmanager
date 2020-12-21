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

#ifndef FINDDIALOGIMPL_H
#define FINDDIALOGIMPL_H

#include "ui_finddialog.h"
#include "package.h"
#include <QDialog>
#include <QThread>
#include <QPointer>
#include <QIcon>
#include <QStandardItemModel>

const int MIN_LENGTH_SEARCH_STRING(2);

enum IterationMode { ectn_ITERATE_AFTERWARDS, ectn_ITERATE_BACKWARDS };

void _copyChildItem(QStandardItem *item, QStandardItem *clone);

class ThreadFind;
class CPUIntensiveComputing;
class QMutex;

class FindDialogImpl : public QDialog, public Ui::FindDialog
{
	Q_OBJECT
	
	private:
    QString m_packagePath; //This member holds the package name from which we're finding files
    bool m_isInstalledPackage; //This flag is true when the package being searched is installed

		CPUIntensiveComputing* m_ri;
		QPointer<ThreadFind> m_tf;
    QAction* actionOpenFile;
    QAction* actionEditFile;
    const QIcon m_iconFile;
    SearchPlace m_searchPlace;
    QString m_targetDir;
    QString m_targetPackage;
    QStandardItemModel *m_qstandardItemModel;
    QMap<QString, QStringList> m_mapPkgFileList;
    QMutex *m_mutex;
    bool m_stopGUIProcessing;

    void setEnableFindButton(bool enableFindButton);
    void iterateOverFoundItems(IterationMode im);
    void stopGUIProcessing();
    QString getPackageName(); //This method returns the package fileName, without path info

  private slots:
		void execFind();
		void stopFind();
		void resetDialog();

    void positionInInstalledPkgList(QTreeWidgetItem*);
    void positionInPkgList(QTreeWidgetItem*);
    void positionInPkgFileList(QTreeWidgetItem*);

    void finishedSearch();
    void execContextMenutwFindResults(QPoint);
    void canOpenFile(QTreeWidgetItem*);
    void openFile();
    void editFile();

	public:
    FindDialogImpl(QWidget *parent);
    ~FindDialogImpl();

    inline SearchPlace getSearchPlace(){ return m_searchPlace; }

    void setSearchPlace(SearchPlace sp);
    void setTargetDir(const QString &targetDir);
    void setTargetPackage(const QString &targetPackage);
    void setQStandardItemModel(const QStandardItemModel *sim, QStandardItem *sourceItem = 0);

    void setFontSize(const int);
    void setPackagePath(const QString&);

  signals:
    void installedPkgSelectedInFind(QString installedPkg);
    void packageInsideDirSelectedInFind(QString directory, QString pkg);
    void fileInsidePkgSelectedInFind(QString file, QString directory);

  public slots:
		void terminated(); 

	protected:
    bool eventFilter( QObject *, QEvent * );
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *);
		void keyReleaseEvent(QKeyEvent* ke);
		void keyPressEvent(QKeyEvent* ke);
};

class ThreadFind : public QThread
{
	private:
		QMap<QString, QStringList> m_map;
		QString m_stringToSearch;
    SearchPlace m_searchPlace;
    QString m_targetDir;
    QStandardItemModel *m_qstandardItemModel;

	public:
    ThreadFind();
    ~ThreadFind();

    void setStringToSearch(const QString &stringToSearch) { m_stringToSearch = stringToSearch; }
    void setSearchPlace( SearchPlace sp ) { m_searchPlace = sp; }
    void setTargetDir( const QString &targetDir ) { m_targetDir = targetDir; }
    void setQStandardItemModel( const QStandardItemModel *sim );
    void freeGarbage();

    void run();
		QMap<QString, QStringList>& getResultMap();
};

#endif
