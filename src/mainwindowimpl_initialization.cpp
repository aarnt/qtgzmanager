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
#include "searchbar.h"
#include "package.h"
#include "tvpackagesitemdelegate.h"
#include "argumentlist.h"
#include "setupdialog.h"
#include "strconstants.h"
#include "uihelper.h"

//Loads various application settings configured in ~/.config/QTGZManager.conf
void MainWindowImpl::loadSettings(){
  //Change UpdaterDir to new location...
  SettingsManager::moveUpdaterDirContents();

  //Checks if the userMirrors file has been created.
  QFile userMirrorsFile(StrConstants::getUserMirrorsFile());

  //If it has not, create one based on internal template
  if (!userMirrorsFile.exists()){
    QFile ftemplate(":/resources/updater/user_mirrors_template");
    ftemplate.copy(StrConstants::getUserMirrorsFile());
    userMirrorsFile.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ReadGroup|QFile::ReadOther);
  }

  m_verticalSplit = splitterVertical->saveState();
  m_horizontalSplit = splitterHorizontal->saveState();
  doInits();

  toolBar->setHidden(!SettingsManager::instance()->getShowToolBar());
  m_PackageListOrderedCol = SettingsManager::instance()->getPackageListOrderedCol();
  m_InstalledPackageListOrderedCol = SettingsManager::instance()->getInstalledPackageListOrderedCol();
  m_PackageListSortOrder = (Qt::SortOrder) SettingsManager::instance()->getPackageListSortOrder();
  m_InstalledPackageListSortOrder =
      (Qt::SortOrder) SettingsManager::instance()->getInstalledPackageListSortOrder();
  m_defaultDirectory = SettingsManager::instance()->getDefaultDirectory();
  m_updaterDirectory = Updater::getUpdaterDirectory();
  m_actionIconifyOnStart->setChecked(SettingsManager::instance()->getStartIconified());

  tvPackage->header()->setSortIndicator( m_PackageListOrderedCol, m_PackageListSortOrder );
  tvPackage->sortByColumn( m_PackageListOrderedCol, m_PackageListSortOrder );
  tvInstalledPackages->header()->setSortIndicator(
        m_InstalledPackageListOrderedCol, m_InstalledPackageListSortOrder );
  tvInstalledPackages->sortByColumn(
        m_InstalledPackageListOrderedCol, m_InstalledPackageListSortOrder );

  initializeDirTreeView();
  showPackagesInDirectory();

  if (SettingsManager::getAutomaticCheckUpdates()) execAutoCheckUpdatesAvailable();
}

//Saves all application settings to ~/.config/QTGZManager.conf
void MainWindowImpl::saveSettings( int saveSettingsReason ){
  int fourPanelOrganizing = 0;
  switch(saveSettingsReason){
  case ectn_FourPanelOrganizing:
    if (actionHideRightView->isChecked()) fourPanelOrganizing += ectn_HIDE_RIGHT;
    if (actionNormalView->isChecked()) fourPanelOrganizing += ectn_NORMAL;
    if (actionMinimizeLowerView->isChecked()) fourPanelOrganizing += ectn_HIDE_BOTTON;
    if (actionMaximizeLowerView->isChecked()) fourPanelOrganizing += ectn_MAXIMIZED;
    SettingsManager::instance()->setFourPanelOrganizing(fourPanelOrganizing);
    break;

  case ectn_PackageList:
    SettingsManager::instance()->setPackageListOrderedCol(m_PackageListOrderedCol);
    SettingsManager::instance()->setPackageListSortOrder(m_PackageListSortOrder);
    break;

  case ectn_InstalledPackageList:
    SettingsManager::instance()->setInstalledPackageListOrderedCol(m_InstalledPackageListOrderedCol);
    SettingsManager::instance()->setInstalledPackageListSortOrder(m_InstalledPackageListSortOrder);
    break;

  case ectn_ToolBar:
    SettingsManager::instance()->setShowToolBar(!toolBar->isHidden());
    break;

  case ectn_DefaultDirectory:
    SettingsManager::instance()->setDefaultDirectory(m_defaultDirectory);
    break;

  case ectn_IconifyOnStart:
    SettingsManager::instance()->setStartIconified(m_actionIconifyOnStart->isChecked());
    break;
  }
}

//Interface is initialized here
void MainWindowImpl::doInits(){
  initializeToolTip();
  initializeActions();
  initializeMenuBar();
  initializeToolBar();
  initializePackageTreeView();
  initializeInstalledPackagesTreeView();
  initializeTodoTreeView();
  initializeStatusBar();

  if (!m_systemTrayIcon)
    initializeSystemTrayIcon();

  m_interfaceInitialized = true;
}

void MainWindowImpl::restoreViews(){
  int fourPanelOrganizing = SettingsManager::instance()->getFourPanelOrganizing();

  if (fourPanelOrganizing % 2 != 0){
    actionHideRightView->setChecked(true);
    hideRightView(ectn_DontSave);
    fourPanelOrganizing--;
  }

  switch(fourPanelOrganizing){
  case ectn_NORMAL :
    normalView(ectn_DontSave);
    actionNormalView->setChecked(true);
    break;
  case ectn_HIDE_BOTTON :
    minimizeLowerView(ectn_DontSave);
    actionMinimizeLowerView->setChecked(true);
    break;
  case ectn_MAXIMIZED :
    maximizeLowerView(ectn_DontSave);
    actionMaximizeLowerView->setChecked(true);
    break;
  }
}

void MainWindowImpl::initializeExitAction(){
  actionExit->setIcon(IconHelper::getIconExit());
  actionExit->setToolTip(actionExit->text());
  actionExit->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q ));
  actionExit->setIconVisibleInMenu(true);
  connect(actionExit, SIGNAL(triggered()), this, SLOT(exitApplication()));
}

void MainWindowImpl::initializeActions(){
  m_actionIconifyOnStart = new QAction(this);
  m_actionIconifyOnStart->setText(tr("Start with hidden window"));
  m_actionIconifyOnStart->setCheckable(true);
  m_actionIconifyOnStart->setChecked(false);  
  actionFreezePkg->setIcon(IconHelper::getIconFrozen());
  actionFreezePkg->setText(tr("Freeze package"));
  actionUnfreezePkg->setIcon(IconHelper::getIconUnFrozen());
  actionUnfreezePkg->setText(tr("Unfreeze package"));
  actionTransformRPMinTGZ->setIcon(QIcon(":/resources/images/tgz4.png"));
  actionTransformRPMinTGZ->setText(tr("Transform in TGZ"));
  actionChangeDefaultDirectory->setIcon(QIcon((":/resources/images/favorites.png")));
  actionChangeDefaultDirectory->setText(tr("Set as default directory"));
  actionOpenThisSnapshot->setIcon(QIcon(":/resources/images/content.png"));
  actionOpenThisSnapshot->setText(actionPackageContent->text());
  actionOpenSnapshot->setIcon(QIcon(":/resources/images/document-open.png"));
  actionOpenSnapshot->setText(tr("Open a snapshot of installed packages"));
  actionOpenSnapshot->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O ));
  actionFindFileInPkgFileList->setText(tr("Find a file"));
  actionFindFileInPkgFileList->setIcon(QIcon((":/resources/images/find.png")));
  actionFindFileInPackage->setText(tr("Find a file"));
  actionFindFileInPackage->setIcon(QIcon((":/resources/images/find.png")));
  actionFindFile->setIcon(QIcon(":/resources/images/find.png"));
  actionFindFile->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F ));
  actionExecuteActions->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E ));
  actionFindPackage->setIcon(QIcon(":/resources/images/find.png"));
  actionFindPackage->setText(tr("Find a package"));
  actionFindPackage->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_K ));
  actionInstall_package->setIcon(QIcon(":/resources/images/1rightarrow.png"));
  actionReinstall_package->setIcon(QIcon(":/resources/images/reload2.png"));
  actionUpgrade_package->setIcon(QIcon(":/resources/images/2rightarrow.png"));
  actionDowngrade_package->setIcon(QIcon(":/resources/images/2leftarrow.png"));
  actionRemove_package->setIcon(QIcon(":/resources/images/close.png"));
  actionDelete_file->setIcon(QIcon(":/resources/images/trashcan.png"));
  actionDelete_All_ActionFiles->setIcon(QIcon(":/resources/images/trashcan.png"));
  actionDelete_SelectedActionFiles->setText("Remove selected files from Action");
  actionDelete_SelectedActionFiles->setIcon(QIcon(":/resources/images/trashcan.png"));
  actionDelete_ActionFile->setIcon(QIcon(":/resources/images/trashcan.png"));
  actionCreate_Directory->setIcon(QIcon(":/resources/images/newFolder.png"));
  actionRemove_Directory->setIcon(QIcon(":/resources/images/trashcan.png"));
  actionHelp->setShortcut( QKeySequence(Qt::Key_F1) );
  actionHelp->setToolTip(tr("Help"));
  actionHelpAbout->setToolTip(StrConstants::getAbout());
  actionSnapshotInstalledPackages->setIcon(QIcon(":/resources/images/camera.png"));
  actionPackageContent->setIcon(QIcon(":/resources/images/content.png"));
  actionPackageInfo->setIcon(QIcon(":/resources/images/info.png"));
  actionInstalledPackageInfo->setIcon(QIcon(":/resources/images/info.png"));
  actionExecuteActions->setIcon(QIcon(":/resources/images/executeActions.png"));
  actionExecuteActions->setEnabled( false );
  actionSnapshotInstalledPackages->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S ));
  actionExpand_All_Items->setIcon(QIcon(":/resources/images/expand.png"));
  actionCollapse_All_Items->setIcon(QIcon(":/resources/images/collapse.png"));
  actionExpandItem->setIcon(QIcon(":/resources/images/expand.png"));
  actionExpandItem->setText(tr("Expand this item"));
  actionCollapseItem->setIcon(QIcon(":/resources/images/collapse.png"));
  actionCollapseItem->setText(tr("Collapse this item"));
  actionDiffToEachOther->setIcon(QIcon(":/resources/images/diff.png"));
  actionDiffToEachOther->setText(tr("Diff to each other"));
  actionEditFile->setIcon(QIcon(":/resources/images/editfile.png"));
  actionEditFile->setText(tr("Edit file"));
  actionOpenFile->setIcon(QIcon(":/resources/images/binary.png"));
  actionOpenFile->setText(tr("Open file"));
  actionOpenDirectory->setIcon(QIcon(":/resources/images/folder.png"));
  actionOpenDirectory->setText(tr("Open directory"));
  actionOpenDirectory->setShortcut(QKeySequence(Qt::Key_F6));
  actionOpenTerminal->setIcon(QIcon(":/resources/images/terminal.png"));
  actionOpenTerminal->setText(tr("Open in terminal"));
  actionOpenTerminal->setShortcut(QKeySequence(Qt::Key_F4));
  actionOpenTerminal->setObjectName("m_actionOpenTerminal");
  actionHideRightView->setIcon(QIcon(":/resources/images/horizontalView.png"));
  actionHideRightView->setShortcut(QKeySequence(Qt::Key_F9));
  actionNormalView->setIcon(QIcon(":/resources/images/normalView.png"));
  actionNormalView->setShortcut(QKeySequence(Qt::Key_F10));
  actionMinimizeLowerView->setIcon(QIcon(":/resources/images/minimizedLowerView.png"));
  actionMinimizeLowerView->setShortcut(QKeySequence(Qt::Key_F11));
  actionMaximizeLowerView->setIcon(QIcon(":/resources/images/verticalView.png"));
  actionMaximizeLowerView->setShortcut(QKeySequence(Qt::Key_F12));
  actionCloseTab->setIcon(QIcon(":/resources/images/window-close.png"));
  actionCloseClickedTab->setIcon(QIcon(":/resources/images/window-close.png"));
  actionCloseAllTabs->setIcon(QIcon(":/resources/images/window-close.png"));
  actionCloseAllTabs->setObjectName("actionCloseAllTabs");
  actionDiffToInstalled->setIcon(QIcon(":/resources/images/diff.png"));
  actionMaximizeLowerView->setToolTip(actionMaximizeLowerView->text());
  actionHideRightView->setToolTip(actionHideRightView->text());
  actionExecuteActions->setToolTip(actionExecuteActions->text());
  actionMaximizeLowerView->setCheckable( true );
  actionMaximizeLowerView->setChecked( false );
  actionHideRightView->setCheckable( true );
  actionHideRightView->setChecked( false );
  actionMinimizeLowerView->setCheckable( true );
  actionMinimizeLowerView->setChecked( false );
  actionNormalView->setCheckable( true );
  actionNormalView->setChecked( true );
  actionSetup->setIcon(QIcon(":/resources/images/setup.png"));
  actionSetup->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T ));
  actionCutPackage->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_X ));
  actionCopyPackage->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_C ));
  actionPastePackage->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_V));

  //Loop through all actions and set their icons (if any) visible to menus.
  for(QAction* ac: this->findChildren<QAction*>(QRegularExpression("(m_a|a)ction\\S*"))){
    ac->setIconVisibleInMenu(true);
  }

  connect(actionTransformRPMinTGZ, SIGNAL(triggered()), this, SLOT(transformRPMinTGZ()));
  connect(actionFreezePkg, SIGNAL(triggered()), this, SLOT(freezePackage()));
  connect(actionUnfreezePkg, SIGNAL(triggered()), this, SLOT(unfreezePackage()));
  connect(actionChangeDefaultDirectory, SIGNAL(triggered()), this, SLOT(changeDefaultDirectory()));
  connect(actionFindFile, SIGNAL(triggered()), this, SLOT(metaFindFile()));
  connect(actionFindPackage, SIGNAL(triggered()), this, SLOT(findPackage()));
  connect(actionOpenFile, SIGNAL(triggered()), this, SLOT(openFile()));
  connect(actionEditFile, SIGNAL(triggered()), this, SLOT(editFile()));
  connect(actionOpenDirectory, SIGNAL(triggered()), this, SLOT(openDirectory()));
  connect(actionOpenTerminal, SIGNAL(triggered()), this, SLOT(openTerminal()));
  connect(actionRefreshPackageLists, SIGNAL(triggered()), this, SLOT(refreshTreeViews()));
  connect(actionMaximizeLowerView, SIGNAL(triggered()), this, SLOT(maximizeLowerView()));
  connect(actionMinimizeLowerView, SIGNAL(triggered()), this, SLOT(minimizeLowerView()));
  connect(actionHideRightView, SIGNAL(triggered()), this, SLOT(hideRightView()));
  connect(actionNormalView, SIGNAL(triggered()), this, SLOT(normalView()));
  connect(actionExpand_All_Items, SIGNAL(triggered()), this, SLOT(expandAllContentItems()));
  connect(actionExpandItem,  SIGNAL(triggered()), this, SLOT(expandThisContentItems()));
  connect(actionCollapseItem,  SIGNAL(triggered()), this, SLOT(collapseThisContentItems()));
  connect(actionCollapse_All_Items, SIGNAL(triggered()), this, SLOT(collapseAllContentItems()));
  connect(actionCloseAllTabs, SIGNAL(triggered()), this, SLOT(closeAllTabs()));
  connect(actionCloseTab, SIGNAL(triggered()), this, SLOT(closeCurrentTab()));
  connect(actionCloseClickedTab, SIGNAL(triggered()), this, SLOT(closeClickedTab()));
  connect(actionExecuteActions, SIGNAL(triggered()), this, SLOT(executePackageActions()));
  connect(actionPackageInfo, SIGNAL(triggered()), this, SLOT(showPackageInfo()));
  connect(actionInstalledPackageInfo, SIGNAL(triggered()), this, SLOT(showInstalledPackageInfo()));
  connect(actionPackageContent, SIGNAL(triggered()), this, SLOT(showPackageContent()));
  connect(actionUpgrade_package, SIGNAL(triggered()), this, SLOT(insertUpgradePackageAction()));
  connect(actionReinstall_package, SIGNAL(triggered()), this, SLOT(insertReinstallPackageAction()));
  connect(actionInstall_package, SIGNAL(triggered()), this, SLOT(insertInstallPackageAction()));
  connect(actionDowngrade_package, SIGNAL(triggered()), this, SLOT(insertDowngradePackageAction()));
  connect(actionRemove_package, SIGNAL(triggered()), this, SLOT(insertRemovePackageAction()));
  connect(actionDelete_file, SIGNAL(triggered()), this, SLOT(deleteFile()));
  connect(actionDiffToInstalled, SIGNAL(triggered()), this, SLOT(diffToInstalled()));
  connect(actionDiffToEachOther, SIGNAL(triggered()), this, SLOT(diffToEachOther()));
  connect(actionDelete_ActionFile, SIGNAL(triggered()), this, SLOT(deleteActionFile()));
  connect(actionDelete_SelectedActionFiles, SIGNAL(triggered()), this, SLOT(deleteSelectedActionFiles()));
  connect(actionDelete_All_ActionFiles, SIGNAL(triggered()), this, SLOT(deleteAllActionFiles()));
  connect(actionCreate_Directory, SIGNAL(triggered()), this, SLOT(createDirectory()));
  connect(actionRemove_Directory, SIGNAL(triggered()), this, SLOT(removeDirectory()));
  connect(actionHelp, SIGNAL(triggered()), this, SLOT(helpQTGZManager()));
  connect(actionHelpAbout, &QAction::triggered, this, &MainWindowImpl::aboutQTGZManager);
  connect(actionDonate, &QAction::triggered, this, &MainWindowImpl::helpDonate);
  connect(actionOpenSnapshot, SIGNAL(triggered()), this, SLOT(openSnapshotOfInstalledPackages()));
  connect(actionOpenThisSnapshot, SIGNAL(triggered()), this, SLOT(openThisSnapshotOfInstalledPackages()));
  connect(actionSnapshotInstalledPackages, SIGNAL(triggered()), this, SLOT(takeSnapshotOfInstalledPackages()));
  connect(actionFindFileInPkgFileList, SIGNAL(triggered()), this, SLOT(findFileInPkgFileListExt()));
  connect(actionFindFileInPackage, SIGNAL(triggered()), this, SLOT(findFileInPackage()));
  connect(m_actionIconifyOnStart, SIGNAL(triggered()), this, SLOT(onActionIconifyTriggered()));
  connect(actionSetup, SIGNAL(triggered()), this, SLOT(showSetup()));
  connect(actionUpdater, SIGNAL(triggered()), this, SLOT(startThreadUpdater()));
  connect(actionCutPackage, SIGNAL(triggered()), this, SLOT(cutPackages()));
  connect(actionCopyPackage, SIGNAL(triggered()), this, SLOT(copyPackages()));
  connect(actionPastePackage, SIGNAL(triggered()), this, SLOT(pastePackages()));

  connect ( m_psw, SIGNAL(directoryChanged ( const QString &)),
            this, SLOT(fileSystemWatcher_installedPackagesDirectoryChanged ( const QString &) ));
  connect ( &m_fsw, SIGNAL(directoryChanged ( const QString &)),
            this, SLOT(fileSystemWatcher_packagesDirectoryChanged ( const QString &) ));
}

void MainWindowImpl::initializeToolTip(){
  qApp->setStyleSheet(StrConstants::getToolTipNormalCSS());
}

//Restore the ToolTip style to default "black on light yellow"
void MainWindowImpl::removeStyleOfToolTip(){  
  qApp->setStyleSheet(StrConstants::getToolTipRemovedCSS());
}

void MainWindowImpl::initializeDirTreeView(){
  m_titleDockDirectories->setReadOnly(true);
  m_titleDockDirectories->setEnabled(false);
  dockDirectories->setTitleBarWidget(m_titleDockDirectories);
  m_modelDir = new QDnDDirModel(this);
  m_modelDir->setFilter(QDir::AllDirs|QDir::CaseSensitive|QDir::NoDotAndDotDot);

  connect(m_modelDir, SIGNAL(rowsRemoved ( const QModelIndex &, int, int)),
          this, SLOT(showPackagesInDirectory()));
  connect(m_modelDir, SIGNAL(fileRenamed(QString,QString,QString)),
          this, SLOT(directoryRenamed(QString, QString, QString)));

  QModelIndex index;

  m_initialDir = m_argList->getSwitchArg("-initialdir", "");
  if ((m_initialDir != "") and (QFile::exists(m_initialDir))) index = m_modelDir->index(m_initialDir);
  else if (m_initialDir == "") m_initialDir = SettingsManager::instance()->getDefaultDirectory();

  m_modelDir->sort(0, Qt::AscendingOrder);
  tvDir->setSortingEnabled(true);
  tvDir->sortByColumn(0, Qt::AscendingOrder);
  m_modelDir->setRootPath("");
  m_modelDir->setIconProvider(new QDnDDirModelIconProvider(m_initialDir));

  QString style ("QLineEdit {" //"QDockWidget::title { "
                 "border: 1px solid gray;"
                 "border-top-right-radius: 2px;"
                 "border-bottom-right-radius: 2px;"
                 "text-align: left;"
                 "color: #111111;"
                 "background-color: lightgray;"
                 "font-family: \"Verdana\";"
                 "font-size: " + QString::number(SettingsManager::getDirectoryFontSize() + 4) + "px;"
                 "padding-top: 2px;"
                 "padding-bottom: 2px;"
                 "padding-left: 1px;}");

  m_titleDockDirectories->setStyleSheet(style);

  index = m_modelDir->index(m_initialDir);
  tvDir->setModel(m_modelDir);
  tvDir->setColumnHidden(1, true);
  tvDir->setColumnHidden(2, true);
  tvDir->setColumnHidden(3, true);
  tvDir->header()->hide();
  tvDir->setAcceptDrops(true);
  tvDir->setDropIndicatorShown(true);

  tvDir->scrollTo(index, QAbstractItemView::PositionAtCenter);
  tvDir->setCurrentIndex(index);
  //tvDir->setStyleSheet( StrConstants::getTreeViewCSS(SettingsManager::getDirectoryFontSize()) );

  static bool onlyOnce = false;

  if (!onlyOnce){
    connect(tvDir, SIGNAL(clicked(const QModelIndex)), this, SLOT(changeDir()));
    connect(tvDir, SIGNAL(activated(const QModelIndex)), tvDir, SIGNAL(clicked(const QModelIndex)));
    connect(tvDir, SIGNAL(clicked(const QModelIndex)), this, SLOT(showPackagesInDirectory()));
    connect(tvDir, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(execContextMenuDirectories(QPoint)));

    onlyOnce = true;
  }

  changeDir();
}

void MainWindowImpl::onTimerGotoDirectoryTimeout(){
  tvDir->scrollTo(tvDir->currentIndex(), QAbstractItemView::PositionAtTop);
  timerGotoDirectory->stop();
}

//Changes the directory showed in the first pane to the parameter received
void MainWindowImpl::gotoDirectory(QString directoryToGo){
  if (m_titleDockDirectories->text() != directoryToGo){
    QDir dir(directoryToGo);
    if (dir.exists()){
      QModelIndex index = m_modelDir->index(directoryToGo);
      tvDir->setCurrentIndex(index);      
      changeDir();
      timerGotoDirectory->start();
      showPackagesInDirectory();
    }
  }
}

void MainWindowImpl::initializePackageTreeView(){
  dockPackages->setStyleSheet(StrConstants::getDockWidgetTitleCSS());

  m_proxyModelPackage = new QSortFilterProxyModel(this);
  m_modelPackage = new QDnDStandardItemModel(this);
  m_proxyModelPackage->setSourceModel(m_modelPackage);
  m_proxyModelPackage->setFilterKeyColumn(1);

  if (SettingsManager::instance()->getShowPackageTooltip())
    tvPackage->setItemDelegate(new tvPackagesItemDelegate(tvPackage));

  tvPackage->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tvPackage->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
  tvPackage->setAllColumnsShowFocus( true );
  tvPackage->setModel(m_proxyModelPackage);
  tvPackage->setSortingEnabled( true );
  tvPackage->sortByColumn( 1, Qt::AscendingOrder);
  tvPackage->setIndentation( 0 );
  tvPackage->header()->setSortIndicatorShown(true);
  tvPackage->header()->setSectionsClickable(true);
  tvPackage->header()->setSectionsMovable(false);
  tvPackage->header()->setDefaultAlignment( Qt::AlignCenter );
  tvPackage->header()->setSectionResizeMode( QHeaderView::Fixed );
  //tvPackage->setStyleSheet(StrConstants::getTreeViewCSS(SettingsManager::getPackagesInDirFontSize()));

  //Prepare it for drag operations
  tvPackage->setSelectionMode(QAbstractItemView::ExtendedSelection);
  tvPackage->setDragEnabled(true);
  dockPackages->setWindowTitle(ctn_LABEL_TREEVIEW_PACKAGES);

#if QT_VERSION >= 0x040700
  leFilterPackage->setPlaceholderText(tr("Filter"));
#else
  leFilterPackage->setToolTip(tr("Filter"));
#endif

  static bool onlyOnce=true;
  if (onlyOnce){
    connect(leFilterPackage, SIGNAL(textChanged (const QString&)), this, SLOT(reapplyPackageFilter()));
    connect(tvPackage->header(), SIGNAL( sectionClicked ( int )),
            this, SLOT( headerViewPackageList_click( int ) ) );
    connect(tvPackage, SIGNAL(clicked(const QModelIndex)), this, SLOT(selectInstalledPackage()));
    connect(tvPackage, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(execContextMenuPackages(QPoint)));
    connect(tvPackage, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showPackageContent()));

    connect(tvPackage->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,
            SLOT(tvPackageSelectionChanged(QItemSelection,QItemSelection)));
    onlyOnce=false;
  }
}

void MainWindowImpl::initializeInstalledPackagesTreeView(){
  dockInstalledPackages->setStyleSheet(StrConstants::getDockWidgetTitleCSS());

  m_proxyModelInstalledPackages = new QSortFilterProxyModel(this);
  m_modelInstalledPackages = new QStandardItemModel(this);
  m_proxyModelInstalledPackages->setSourceModel(m_modelInstalledPackages);
  m_proxyModelInstalledPackages->setFilterKeyColumn(1);

#if QT_VERSION >= 0x040700
  leFilterInstalledPackages->setPlaceholderText(tr("Filter"));
#else
  leFilterInstalledPackages->setToolTip(tr("Filter"));
#endif

  QStringList list = Package::getInstalledPackageNames();
  QStandardItem *parentItem = m_modelInstalledPackages->invisibleRootItem();
  QList<QStandardItem*> lIcons, lNames;

  if ( m_frozenPkgList->isEmpty() ){
    for( QString s: list ){
      QStandardItem *i = new QStandardItem( IconHelper::getIconUnFrozen(), "N");
      lIcons << i;
      //lIcons << new QStandardItem(IconHelper::getIconUnFrozen(), "_UnFrozen" );

      lNames << new QStandardItem( s );
    }
  }
  else{
    for( QString s: list ){
      if ( m_frozenPkgList->indexOf( QRegExp(QRegExp::escape(Package::getBaseName(s))), 0 ) == -1 )
      {
        QStandardItem *i = new QStandardItem( IconHelper::getIconUnFrozen(), "N");
        lIcons << i;

        //lIcons << new QStandardItem( IconHelper::getIconUnFrozen(), "_UnFrozen" );
      }
      else
      {
        QStandardItem *i = new QStandardItem( IconHelper::getIconFrozen(), "F");
        lIcons << i;

        //lIcons << new QStandardItem( IconHelper::getIconFrozen(), "_Frozen" );
      }

      lNames << new QStandardItem( s );
    }
  }

  parentItem->insertColumn(0, lIcons );
  parentItem->insertColumn(1, lNames );

  QStringList sl;
  m_modelInstalledPackages->setHorizontalHeaderLabels(sl << "" << tr("Name"));
  tvInstalledPackages->setModel(m_proxyModelInstalledPackages);

  dockInstalledPackages->setWindowTitle(tr("%1 Packages Installed").arg(QString::number(list.size())));
  tvInstalledPackages->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  if (SettingsManager::instance()->getShowPackageTooltip())
    tvInstalledPackages->setItemDelegate(new tvPackagesItemDelegate(tvInstalledPackages));

  //tvInstalledPackages->setStyleSheet(StrConstants::getTreeViewCSS(SettingsManager::getInstalledPackagesFontSize()));
  tvInstalledPackages->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tvInstalledPackages->setSelectionMode(QAbstractItemView::ExtendedSelection);
  tvInstalledPackages->setSortingEnabled( true );
  //tvInstalledPackages->sortByColumn( 1, Qt::AscendingOrder);
  tvInstalledPackages->setIndentation( 0 );
  tvInstalledPackages->setAllColumnsShowFocus( true );
  tvInstalledPackages->header()->setSortIndicatorShown(true);
  tvInstalledPackages->header()->setSectionsClickable(false);
  tvInstalledPackages->header()->setSectionsMovable(false);
  tvInstalledPackages->setColumnWidth(0, 24);
  tvInstalledPackages->setColumnWidth(1, 50);
  //tvInstalledPackages->header()->setSortIndicator(
  //      m_InstalledPackageListOrderedCol, m_InstalledPackageListSortOrder );
  //tvInstalledPackages->sortByColumn( m_InstalledPackageListOrderedCol, m_InstalledPackageListSortOrder );
  tvInstalledPackages->header()->setSortIndicator(1, Qt::AscendingOrder); //m_InstalledPackageListSortOrder );
  tvInstalledPackages->sortByColumn( 1, Qt::AscendingOrder); //m_InstalledPackageListSortOrder );
  tvInstalledPackages->header()->setDefaultAlignment( Qt::AlignCenter );
  tvInstalledPackages->header()->setSectionResizeMode( QHeaderView::Fixed );
  tvInstalledPackages->setUniformRowHeights(true);

  static bool onlyOnce=true;
  if (onlyOnce){
    connect(leFilterInstalledPackages, SIGNAL(textChanged (const QString&)),
            this, SLOT(reapplyInstalledPackagesFilter()));
    connect(tvInstalledPackages->header(), SIGNAL( sectionClicked ( int )),
            this, SLOT( headerViewInstalledPackageList_click( int ) ) );
    connect(tvInstalledPackages, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showPackageContent()));
    connect(tvInstalledPackages, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(execContextMenuInstalledPackages(QPoint)));

    connect(tvInstalledPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(tvInstalledPackagesSelectionChanged(QItemSelection,QItemSelection)));

    onlyOnce=false;
  }
}

void MainWindowImpl::initializeTodoTreeView(){
  m_modelTodo = new QDnDStandardItemModel(this);

  if (SettingsManager::instance()->getShowPackageTooltip())
    tvTODO->setItemDelegate(new tvPackagesItemDelegate(tvTODO));

  tvTODO->setModel(m_modelTodo);
  tvTODO->setSelectionMode(QAbstractItemView::ExtendedSelection);
  tvTODO->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tvTODO->setDropIndicatorShown(true);
  tvTODO->setAcceptDrops(true);
  tvTODO->header()->setSortIndicatorShown(false);
  tvTODO->header()->setSectionsClickable(false);
  tvTODO->header()->setSectionsMovable(false);
  tvTODO->header()->setDefaultAlignment( Qt::AlignCenter );
  //tvTODO->setStyleSheet( StrConstants::getTreeViewCSS(SettingsManager::getTodoFontSize()) );

  textEdit->setStyleSheet("QTextEdit::font { font-family:\"Verdana\";"
                                     " font-size: " + QString::number(SettingsManager::getTodoFontSize()) + "px;}" );

  textEdit->setObjectName("textBrowser");

  SearchBar *searchBar = new SearchBar(this);
  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChanged(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosed()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNext()));
  connect(searchBar, SIGNAL(findPrevious()), this, SLOT(searchBarFindPrevious()));

  QGridLayout *gLayout = qobject_cast<QGridLayout*>(tabOutput->layout());
  gLayout->addWidget(searchBar, 1, 0, 1, 1);

  m_modelTodo->setSortRole(0);
  m_modelTodo->setColumnCount(0);
  m_downgrade = new QStandardItem(IconHelper::getIconInferior(), StrConstants::getTodoDowngradeText());
  m_install = new QStandardItem(IconHelper::getIconNotInstalled(), StrConstants::getTodoInstallText());
  m_remove = new QStandardItem(IconHelper::getIconRemove(), StrConstants::getTodoRemoveText());
  m_upgrade = new QStandardItem(IconHelper::getIconSuperior(), StrConstants::getTodoUpgradeText());
  m_reinstall = new QStandardItem(IconHelper::getIconInstalled(), StrConstants::getTodoReinstallText());

  QStandardItem* parentItem = m_modelTodo->invisibleRootItem();
  parentItem->appendRow(m_downgrade);
  parentItem->appendRow(m_install);
  parentItem->appendRow(m_reinstall);
  parentItem->appendRow(m_remove);
  parentItem->appendRow(m_upgrade);

  m_modelTodo->setHorizontalHeaderLabels(QStringList() << tr("Package Actions ( Ctrl+E to execute )"));

  twTODO->setTabPosition(QTabWidget::North);
  twTODO->setUsesScrollButtons(true);
  twTODO->setElideMode(Qt::ElideNone);
  twTODO->setTabsClosable(true);
  twTODO->setMovable(true);
  twTODO->initTabBar();

  m_tabBar = twTODO->getTabBar();
  connect(m_tabBar, SIGNAL(tabMoved(int,int)), this, SLOT(onTabMoved(int, int)));
  connect(m_modelTodo, SIGNAL(rowsInserted ( const QModelIndex , int, int )),
          this, SLOT(verifyStateTODO ( const QModelIndex , int, int )));
  connect(m_modelTodo, SIGNAL(rowsRemoved ( const QModelIndex , int, int )),
          this, SLOT(verifyStateTODO ( const QModelIndex , int, int )));

  connect(tvTODO, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(execContextMenuTodoTreeView(QPoint)));
  connect(tvTODO->model(), SIGNAL(rowsInserted(const QModelIndex&, int, int)), this,
          SLOT(tvTODORowsInserted(const QModelIndex&, int, int)));
  connect(tvTODO->model(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)), this,
          SLOT(tvTODORowsRemoved(const QModelIndex&,int,int)));
  connect(tvTODO->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(tvTODOSelectionChanged(QItemSelection,QItemSelection)));

  connect(twTODO, SIGNAL(currentChanged(int)), this, SLOT(enableCloseTabButton(int)) );
  connect(twTODO, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)) );

  m_tabBar->setFont(QFont("Verdana", SettingsManager::getTodoFontSize() + 1));

  twTODO->setCurrentIndex(0);
}

void MainWindowImpl::initializeMenuBar(){
  menubar->setContextMenuPolicy(Qt::NoContextMenu);
  menuFile->addAction(actionOpenSnapshot);
  menuFile->addAction(actionSnapshotInstalledPackages);
  menuFile->addSeparator();
  menuFile->addAction(actionExecuteActions);
  menuFile->addAction(actionFindFile);
  menuFile->addAction(actionFindPackage);
  menuFile->addSeparator();
  menuFile->addAction(actionUpdater);
  menuFile->addAction(actionExit);
  menuView->addAction(actionHideRightView);
  menuView->addAction(actionNormalView);
  menuView->addAction(actionMinimizeLowerView);
  menuView->addAction(actionMaximizeLowerView);
  menuOptions->addAction(actionSetup);
  menuHelp->addAction(actionHelp);
  menuHelp->addSeparator();
  menuHelp->addAction(actionDonate);
  menuHelp->addAction(actionHelpAbout);
  menubar->setVisible(true);
  menubar->setStyleSheet(" QMenuBar { font: " + QString::number(SettingsManager::getMenuFontSize()) + "px; }");

  setStyleSheet("QMenu { font: " + QString::number(SettingsManager::getMenuFontSize()) + "px; }");
}

void MainWindowImpl::initializeToolBar(){
  QActionGroup *ag = new QActionGroup(this);

  toolBar->setWindowTitle(tr("Show toolbar"));
  toolBar->setStyleSheet(StrConstants::getToolBarCSS());
  toolBar->setToolButtonStyle ( Qt::ToolButtonIconOnly );
  toolBar->addAction(actionHideRightView);
  toolBar->addAction(ag->addAction(actionNormalView));
  toolBar->addAction(ag->addAction(actionMinimizeLowerView));
  toolBar->addAction(ag->addAction(actionMaximizeLowerView));
  toolBar->addSeparator();
  toolBar->addAction(actionFindFile);
  toolBar->addAction(actionExecuteActions);
  toolBar->addAction(actionSnapshotInstalledPackages);
  toolBar->addSeparator();
  toolBar->addAction(actionExit);

  connect(toolBar->toggleViewAction(), SIGNAL(triggered()), this, SLOT(showToolBar()));
}

void MainWindowImpl::initializeSystemTrayIcon(){
  m_systemTrayIcon = new QSystemTrayIcon( *m_appIcon, this );
  m_systemTrayIcon->setObjectName("systemTrayIcon");
  m_systemTrayIcon->setToolTip( StrConstants::getApplicationName() );
  m_systemTrayIcon->show();

  m_systemTrayIconMenu = new QMenu( this );
  m_actionHelp = new QAction( this );
  m_actionHelp->setText( tr("Help"));
  m_systemTrayIconMenu->setStyleSheet(" QMenu { font: " + QString::number(SettingsManager::getMenuFontSize()) + "px; }");
  m_systemTrayIconMenu->addAction( m_actionHelp );
  m_systemTrayIconMenu->addAction( actionExit );
  m_systemTrayIcon->setContextMenu( m_systemTrayIconMenu );

  connect( m_actionHelp, SIGNAL(triggered()), this, SLOT(helpQTGZManager()));
  connect ( m_systemTrayIcon , SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ),
            this, SLOT( execSystemTrayActivated ( QSystemTrayIcon::ActivationReason ) ) );
}

void MainWindowImpl::initializeStatusBar(){
  m_lblStatus->setFrameStyle( QFrame::NoFrame );
  m_statusbar = new SimpleStatusBar( this );
  setStatusBar( m_statusbar );
  statusBar()->addPermanentWidget( m_lblStatus, true );

  if (!SettingsManager::getShowStatusBar())
    statusBar()->hide();
}

//Pops the Options/Settings dialog up.
void MainWindowImpl::showSetup(){
  SetupDialog *setupDialog = new SetupDialog(this);
  setupDialog->setFontSize(SettingsManager::getMenuFontSize());
  setupDialog->show();    

  connect(setupDialog, SIGNAL(accepted()), this, SLOT(refreshUIAfterSetup()));
}

//Changes the number of selected items in the second pane: YY in XX(YY) Packages in directory
void MainWindowImpl::tvPackageSelectionChanged(const QItemSelection&, const QItemSelection&){
  dockPackages->setWindowTitle(tr("%1 (%2) Packages in Directory").arg(
                                 QString::number(m_proxyModelPackage->rowCount())).
                               arg(QString::number(tvPackage->selectionModel()->selectedRows().count())));
  selectInstalledPackage();
}

//Changes the number of selected items in the third pane: YY in XX(YY) Packages installed
void MainWindowImpl::tvInstalledPackagesSelectionChanged(const QItemSelection&, const QItemSelection&){
  dockInstalledPackages->setWindowTitle(tr("%1 (%2) Packages Installed").arg(
                                 QString::number(m_proxyModelInstalledPackages->rowCount())).
                               arg(QString::number(tvInstalledPackages->selectionModel()->selectedRows().count())));
}
