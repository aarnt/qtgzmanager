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

#ifndef MAINWINDOWIMPL_H
#define MAINWINDOWIMPL_H
//
#include "ui_mainwindow.h"
//

#include <iostream>
#include "qdnddirmodel.h"
#include "qdndstandarditemmodel.h"
#include "finddialogimpl.h"
#include "tvpackagesitemdelegate.h"
#include "updaterthread.h"
#include "simplestatusbar.h"
#include "uihelper.h"

#include <QStringList>
#include <QDir>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QKeyEvent>
#include <QProcess>
#include <QToolButton>
#include <QSystemTrayIcon>
#include <QFileSystemWatcher>
#include <QTimer>

enum SaveSettingsReason { ectn_FourPanelOrganizing, ectn_PackageList, ectn_InstalledPackageList,
                          ectn_ToolBar, ectn_DefaultDirectory, ectn_IconifyOnStart,
                          ectn_DontSave, ectn_Save };

const int ctn_PACKAGE_ICON ( 0 );
const int ctn_PACKAGE_NAME ( 1 );

class FrozenPkgListSingleton;
class SelectedPackage;
class ArgumentList;
class QLabel;
class UnixCommand;
class QTextBrowser;
class SearchBar;

template <typename T> class QFutureWatcher;

class MainWindowImpl : public QMainWindow, public Ui::MainWindow
{
  Q_OBJECT

  private:
    const QString ctn_LABEL_TREEVIEW_PACKAGES;

    UnixCommand *m_unixCommand;
    ArgumentList *m_argList;

    QTimer *timerGotoDirectory;
    bool m_interfaceInitialized;
    bool m_reallyWannaClose;
    bool m_ignoreCancelledActions;

    QString m_initialDir;
    QString m_defaultDirectory;
    QString m_updaterDirectory;
    PackagesClipBoard m_packagesClipboard;

    SearchBar *m_searchBar;

    AutoCheckUpdatesAvailableThread *m_autoCheckUpdatesThread;
    UpdaterThread *m_updaterThread;
    ReinstallSlackGPGKeyThread *m_reinstallSlackGPGKeyThread;
    FrozenPkgListSingleton *m_frozenPkgList;
    QFutureWatcher<QStringList> *m_fw;

    QFileSystemWatcher m_fsw;  //Watches for changes in the Packages Dir (2nd view) NEEDS inotify support!
    QFileSystemWatcher *m_psw; //Watches for changes in "/var/log/packages"

    QStringList m_discardedPackageList;	
		QByteArray m_horizontalSplit;
		QByteArray m_verticalSplit;		
    QByteArray m_savedGeometry;
    FindDialogImpl *m_findDialog;

    QLabel *m_lblStatus;
    QTabBar *m_tabBar;
    QIcon *m_appIcon;

    QAction *m_actionAbout;
    QAction *m_actionIconifyOnStart;
    SimpleStatusBar *m_statusbar;
    QSystemTrayIcon *m_systemTrayIcon;
    QMenu *m_systemTrayIconMenu;

    QLineEdit *m_titleDockDirectories;
    QList<QModelIndex> *m_foundFilesInPkgFileList;
    int m_indFoundFilesInPkgFileList;

		int m_PackageListOrderedCol, m_InstalledPackageListOrderedCol;
		Qt::SortOrder m_PackageListSortOrder, m_InstalledPackageListSortOrder;
		int m_clickedTab;

    //This member keeps the current number (out of total updates) of the package being downloaded
    int m_nowDownloading;

    QDnDDirModel *m_modelDir; //For the Directory treeView
    QDnDStandardItemModel *m_modelPackage; //For the Packages TreeView
    QDnDStandardItemModel *m_modelTodo; //For the Todo treeView

    QStandardItemModel *m_modelInstalledPackages; //For the InstalledPackages TreeView
    QSortFilterProxyModel *m_proxyModelPackage; //For the Packages Treeview's filter functionality
    QSortFilterProxyModel *m_proxyModelInstalledPackages; //For the InstalledPackages Treeview's filter functionality
    QStandardItem *m_install, *m_remove, *m_downgrade, *m_upgrade, *m_reinstall;

    QList<SelectedPackage> getSelectedInstalledPackage();

    QString _extractBaseFileName(const QString &fileName);

    void _expandItem(QTreeView*, QStandardItemModel*, QModelIndex*);
    void _collapseItem(QTreeView*, QStandardItemModel*, QModelIndex);
    void _collectActionExecGarbage();

    QString getTargetDirectory();
		bool hasTheExecutable( QString );

    void doInits();
    void initializeToolTip();
    void initializeExitAction();
    void initializeActions();
    void initializeMenuBar();
    void initializeToolBar();
    void initializeDirTreeView();
    void initializePackageTreeView();
    void initializeInstalledPackagesTreeView();
    void initializeTodoTreeView();   
    void initializeSystemTrayIcon();
    void initializeStatusBar();

    void _cutCopyPackages();
    void createGenericTab(const QString tabName, const QString tabContents);
    void createTabPkgFileList(const QString, const QStringList);
		void createTabPkgDiff(const QString pkg, const QString installedPkg, const QString diff );    
    void createTabUpdater();
    void createTabSnapshotAnalisys(QString snapshotFileName, QString content);

    QTextBrowser* getUpdaterOutput();
    void insertUpdaterText(const QString &);

		void removeAbsoluteDir( QStandardItemModel*, QModelIndex );
		void refreshPackageTreeView();
		void refreshInstalledPackageTreeView();

    bool hasPendingActions();

		QString filterPackageFoundStyleSheet();
		QString filterPackageNotFoundStyleSheet();

    void loadSettings();
    void saveSettings( int saveSettingsReason );
    void restoreViews();

    bool isFindDisabled();

    QString getHtmlListOfCancelledActions();
    void clearTodoTreeView();
    void startThreadReinstallSlackGPGKey();

    QString getToolTipNormalStyleSheet();
    QString getStrSilentOutput();

    QString _removeStringBugs(const QString str);        
    void _positionTextEditCursorAtEnd();
    void _tvTODORowsChanged(const QModelIndex& parent);
    void _tvTODOAdjustItemText(QStandardItem *item); 
    void _openSnapshotOfInstalledPackages(const QString);

  protected:

    void resizeEvent ( QResizeEvent * );
		void keyPressEvent( QKeyEvent* );
		void keyReleaseEvent( QKeyEvent* );		
		void closeEvent( QCloseEvent* );
		bool eventFilter( QObject *, QEvent * );

  protected slots:

    virtual void tvPackageSelectionChanged (const QItemSelection&, const QItemSelection&);
    virtual void tvInstalledPackagesSelectionChanged (const QItemSelection&, const QItemSelection&);
    virtual void tvTODOSelectionChanged (const QItemSelection&, const QItemSelection&);
    virtual void tvTODORowsInserted(const QModelIndex& parent, int, int);
    virtual void tvTODORowsRemoved(const QModelIndex& parent, int, int);

  public:

    static MainWindowImpl* returnMainWindow(){
      static MainWindowImpl *w=0;
      if (w != 0) return w;
      for (QWidget *widget: QApplication::topLevelWidgets()) {
        if (widget->objectName() == "MainWindow") w = (MainWindowImpl*) widget;
      }
      return w;
    }

    MainWindowImpl( QWidget * parent = 0); //, Qt::WFlags f = 0 );
    ~MainWindowImpl();

    inline bool isUpdaterRunning(){ return (m_updaterThread != 0 && m_updaterThread->isRunning()); }

    QDnDDirModel* getModelDir() { return m_modelDir; }
    QDnDStandardItemModel* getModelPackage() { return m_modelPackage; }
    QTreeView* getTODOTreeView() { return tvTODO; }

    void removeStyleOfToolTip();

    QString getSelectedDirectory(){ return (dockDirectories->windowTitle()); }
    QList<SelectedPackage> getSelectedPackage();	

    void setDisablePatchDownload(){ actionUpdater->setDisabled(true); }

  private slots:
  
    void onTimerGotoDirectoryTimeout();
    void verifyStateTODO( const QModelIndex , int, int );
    void exitApplication();

    void execContextMenuDirectories(QPoint point);
    void execContextMenuPackages(QPoint point);
    void execContextMenuInstalledPackages(QPoint point);  
    void execContextMenuTodoTreeView(QPoint point);
		void execContextMenuPkgFileList(QPoint);
		void execSystemTrayActivated ( QSystemTrayIcon::ActivationReason );

	 	void aboutQTGZManager();
    void deleteFile();
	 	void deleteActionFile();
	 	void deleteAllActionFiles();
    void deleteSelectedActionFiles();
    
    void createDirectory();
    void removeDirectory();
    void changeDefaultDirectory();

    void actionsProcessStarted();
    void actionsProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void actionsProcessRaisedError();
    void actionsProcessReadOutput();

    void tgz2lzmProcessStarted();
    void tgz2lzmProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void tgz2lzmProcessRaisedError();
    void tgz2lzmProcessReadOutput();
		
    void rpm2tgzProcessStarted();
    void rpm2tgzProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void rpm2tgzProcessRaisedError();
    void rpm2tgzProcessReadOutput();

    void reapplyPackageFilter();
		void reapplyInstalledPackagesFilter();
    void clearFilterPackage();
    void clearFilterInstalledPackage();

    void conditionalGotoNormalView();
    void conditionalGotoNormalViewSimple();

    void transformRPMinTGZ(const QString& = "");
		void transformTGZinLZM();

    void _deallocateTabContent(int tabIndex);
    void closeCurrentTab();
		void closeClickedTab();
		void closeAllTabs();
		void enableCloseTabButton( int );
    void tabCloseRequested( int );

    void diffToInstalled();
    void diffToEachOther();
    void showSetup();

    QString showFullPathOfObject(const QModelIndex & index);

    void insertInstallPackageAction();
    void insertDowngradePackageAction();
    void insertRemovePackageAction();
    void insertRemovePackageAction(QString packageName);
    void insertUpgradePackageAction();
    void insertReinstallPackageAction();

    void showPackageInfo();   
		void showInstalledPackageInfo();
    bool showPackageContent();
    void executePackageActions();
		void expandAllContentItems();
		void expandThisContentItems();
		void collapseAllContentItems();
		void collapseThisContentItems();

    void hideRightView(int SaveSettingsReason = ectn_Save);
    void minimizeLowerView(int SaveSettingsReason = ectn_Save);
    void maximizeLowerView(int SaveSettingsReason = ectn_Save);
    void normalView(int SaveSettingsReason = ectn_Save);
    void metaFindFile();
    void findFile();
    void findFileInPkgFileList();
    void findFileInPkgFileListExt();
    void findFileInPackage();
    void findPackage();
    void openFile();
    void editFile();
    void openDirectory();
    void openTerminal();
		void openFileOrDirectory(const QModelIndex&);

		void headerViewPackageList_click( int );
		void headerViewInstalledPackageList_click( int );

    void fileSystemWatcher_packagesDirectoryChanged ( const QString &);
    void fileSystemWatcher_installedPackagesDirectoryChanged ( const QString &);

		bool close();

    void showPackagesInDirectory(bool preserveSelected = true);
		void refreshTreeViews();
		void changeDir();
    void showToolBar();
    void refreshUIAfterSetup();
    void onActionIconifyTriggered();

    //Automatic check for available updates...
    void execAutoCheckUpdatesAvailable();
    bool execPackageContent();
    void openSnapshotOfInstalledPackages();
    void openThisSnapshotOfInstalledPackages();
    void takeSnapshotOfInstalledPackages(DumpInstalledPackageListOptions options = ectn_WITH_MODIFIED_DATE);
    void freezePackage();
    void unfreezePackage();

    void threadAutoCheckUpdatesAvailableReady(int);
    void startThreadUpdater();

    void threadUpdaterUnknownSlackwareVersion();
    void threadUpdaterVerifySlackwareSignature();
    void threadUpdaterSlackwareSignatureToDownload();
    void threadUpdaterSlackwareSignatureToInstall();
    void threadUpdaterPackageListToDownload();
    void threadUpdaterChecksumsToDownload();
    void threadUpdaterPackageListChecksum(bool value);
    void threadUpdaterNumberOfUpdates(int value);
    void threadUpdaterPackageToDownload(QString value);
    void threadUpdaterSignatureOfPackageToDownload(QString value);
    void threadUpdaterPackageSignatureChecking(bool value);
    void threadUpdaterStarted();
    void threadUpdaterFinished();
    void threadUpdaterNoInternetAvailable();
    void threadUpdaterNoPingBinFound();
    void threadUpdaterNoCurlBinFound();
    void threadUpdaterNoGpgBinFound();
    void threadUpdaterSlackwareMirrorDown();
    void threadUpdaterUndefinedError();
    void threadReinstallSlackGPGKeyStarted();
    void threadReinstallSlackGPGKeyFinished();
    void threadReinstallSlackGPGKeyUndefinedError();
    void threadUpdaterCurlDownloadStatus(int);
    void onTabMoved(int, int);

    void _positionInFirstMatch();
    void searchBarTextChanged(const QString textToSearch);
    void searchBarFindNext();
    void searchBarFindPrevious();
    void searchBarClosed();

    void searchBarTextChangedEx(const QString textToSearch);
    void searchBarFindNextEx();
    void searchBarFindPreviousEx();
    void searchBarClosedEx();

    void cutPackages();
    void copyPackages();
    void pastePackages();

    void positionInInstalledPkgList( const QString& );
    void positionInPkgList( const QString &dir, const QString &pkg );
    void positionInPkgFileList( const QString &fileName, const QString &directory );

  public slots:

    void show();
    void selectInstalledPackage();
    void toggleToolBarStatus();    
    void directoryRenamed(QString path, QString oldName, QString newName);
    void gotoDirectory(QString directoryToGo);
};

#endif
