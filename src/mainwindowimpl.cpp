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

#include "mainwindowimpl.h"
#include "package.h"
#include "tvpackagesitemdelegate.h"
#include "unixcommand.h"
#include "wmhelper.h"
#include "argumentlist.h"
#include "updater.h"
#include "updaterthread.h"
#include "strconstants.h"
#include "searchbar.h"
#include "uihelper.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QFutureWatcher>
#include <QDesktopServices>
#include <QtConcurrent/QtConcurrentRun>

using namespace QtConcurrent;

MainWindowImpl::MainWindowImpl(QWidget * parent)
  :	QMainWindow(parent),
  ctn_LABEL_TREEVIEW_PACKAGES(tr("0 Packages in Directory")),
  m_ignoreCancelledActions(false),
  m_updaterThread(0),
  m_reinstallSlackGPGKeyThread(0),
  m_modelPackage(0),
  m_modelTodo(0),
  m_modelInstalledPackages(0)
{
  m_modelDir=0;
  m_unixCommand=0;
  m_install=0;
  m_remove=0;
  m_downgrade=0;
  m_upgrade=0;
  m_reinstall=0;
  m_interfaceInitialized = false;
  m_reallyWannaClose = !SettingsManager::getWindowCloseHidesApp();
  m_frozenPkgList = FrozenPkgListSingleton::instance();
	m_clickedTab = -1;  
  m_findDialog = 0;
  m_lblStatus = new QLabel(this);
  m_fw = new QFutureWatcher<QStringList>(this);
  m_argList = new ArgumentList();
  m_appIcon = new QIcon(":/resources/images/QTGZ.png");
  m_systemTrayIcon = 0;
  m_titleDockDirectories = new QLineEdit(this);
  m_titleDockDirectories->setObjectName("titleDockDirectories");
  m_psw = new QFileSystemWatcher(QStringList() << ctn_PACKAGES_DIR, this);
  InstalledPkgListSingleton::instance()->setFileSystemWatcher(m_psw);
  m_fsw.addPath( QDir::homePath() );
  m_foundFilesInPkgFileList = new QList<QModelIndex>();

  setupUi( this );
  setWindowTitle(StrConstants::getApplicationName());
  setWindowIcon(*m_appIcon);
  setMinimumSize(QSize(640, 480));

  initializeExitAction();

  m_savedGeometry = this->saveGeometry();
  qApp->installEventFilter(this);

  timerGotoDirectory = new QTimer(this);
  connect(timerGotoDirectory, SIGNAL(timeout()), this, SLOT(onTimerGotoDirectoryTimeout()));
  timerGotoDirectory->setInterval(50);
}

MainWindowImpl::~MainWindowImpl(){
  _collectActionExecGarbage();
  Package::removeTempFiles();
  //Remove lock files from QtSingleApp
  UnixCommand::removeTemporaryFiles();

  if (m_modelDir != 0)
    delete m_modelDir;

  if (m_argList != 0)
    delete m_argList;

  if (m_modelPackage != 0)
    delete m_modelPackage;

  if (m_modelInstalledPackages != 0)
    delete m_modelInstalledPackages;

  if (m_install != 0)
    delete m_install;

  if (m_remove != 0)
    delete m_remove;

  if (m_downgrade != 0)
    delete m_downgrade;

  if (m_upgrade != 0)
    delete m_upgrade;

  if (m_reinstall != 0)
    delete m_reinstall;

  if (m_modelTodo != 0)
    delete m_modelTodo;
}

void MainWindowImpl::_collectActionExecGarbage(){
  if (m_unixCommand){
    //This should not have happened...
    m_unixCommand->removeTemporaryActionFile();
    delete m_unixCommand;
    m_unixCommand = 0;
  }
}

void MainWindowImpl::show(){
  static int appearence=1;
  static bool firstExecution=true;
  QByteArray windowSize=SettingsManager::getWindowSize();

  if (firstExecution || appearence <= 2){
    firstExecution=false;

    if ((m_argList->getSwitch("-starticonified") ||
         SettingsManager::instance()->getStartIconified()) && (appearence == 1)){

      initializeSystemTrayIcon();
      hide();
      appearence++;
    }
    else{
      if (!m_interfaceInitialized)
        loadSettings();

      if(windowSize.size() > 1){
        restoreGeometry(windowSize);
        QMainWindow::show();
      }
      else
        QMainWindow::showMaximized();

      restoreViews();
      appearence=3;
    }
  }
  else{
    if (appearence==2){
      restoreGeometry(windowSize);
      appearence=3;
    }

    if (!m_interfaceInitialized){
      QMainWindow::show();
      loadSettings();
    }
    else QMainWindow::show();
  }

  QModelIndex index = m_modelDir->index(m_initialDir);
  tvDir->scrollTo(index, QAbstractItemView::PositionAtCenter);
  tvDir->setCurrentIndex(index);
}

void MainWindowImpl::changeDir(){
  dockDirectories->setWindowTitle(m_modelDir->filePath(tvDir->currentIndex()));
  m_titleDockDirectories->setText(dockDirectories->windowTitle());
  m_titleDockDirectories->setToolTip(m_titleDockDirectories->text());
  tvDir->expand(tvDir->currentIndex());
}

void MainWindowImpl::fileSystemWatcher_packagesDirectoryChanged ( const QString &directory ){
  QDir dir(directory);
  if (dir.exists())
    showPackagesInDirectory();
}

void MainWindowImpl::fileSystemWatcher_installedPackagesDirectoryChanged( const QString& ){
  refreshInstalledPackageTreeView();
  showPackagesInDirectory();
}

void MainWindowImpl::headerViewPackageList_click( int col ){
	m_PackageListOrderedCol = col;
	m_PackageListSortOrder = tvPackage->header()->sortIndicatorOrder();

  saveSettings(ectn_PackageList);
}

void MainWindowImpl::headerViewInstalledPackageList_click( int col ){
	m_InstalledPackageListOrderedCol = col;
	m_InstalledPackageListSortOrder = tvInstalledPackages->header()->sortIndicatorOrder();

  saveSettings(ectn_InstalledPackageList);
}

void MainWindowImpl::enableCloseTabButton( int ){
	if (twTODO->count() <= 2){
		twTODO->removeAction( actionCloseTab );
        m_lblStatus->setText("");
	}
	else if (twTODO->count() > 2){
		if (twTODO->count() >= 3){
			if ((twTODO->count() > 3) && (twTODO->currentWidget()->findChild<QAction*>("actionCloseAllTabs") == 0))
				twTODO->currentWidget()->addAction( actionCloseAllTabs );	

			//we verify if there is a tvPkgFileList active
			QTreeView *t = twTODO->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
			if ( t != 0){ 
				t->setFocus();
				showFullPathOfObject( t->currentIndex() );
			}
			else{ 
        m_lblStatus->setText("");
			}
		}
    else twTODO->currentWidget()->removeAction( actionCloseAllTabs );
	}
}

void MainWindowImpl::verifyStateTODO( const QModelIndex , int, int ){
  if (hasPendingActions())
		actionExecuteActions->setEnabled( true );
	else actionExecuteActions->setEnabled( false );
}

void MainWindowImpl::execContextMenuDirectories(QPoint point){	
	showPackagesInDirectory();
	QMenu menu(this);            

  if (!isFindDisabled())
    menu.addAction(actionFindPackage);

  menu.addAction(actionOpenDirectory);
  menu.addAction(actionOpenTerminal);

  if (dockDirectories->windowTitle() != m_defaultDirectory &&
      dockDirectories->windowTitle() != m_updaterDirectory)
    menu.addAction(actionChangeDefaultDirectory);

  menu.addAction(actionCreate_Directory);

  if (dockDirectories->windowTitle() != m_defaultDirectory &&
      dockDirectories->windowTitle() != m_updaterDirectory)
    menu.addAction(actionRemove_Directory);

  if (m_packagesClipboard.count() > 0){
    menu.addSeparator();
    menu.addAction(actionPastePackage);
  }

	QPoint pt2 = tvDir->mapToGlobal(point);
	pt2.setY(pt2.y() + tvDir->header()->height());
	menu.exec(pt2);        	        	
}

void MainWindowImpl::diffToEachOther(){
  tvPackage->repaint(tvPackage->rect());
	QCoreApplication::processEvents();	
  CPUIntensiveComputing ri;
  SelectedPackage pkg1 = getSelectedPackage()[0];
	SelectedPackage pkg2 = getSelectedPackage()[1];

	for (int c=2; c<twTODO->count(); c++)
    if (twTODO->tabText(c) == ( pkg1.getFileName() + " DIFF")){
    twTODO->setCurrentIndex ( c );
    conditionalGotoNormalViewSimple();
    return;
  }

  QString out = UnixCommand::executeDiffToEachOther(pkg1.getCompleteFileName(), pkg2.getCompleteFileName());
  if (out == 0){
    QMessageBox::critical(MainWindowImpl::returnMainWindow(), QObject::tr("Diff"),
                          QObject::tr("This package seems corrupted!"));
    return;
  }
  else if (out == ctn_PACKAGES_WITH_SAME_CONTENT){
    QMessageBox::information(MainWindowImpl::returnMainWindow(), QObject::tr("Diff"),
                             QObject::tr("The packages have the same file list!"));
    return;
  }

  conditionalGotoNormalViewSimple();
  createTabPkgDiff(pkg1.getFileName(), pkg2.getFileName(), out);
}

void MainWindowImpl::diffToInstalled(){
  tvPackage->repaint(tvPackage->rect());
	QCoreApplication::processEvents();	
  CPUIntensiveComputing ri;
  SelectedPackage pkg = getSelectedPackage()[0];

	for (int c=2; c<twTODO->count(); c++)
    if (twTODO->tabText(c) == ( pkg.getFileName() + " DIFF(inst)")){
    twTODO->setCurrentIndex ( c );
    conditionalGotoNormalViewSimple();
    return;
  }

  Result res = Package::getStatus(pkg.getFileName());

  QString out = UnixCommand::executeDiffToInstalled(pkg.getCompleteFileName(), res.getInstalledPackage());
  if (out == 0){
    QMessageBox::critical(MainWindowImpl::returnMainWindow(), QObject::tr("Diff"),
                          QObject::tr("This package seems corrupted!"));
    return;
  }

  conditionalGotoNormalViewSimple();
  createTabPkgDiff(pkg.getFileName(), res.getInstalledPackage(), out);
}

void MainWindowImpl::execContextMenuPackages(QPoint point){
  if ((m_modelPackage->rowCount()==0) || (tvPackage->selectionModel()->selectedRows().count()==0)){
    //Do we have packages in ClipBoard?
    if (m_packagesClipboard.count() > 0){
      QMenu *menu = new QMenu(this);
      menu->addAction(actionPastePackage);
      QPoint pt2 = tvPackage->mapToGlobal(point);
      pt2.setY(pt2.y() + tvPackage->header()->height());
      menu->exec(pt2);
    }

    return;
  }

	QMenu *menu = new QMenu(this);            
	QIcon icon;  

	//Which actions do we have to put? It depends on package status
	bool allSameType=true;
  bool allTGZ=false;
  int tgzCount=0;
  bool allRPM=false;
  int rpmCount=0;
  int tgzOtherVersion=0;
  int tgzSuperior=0;

  QIcon lastType;

	QList<SelectedPackage> lsp = getSelectedPackage();	

  for(SelectedPackage ami: lsp){
    if ((lastType.cacheKey() != 0) &&
        (lastType.pixmap(QSize(22,22)).toImage()) != ami.getIcon().pixmap(QSize(22,22)).toImage())
      allSameType = false;
    if (ami.getIcon().pixmap(QSize(22,22)).toImage() ==
        IconHelper::getIconRPM().pixmap(QSize(22,22)).toImage())
			rpmCount++;
    else if (ami.getIcon().pixmap(QSize(22,22)).toImage() !=
        IconHelper::getIconBinary().pixmap(QSize(22,22)).toImage())
      tgzCount++;

    if (ami.getIcon().pixmap(QSize(22,22)).toImage() ==
        IconHelper::getIconOtherVersion().pixmap(QSize(22,22)).toImage())
      tgzOtherVersion++;
    else if (ami.getIcon().pixmap(QSize(22,22)).toImage() ==
        IconHelper::getIconSuperior().pixmap(QSize(22,22)).toImage())
      tgzSuperior++;

    lastType = ami.getIcon();
	}

  //Treat packages classified as OtherVersion the same as SuperiorVersion.
  if (!allSameType && rpmCount==0 && (tgzOtherVersion+tgzSuperior == tgzCount))
    allSameType = true;

	if ( tgzCount == lsp.count() ) {
		allTGZ = true;
		allRPM = false;
	}
	else if ( rpmCount == lsp.count() ) {
		allTGZ = false;
		allRPM = true;
	}

	if (allTGZ){
		//First Info, so when the user mistakenly clicks in it, QTGZ does less processing.
		menu->addAction(actionPackageContent);

		if (tgzCount == 1){
			menu->addAction(actionPackageInfo); 	 
      if (!isFindDisabled()) menu->addAction(actionFindFileInPackage);
		}
	}
  else if ( (allRPM) && (UnixCommand::hasTheExecutable(ctn_RPM2TGZBIN)) ){
    menu->addAction(actionTransformRPMinTGZ);
	}

	if ( lsp.count() == 1 ){   
    //Here, we include the logic for package diff to installed
		icon = lsp[0].getIcon();
    if ( (icon.pixmap(QSize(22,22)).toImage() == IconHelper::getIconSuperior().pixmap(QSize(22,22)).toImage()) ||
         (icon.pixmap(QSize(22,22)).toImage() == IconHelper::getIconInferior().pixmap(QSize(22,22)).toImage()) ||
         (icon.pixmap(QSize(22,22)).toImage() == IconHelper::getIconOtherVersion().pixmap(QSize(22,22)).toImage()) ||
         (icon.pixmap(QSize(22,22)).toImage() == IconHelper::getIconOtherArch().pixmap(QSize(22,22)).toImage())) {
      menu->addAction(actionDiffToInstalled);
		}
	}
	else if (( lsp.count() == 2 ) && (allTGZ) ) {  
    //Here, we include the logic for package diff to each other
		if ( lsp[0].getFileName() != lsp[1].getFileName()){
			QString s1 = Package::getBaseName(lsp[0].getFileName());
			QString s2 = Package::getBaseName(lsp[1].getFileName());
      if (s1 == s2) menu->addAction(actionDiffToEachOther);
		}
	}

	if ((allSameType) && (allTGZ) ) {
		icon = lsp[0].getIcon();
    if (icon.pixmap(QSize(22,22)).toImage() == IconHelper::getIconInferior().pixmap(QSize(22,22)).toImage()) {
			menu->addAction(actionDelete_file);
			menu->addAction(actionDowngrade_package);        		        		
		}
    else if (icon.pixmap(QSize(22,22)).toImage() == IconHelper::getIconSuperior().pixmap(QSize(22,22)).toImage()){
			menu->addAction(actionDelete_file);
			menu->addAction(actionUpgrade_package);        		        		       		        		
		}
    else if (icon.pixmap(QSize(22,22)).toImage() == IconHelper::getIconOtherVersion().pixmap(QSize(22,22)).toImage()){
			menu->addAction(actionDelete_file);
			menu->addAction(actionUpgrade_package);        		        		       		        		
		}
    else if (icon.pixmap(QSize(22,22)).toImage() == IconHelper::getIconInstalled().pixmap(QSize(22,22)).toImage()){
			menu->addAction(actionDelete_file);
			menu->addAction(actionReinstall_package);
			menu->addAction(actionRemove_package);        		        		       		        		       		
		}
    else if (icon.pixmap(QSize(22,22)).toImage() == IconHelper::getIconNotInstalled().pixmap(QSize(22,22)).toImage() ||
             icon.pixmap(QSize(22,22)).toImage() == IconHelper::getIconOtherArch().pixmap(QSize(22,22)).toImage()){
			menu->addAction(actionDelete_file);
      menu->addAction(actionInstall_package);
		}        	
    else if (icon.pixmap(QSize(22,22)).toImage() == IconHelper::getIconInternalError().pixmap(QSize(22,22)).toImage()) {
			menu->addAction(actionDelete_file);        		   		
		}
    else if (icon.pixmap(QSize(22,22)).toImage() == IconHelper::getIconFrozen().pixmap(QSize(22,22)).toImage()) {
			menu->addAction(actionDelete_file);        		   		
		}
	}
    else if (icon.pixmap(QSize(22,22)).toImage() == IconHelper::getIconBinary().pixmap(QSize(22,22)).toImage()) {
      menu->addAction(actionOpenThisSnapshot);
      menu->addAction(actionDelete_file);
    }
	else{
		menu->addAction(actionDelete_file);
	}	

  menu->addSeparator();
  menu->addAction(actionCutPackage);
  menu->addAction(actionCopyPackage);
	QPoint pt2 = tvPackage->mapToGlobal(point);
	pt2.setY(pt2.y() + tvPackage->header()->height());
	menu->exec(pt2);        	
}

void MainWindowImpl::execContextMenuInstalledPackages(QPoint point){  
  if ((m_modelInstalledPackages->rowCount()==0) ||
      (tvInstalledPackages->selectionModel()->selectedIndexes().count()==0)) return;

  QMenu menu(this);

  //Which actions do we have to put? It depends on package status!
	bool allSameType = true;
  QIcon lastType;

  for(QModelIndex item: tvInstalledPackages->selectionModel()->selectedIndexes()){
    if (item.column() == ctn_PACKAGE_ICON){
      QModelIndex mi = m_proxyModelInstalledPackages->mapToSource(item);
      if ((lastType.cacheKey() != 0) &&
          (lastType.pixmap(QSize(22,22)).toImage() !=
           m_modelInstalledPackages->item( mi.row(), mi.column() )->icon().pixmap(QSize(22,22)).toImage())) {
				allSameType = false; 
				break;
			}
      lastType = m_modelInstalledPackages->item( mi.row(), mi.column() )->icon();
		}
	}

  if (allSameType == true){
    if (lastType.pixmap(QSize(22,22)).toImage() == IconHelper::getIconFrozen().pixmap(QSize(22,22)).toImage())
      menu.addAction(actionUnfreezePkg);
    else
      menu.addAction(actionFreezePkg);
	}	        		        		       		        		       		

  menu.addSeparator();
  menu.addAction(actionPackageContent);

  if ( tvInstalledPackages->selectionModel()->selectedIndexes().count() == 2){
    menu.addAction(actionInstalledPackageInfo);
    if (!isFindDisabled()) menu.addAction(actionFindFileInPackage);
  }

  if (lastType.pixmap(QSize(22,22)).toImage() != IconHelper::getIconFrozen().pixmap(QSize(22,22)).toImage())
    menu.addAction(actionRemove_package);

	QPoint pt2 = tvInstalledPackages->mapToGlobal(point);
	pt2.setY(pt2.y() + tvInstalledPackages->header()->height());
  menu.exec(pt2);
}

void MainWindowImpl::conditionalGotoNormalView(){
  if (isHidden()) show();

  if (!actionMaximizeLowerView->isChecked() && !actionNormalView->isChecked()){
    actionNormalView->setChecked(true);
    normalView();
  }
}

void MainWindowImpl::conditionalGotoNormalViewSimple(){
  if (isHidden()) show();

  if (!actionNormalView->isChecked()){
    actionNormalView->setChecked(true);
    normalView();
  }
}

void MainWindowImpl::execSystemTrayActivated ( QSystemTrayIcon::ActivationReason ar ){
  switch (ar) {
  case QSystemTrayIcon::Trigger:
  case QSystemTrayIcon::DoubleClick:
    if ( this->isHidden() ){
      this->restoreGeometry( m_savedGeometry );
			if (this->isMinimized()) this->setWindowState(Qt::WindowNoState);
			this->show();
		}
    else {
      m_savedGeometry = this->saveGeometry();
			this->hide();
		}
    break;
  default: break;
  }
}

void MainWindowImpl::_deallocateTabContent(int tabIndex){
  QTreeView *t = twTODO->widget(tabIndex)->findChild<QTreeView*>("tvPkgFileList");
  if ( t != 0){
    QStandardItemModel *sim = (QStandardItemModel*) t->model();
    sim->clear();

    delete sim;
    delete t;
  }
  else{
    QTextBrowser *tb = twTODO->widget(tabIndex)->findChild<QTextBrowser*>("textBrowser");
    if (tb) tb->setHtml("");
    SearchBar *sb = twTODO->widget(tabIndex)->findChild<SearchBar*>("searchbar");
    if (sb) delete sb;
  }
}

void MainWindowImpl::tabCloseRequested( int tab ){
  m_clickedTab = tab;
  closeClickedTab();
}

void MainWindowImpl::closeClickedTab(){
  if ((m_clickedTab != -1) && (m_clickedTab != 0) && (m_clickedTab != 1)){
    _deallocateTabContent(m_clickedTab);
    twTODO->removeTab(m_clickedTab);
  }
	if (twTODO->count() == 2){
		twTODO->setCurrentIndex(0);
		enableCloseTabButton(-1);	
    //If we have only two tabs, we can switch focus to package list
    tvPackage->setFocus();
	}
}

void MainWindowImpl::closeCurrentTab() {
	if (twTODO->currentWidget()->isActiveWindow() && (twTODO->currentIndex()!=0 && twTODO->currentIndex()!=1)){ 
    _deallocateTabContent(twTODO->currentIndex());
    twTODO->removeTab(twTODO->currentIndex());
		enableCloseTabButton(-1);	

		if (twTODO->count() == 2)	twTODO->setCurrentIndex(0);
	}	
	if (twTODO->count() == 2){
		enableCloseTabButton(-1);	
    //If we have only two tabs, we can switch focus to package list
    tvPackage->setFocus();
	}
}

void MainWindowImpl::closeAllTabs() {
	while ( twTODO->count() > 2 ){  
    _deallocateTabContent(twTODO->count()-1);
    twTODO->removeTab( twTODO->count()-1 );
		QCoreApplication::processEvents();	
	}

	twTODO->setCurrentIndex(0);
  //As we have only two tabs, we can switch focus to package list
  tvPackage->setFocus();
}

void MainWindowImpl::hideRightView(int saveSettingsReason){
  if ((actionHideRightView->isChecked()) && (splitterVertical->sizes()[2] == 0)) return;

  static QByteArray stateV = splitterVertical->saveState();
	QList<int> l, rl;
  rl = splitterVertical->sizes();

  if ( rl[2] == 0 ){
    splitterVertical->restoreState( stateV );
    if (!actionMaximizeLowerView->isChecked()){
      selectInstalledPackage();
      leFilterInstalledPackages->setFocus();
    }
  }
  else{
    splitterVertical->setSizes( l << dockDirectories->width() << dockPackages->width() << 0 );
    QTreeView *t = twTODO->currentWidget()->findChild<QTreeView*>("tvPkgFileList");

    if (t && splitterHorizontal->sizes().at(1) != 0) t->setFocus();
    else tvPackage->setFocus();
  }

  if (saveSettingsReason == ectn_Save) saveSettings(ectn_FourPanelOrganizing);
}

void MainWindowImpl::normalView(int saveSettingsReason){
  splitterVertical->restoreState(m_verticalSplit);
  splitterHorizontal->restoreState(m_horizontalSplit);
	if (actionHideRightView->isChecked()) hideRightView();

  if (actionExecuteActions->isEnabled()){
    QTreeView *t = twTODO->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
    if (t){
      t->setFocus();
      if (t->currentIndex().isValid()) t->setCurrentIndex(t->currentIndex());
      else t->setCurrentIndex(t->model()->index(0,0));
      if (m_lblStatus->text()=="") showFullPathOfObject(t->currentIndex());
    }
    else if (hasPendingActions()){
      twTODO->setCurrentIndex(0);
      t = twTODO->currentWidget()->findChild<QTreeView*>("tvTODO");
      t->setFocus();
    }
    else tvPackage->setFocus();
  }

  if (saveSettingsReason == ectn_Save) saveSettings(ectn_FourPanelOrganizing);
  tvDir->scrollTo(tvDir->currentIndex(), QAbstractItemView::PositionAtCenter);
}

void MainWindowImpl::minimizeLowerView(int saveSettingsReason){
  m_lblStatus->setText("");
	tvPackage->setFocus();
  if (actionHideRightView->isChecked()) hideRightView();

	QList<int> l, rl;
  rl = splitterHorizontal->sizes();
  if ( rl[1] != 0 )
    splitterHorizontal->setSizes( l << twTODO->height() << 0 );

  if (actionHideRightView->isChecked()) hideRightView();
  if (saveSettingsReason == ectn_Save) saveSettings(ectn_FourPanelOrganizing);
}

void MainWindowImpl::maximizeLowerView(int saveSettingsReason){	  
  QList<int> l, rl;
  rl = splitterHorizontal->sizes();

  if ( rl[0] != 0 ){
    splitterHorizontal->setSizes( l << 0 << twTODO->maximumHeight());
  }

  QTreeView *t = twTODO->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
  if (t){
    t->setFocus();
    if (t->currentIndex().isValid()) t->setCurrentIndex(t->currentIndex());
    else t->setCurrentIndex(t->model()->index(0,0));
    if (m_lblStatus->text()=="") showFullPathOfObject(t->currentIndex());
  }
  else if ((t = twTODO->currentWidget()->findChild<QTreeView*>("tvTODO")))
    t->setFocus();
  else{
    QTextBrowser *b = twTODO->currentWidget()->findChild<QTextBrowser*>();
    if (b)
      b->setFocus();
    else
      twTODO->setFocus();
  }

  if (saveSettingsReason == ectn_Save) saveSettings(ectn_FourPanelOrganizing);
}

void MainWindowImpl::onActionIconifyTriggered(){
  saveSettings(ectn_IconifyOnStart);
}

void MainWindowImpl::showToolBar(){
  saveSettings(ectn_ToolBar);
}

void MainWindowImpl::refreshUIAfterSetup(){
  toolBar->setHidden(!SettingsManager::getShowToolBar());
  statusBar()->setHidden(!SettingsManager::getShowStatusBar());
}

void MainWindowImpl::refreshTreeViews(){
	showPackagesInDirectory();
}

void MainWindowImpl::refreshPackageTreeView(){
  delete m_modelPackage;
  delete m_proxyModelPackage;
  initializePackageTreeView();
}

void MainWindowImpl::refreshInstalledPackageTreeView(){
  delete m_modelInstalledPackages;
  delete m_proxyModelInstalledPackages;

  initializeInstalledPackagesTreeView();
	reapplyInstalledPackagesFilter();
}

void MainWindowImpl::helpQTGZManager(){
  if (this->isHidden()) show();
  conditionalGotoNormalView();

  QString aux(tr("About"));
  QString translated_about = QApplication::translate ( "MainWindow", aux.toUtf8() );
	for (int c=2; c<twTODO->count(); c++)
		if (twTODO->tabText(c) == translated_about){
    twTODO->setCurrentIndex(c);
    QTextBrowser *tb = twTODO->currentWidget()->findChild<QTextBrowser*>("textBrowser");
    if (tb) tb->setFocus();
    return;
  }

	QWidget *tabAbout = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout ( tabAbout );
	gridLayoutX->setSpacing ( 0 );
	gridLayoutX->setMargin ( 0 );

	QTextBrowser *text = new QTextBrowser(tabAbout);
  text->setObjectName("textBrowser");
	text->setReadOnly(true);
	text->setFrameShape(QFrame::NoFrame);
	text->setFrameShadow(QFrame::Plain);
  text->setOpenExternalLinks(true);
	gridLayoutX->addWidget ( text, 0, 0, 1, 1 );

  QString url = "qrc:/resources/help/help_" + QLocale::system().name() + ".html";
  text->setSource(QUrl(url));

  if (text->document()->isEmpty()){
    url = "qrc:/resources/help/help_en_US.html";
    text->setSource(QUrl(url));
  }

  text->show();

  int tindex = twTODO->addTab( tabAbout, QApplication::translate (
      "MainWindow", aux.toUtf8()) );
  twTODO->setTabText(twTODO->indexOf(tabAbout), QApplication::translate(
      "MainWindow", aux.toUtf8()) );

  QWidget *w = m_tabBar->tabButton(tindex, QTabBar::RightSide);
  w->setToolTip(tr("Close tab"));
  w->setObjectName("toolButton");

  SearchBar *searchBar = new SearchBar(this);
  MyHighlighter *highlighter = new MyHighlighter(text, "");

  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChanged(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosed()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNext()));
  connect(searchBar, SIGNAL(findPrevious()), this, SLOT(searchBarFindPrevious()));

  gridLayoutX->addWidget(searchBar, 1, 0, 1, 1);
  gridLayoutX->addWidget(new SyntaxHighlighterWidget(this, highlighter));

  text->show();
  twTODO->setCurrentIndex(tindex);
  text->setFocus();
}

void MainWindowImpl::helpDonate()
{
  const QString url="http://sourceforge.net/donate/index.php?group_id=186459";
  QDesktopServices::openUrl(QUrl(url));
}

void MainWindowImpl::aboutQTGZManager()
{
  QString aboutText = "<b>" + StrConstants::getApplicationName() + QLatin1String("</b><br><br>");
  aboutText += StrConstants::getVersion() + QLatin1String(": ") + StrConstants::getApplicationVersion() +
      QLatin1String(" - ") + StrConstants::getQtVersion() + QLatin1String("<br>");
  aboutText += StrConstants::getURL() + QLatin1String(": ") +
    QStringLiteral("<a href=\"https://qtgzmanager.wordpress.com/\">https://qtgzmanager.wordpress.com</a><br>");
  aboutText += StrConstants::getLicense() + QLatin1String(": ") +
      QStringLiteral("<a href=\"http://www.gnu.org/licenses/gpl-2.0.html\">GPL v2</a><br><br>");
  aboutText += QStringLiteral("&copy; Alexandre A Arnt<br><br>");

  QMessageBox::about(this, StrConstants::getAbout(), aboutText);
}

void MainWindowImpl::openFileOrDirectory(const QModelIndex& mi){
  const QStandardItemModel *sim = qobject_cast<const QStandardItemModel*>(mi.model());
  QStandardItem *si = sim->itemFromIndex(mi);
  QFile f;
  if ((si->icon().pixmap(QSize(22,22)).toImage() ==
       IconHelper::getIconBinary().pixmap(QSize(22,22)).toImage())){
    openFile();
  }
}

void MainWindowImpl::metaFindFile(){
  if (isFindDisabled()) return;

  QTreeView *t = twTODO->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
  if ( (t != 0) && (t->hasFocus() ||
                    (!actionMinimizeLowerView->isChecked() && actionHideRightView->isChecked())) ){
    findFileInPkgFileList();
    return;
  }

  QTextBrowser *tb = twTODO->currentWidget()->findChild<QTextBrowser*>("textBrowser");
  if (!tb) tb = twTODO->currentWidget()->findChild<QTextBrowser*>("updaterOutput");
  SearchBar *searchBar = twTODO->currentWidget()->findChild<SearchBar*>("searchbar");

  if (tb && tb->toPlainText().size() > 0 && (tb->hasFocus() ||
             ((searchBar) && (searchBar->hasFocus())) ||
             ((!actionMinimizeLowerView->isChecked()) && (actionHideRightView->isChecked())))){
    if (searchBar) searchBar->show();
  }

  else findFile();
}

void MainWindowImpl::findFile(){
  if (isFindDisabled()) return;

  toggleToolBarStatus();
  m_findDialog = new FindDialogImpl( this );
  connect(m_findDialog, SIGNAL(installedPkgSelectedInFind(QString)),
          this, SLOT(positionInInstalledPkgList(QString)));

  m_findDialog->setSearchPlace(ectn_INSIDE_INSTALLED_PACKAGES);
  m_findDialog->setFontSize(SettingsManager::getMenuFontSize());
  connect(m_findDialog, SIGNAL(destroyed()), this, SLOT(toggleToolBarStatus()));
  m_findDialog->show();
}

void MainWindowImpl::findFileInPkgFileList(){
/*
  if (isFindDisabled()) return;
  QTreeView *t = twTODO->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
  if (t == 0) return;

  toggleToolBarStatus();
  QStandardItemModel *sim = (QStandardItemModel*) t->model();

  m_findDialog = new FindDialogImpl( this );
  connect(m_findDialog, SIGNAL(fileInsidePkgSelectedInFind(QString,QString)),
          this, SLOT(positionInPkgFileList(QString, QString)));

  QString packagePath = twTODO->currentWidget()->statusTip();
  m_findDialog->setPackagePath(packagePath);
  m_findDialog->setFontSize(SettingsManager::getMenuFontSize());
  QString headerLabel = sim->horizontalHeaderItem(0)->text();
  m_findDialog->setTargetPackage(headerLabel.section('"', 1, 1));
  m_findDialog->setSearchPlace(ectn_INSIDE_QSTDITEMMODEL);
  m_findDialog->setQStandardItemModel(qobject_cast<QStandardItemModel*>(t->model()));

  connect(m_findDialog, SIGNAL(destroyed()), this, SLOT(toggleToolBarStatus()));
  m_findDialog->show();
*/

  QTreeView *tb = twTODO->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
  SearchBar *searchBar = twTODO->currentWidget()->findChild<SearchBar*>("searchbar");

  if (tb && tb->model()->rowCount() > 0 && searchBar)
  {
    if (searchBar)
    {
      searchBar->clear();
      searchBar->show();
    }
  }
}

void MainWindowImpl::findFileInPkgFileListExt(){
  if (isFindDisabled()) return;
  QTreeView *t = twTODO->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
  if (t == 0) return;

  toggleToolBarStatus();
  QStandardItemModel *sim = (QStandardItemModel*) t->model();

  m_findDialog = new FindDialogImpl( this );
  connect(m_findDialog, SIGNAL(fileInsidePkgSelectedInFind(QString,QString)),
          this, SLOT(positionInPkgFileList(QString, QString)));

  QString packagePath = twTODO->currentWidget()->statusTip();
  m_findDialog->setPackagePath(packagePath);
  m_findDialog->setFontSize(SettingsManager::getMenuFontSize());
  QString headerLabel = sim->horizontalHeaderItem(0)->text();
  m_findDialog->setTargetPackage(headerLabel.section('"', 1, 1));
  m_findDialog->setTargetDir(showFullPathOfObject(t->currentIndex()));
  m_findDialog->setSearchPlace(ectn_INSIDE_QSTDITEMMODEL);
  m_findDialog->setQStandardItemModel(qobject_cast<QStandardItemModel*>(t->model()),
                                      sim->itemFromIndex(t->currentIndex()));
  connect(m_findDialog, SIGNAL(destroyed()), this, SLOT(toggleToolBarStatus()));
  m_findDialog->show();
}

void MainWindowImpl::findPackage(){
  if (isFindDisabled()) return;

  toggleToolBarStatus();
  m_findDialog = new FindDialogImpl( this );
  connect(m_findDialog, SIGNAL(packageInsideDirSelectedInFind(QString,QString)),
          this, SLOT(positionInPkgList(QString, QString)));

  m_findDialog->setFontSize(SettingsManager::getMenuFontSize());
  m_findDialog->setTargetDir(dockDirectories->windowTitle());
  m_findDialog->setSearchPlace(ectn_INSIDE_DIRECTORY);
  connect(m_findDialog, SIGNAL(destroyed()), this, SLOT(toggleToolBarStatus()));
  m_findDialog->show();
}

bool MainWindowImpl::isFindDisabled(){
  return !actionFindFile->isEnabled();
}

void MainWindowImpl::toggleToolBarStatus(){
  actionFindPackage->setEnabled(!actionFindPackage->isEnabled());
  actionFindFile->setEnabled(!actionFindFile->isEnabled());
}

void MainWindowImpl::findFileInPackage(){
  if (showPackageContent()) findFileInPkgFileList();
}

void MainWindowImpl::openFile(){
  QString packagePath = twTODO->currentWidget()->statusTip();
  bool isInstalledPackage=true;

  if (packagePath.indexOf(ctn_PACKAGES_DIR) == -1){
    QFileInfo info(packagePath);
    QString package = info.fileName();
    isInstalledPackage = (Package::getStatus(package).getClassification() == ectn_INSTALLED);
  }

  if (isInstalledPackage){
    QFileInfo selectedFile(m_lblStatus->text());
    if (selectedFile.exists())
      WMHelper::openFile(m_lblStatus->text());
    else
      WMHelper::openFile(m_lblStatus->text(), packagePath); //CHANGED!
  }
  else
    WMHelper::openFile(m_lblStatus->text(), packagePath);
}

void MainWindowImpl::editFile(){
  WMHelper::editFile(m_lblStatus->text());
}

//Returns the selected directory according to the panel being focused.
QString MainWindowImpl::getTargetDirectory(){
  QString targetDir;
  QTreeView *t = twTODO->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
  if ((t) && (t->hasFocus()) && (m_lblStatus->text() != "")){
    QFileInfo fi(m_lblStatus->text());
    if (fi.isDir()) targetDir = m_lblStatus->text();
    else targetDir = fi.path();
  }
  else if	(dockDirectories->windowTitle() != "") targetDir = dockDirectories->windowTitle();

  return targetDir;
}

void MainWindowImpl::openDirectory(){
  WMHelper::openDirectory(getTargetDirectory());
}

void MainWindowImpl::openTerminal(){
  WMHelper::openTerminal(getTargetDirectory());
}

void MainWindowImpl::deleteFile(){
  QDir d(m_modelDir->filePath(tvDir->currentIndex()));
	d.setFilter(QDir::Files);		
	QList<SelectedPackage> lmi = getSelectedPackage();		
	int res;

	if (lmi.size() > 1)
		res = QMessageBox::question(this, tr("Confirmation"), 
                                tr("Are you sure you want to delete these files?"),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No);
	else
		res = QMessageBox::question(this, tr("Confirmation"), 
                                tr("Are you sure you want to delete this file?"),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No);

	if (res == QMessageBox::Yes){
    for(SelectedPackage mi: lmi){
      QString fileToDelete = mi.getCompleteFileName();
      QFile::remove(fileToDelete);
      //if there's a signature, delete it too.
      QFile::remove(fileToDelete + ".asc");
		}			
	}
}

void MainWindowImpl::exitApplication(){
  m_reallyWannaClose = true;

  if (m_interfaceInitialized)
    close();
  else
    qApp->quit();;
}

bool MainWindowImpl::close(){ 
  bool res = QWidget::close();

  if (res == true){
    QByteArray windowSize=saveGeometry();
    SettingsManager::setWindowSize(windowSize);
    qApp->exit();
		return( true );
	}
	else return( false );
}

void MainWindowImpl::directoryRenamed(QString path, QString oldName, QString newName){
  if ((m_updaterDirectory == path + QDir::separator() + oldName) && oldName != newName){
    QDir newDir(path);
    newDir.rename(newName, oldName);
  }

  if (newName != oldName){
    dockDirectories->setWindowTitle(path + QDir::separator() + newName);
  }
}

QString MainWindowImpl::showFullPathOfObject( const QModelIndex & index ){
  if (!index.isValid()) return "";

	const QStandardItemModel *sim = qobject_cast<const QStandardItemModel*>( index.model() );	

	QStringList sl;	
	QModelIndex nindex;
	QString str;
	sl << sim->itemFromIndex( index )->text();
	nindex = index;

	while (1){
		nindex = sim->parent( nindex ); 
		if ( nindex != sim->invisibleRootItem()->index() ) sl << sim->itemFromIndex( nindex )->text();
		else break;
	}
	str = QDir::separator() + str;

	for ( int i=sl.count()-1; i>=0; i-- ){
		if ( i < sl.count()-1 ) str += QDir::separator();
		str += sl[i];
	}

  m_lblStatus->setText( str );
  return str;
}

void MainWindowImpl::execContextMenuPkgFileList(QPoint point){
	QTreeView *t = twTODO->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
	if (t == 0) return;

  QFileInfo info(twTODO->currentWidget()->statusTip());
  bool isInstalledPackage = (Package::getStatus(info.fileName()).getClassification() == ectn_INSTALLED);

  QModelIndex mi = t->currentIndex();
	QMenu menu(this);            
	QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(t->model());
	QStandardItem *si = sim->itemFromIndex(mi);
	if (si == 0) return;
  if (si->hasChildren() && (!t->isExpanded(mi))) menu.addAction( actionExpandItem );
  if (si->hasChildren() && (t->isExpanded(mi))) menu.addAction( actionCollapseItem );

	if (menu.actions().count() > 0) menu.addSeparator(); 
	menu.addAction( actionCollapse_All_Items );
	menu.addAction( actionExpand_All_Items );
	menu.addSeparator();

	QDir d;
  QFile f(m_lblStatus->text());
  if ( si->icon().pixmap(QSize(22,22)).toImage() == IconHelper::getIconFolder().pixmap(QSize(22,22)).toImage() ){
    if ( d.exists( m_lblStatus->text() )){
      menu.addAction( actionOpenDirectory );
      menu.addAction( actionOpenTerminal );
    }
    if (sim->hasChildren(mi) && (!isFindDisabled()))
      menu.addAction( actionFindFileInPkgFileList );
	}  
  else if ( (isInstalledPackage && f.exists( m_lblStatus->text())) ||
            (isInstalledPackage && !f.exists( m_lblStatus->text())) ||
            (isInstalledPackage == false) ) {
    menu.addAction ( actionOpenFile );
	}
  if (!UnixCommand::isRootRunning() && isInstalledPackage &&
      f.exists( m_lblStatus->text()) && UnixCommand::isTextFile(m_lblStatus->text()))
    menu.addAction( actionEditFile );

	menu.addSeparator();
	menu.addAction( actionCloseTab );
	if (twTODO->count() > 3) 
		menu.addAction( actionCloseAllTabs );

	QPoint pt2 = t->mapToGlobal(point);
	pt2.setY(pt2.y() + t->header()->height());
	menu.exec(pt2);        	        	
}

void MainWindowImpl::expandAllContentItems(){
	QTreeView *tv =  twTODO->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;		
	if ( tv != 0 ){
		tv->repaint(tv->rect());
 		QCoreApplication::processEvents();
		tv->expandAll();
	}
}

void MainWindowImpl::_expandItem(QTreeView* tv, QStandardItemModel* sim, QModelIndex* mi){
  for (int i=0; i<sim->rowCount(*mi); i++){
    if (sim->hasChildren(*mi)){
      tv->expand(*mi);
      QModelIndex mi2 = sim->index(i, 0, *mi);
      _expandItem(tv, sim, &mi2);
		}
  }
}

void MainWindowImpl::_collapseItem(QTreeView* tv, QStandardItemModel* sim, QModelIndex mi){
	for (int i=0; i<sim->rowCount(mi); i++){
		if (sim->hasChildren(mi)){			
	 		QCoreApplication::processEvents();
			tv->collapse(mi);
      QModelIndex mi2 = sim->index(i, 0, mi);
			_collapseItem(tv, sim, mi2);
		}
	}
}

void MainWindowImpl::expandThisContentItems(){
	QTreeView *tv =  twTODO->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
	if ( tv != 0 ){
		tv->repaint(tv->rect());
    QCoreApplication::processEvents();
		QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tv->model());
		QModelIndex mi = tv->currentIndex();
    if (sim->hasChildren(mi))	_expandItem(tv, sim, &mi);
	} 
}

void MainWindowImpl::collapseThisContentItems(){
	QTreeView *tv =  twTODO->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
	if ( tv != 0 ){
		tv->repaint(tv->rect());
 		QCoreApplication::processEvents();
		QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tv->model());
		QModelIndex mi = tv->currentIndex();
		if (sim->hasChildren(mi))	_collapseItem(tv, sim, mi);
	} 
}

void MainWindowImpl::collapseAllContentItems(){
	QTreeView *tv =  twTODO->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
	if ( tv != 0 ) tv->collapseAll();
}

void  MainWindowImpl::removeAbsoluteDir( QStandardItemModel *im, QModelIndex index ){
	if (index.isValid() == false) return;
	else if ( im->itemFromIndex(index)->hasChildren() ) {
		QStandardItem* si = im->itemFromIndex(index)->child(0, 0);
		removeAbsoluteDir(im, im->index(0, 0, si->index()));		
	}
	else{
		QStandardItem *item = im->itemFromIndex(index);
		QFileInfo fi( item->text() );
		if (fi.isDir()) item->setText( fi.path() );
	}
}

void MainWindowImpl::createDirectory(){
	QModelIndex index = tvDir->currentIndex();
	if (!index.isValid())
		return;

	QString dirName = QInputDialog::getText(this,
                                          tr("Create directory"),
                                          tr("Directory name"));

	if ((dirName != 0) && (dirName.length() > 0)) {
    if (!m_modelDir->mkdir(index, dirName).isValid())
			QMessageBox::information(this, tr("Create directory"),
                               tr("Failed to create the directory"));
	}		
}

void MainWindowImpl::removeDirectory(){
  QModelIndex index = tvDir->currentIndex();

	if (!index.isValid()) return;

	bool ok;
  if (m_modelDir->fileInfo(index).isDir()) {
		int res;
		res = QMessageBox::question(this, tr("Confirmation"), 
                                tr("Are you sure you want to remove this directory?"),
                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
		if (res == QMessageBox::Yes){
      ok = m_modelDir->rmdir(index);
      if (!ok) QMessageBox::information(this, tr("Remove"),
                                        tr("Failed to remove") + " " + (m_modelDir->fileName(index)));
		}
	}	  
}

void MainWindowImpl::changeDefaultDirectory(){
  if (dockDirectories->windowTitle() != ""){
    tvPackage->repaint(tvPackage->rect());
    QCoreApplication::processEvents();

    CPUIntensiveComputing ri;

    m_defaultDirectory = dockDirectories->windowTitle();
    saveSettings(ectn_DefaultDirectory);

    m_updaterDirectory = SettingsManager::getUpdaterDirectory();

    initializeDirTreeView();
  }
}

void MainWindowImpl::showPackagesInDirectory( bool preserveSelected ){  
  static QString bkpDir = "";
  changeDir();
  QList<SelectedPackage> lsp = getSelectedPackage();
  SelectedPackage sp;
  static QString bkpPkg = "";

  if (preserveSelected && lsp.count() > 0){
    sp = lsp.at(0);
    bkpPkg = sp.getFileName();
  }

	QPointer<CPUIntensiveComputing> gRi;	
  if (!m_fsw.directories().isEmpty()) m_fsw.removePaths(m_fsw.directories());
  if (tvDir->currentIndex().isValid()) m_fsw.addPath( m_modelDir->filePath(tvDir->currentIndex() ));
  m_modelPackage->clear();
  QDir d(m_modelDir->filePath(tvDir->currentIndex()));

	QStringList fl;
  QStringList list = d.entryList(fl << ctn_DUMP_FILE + "??_??_????_??_??_??.txt"
                                 << "*" + ctn_TXZ_PACKAGE_EXTENSION
                                 << "*" + ctn_TGZ_PACKAGE_EXTENSION
                                 << "*" + ctn_RPM_PACKAGE_EXTENSION,
                                 QDir::Files, QDir::Name);

	if ( list.count() > 0 )	gRi = new CPUIntensiveComputing();		

  QList<QStandardItem*> items, icons;

  for(QString str: list) {
    Result res = Package::getStatus(str);
		QStandardItem *s;

		switch (res.getClassification()) { 
    case ectn_DUMP_FILE:
            s = new QStandardItem(
          IconHelper::getIconBinary(), "_DUMP");
            icons << s;
            s = new QStandardItem( str );
            items << s; break;

    case ectn_RPM:
			s = new QStandardItem(
          IconHelper::getIconRPM(), "_RPM");
			icons << s;
			s = new QStandardItem( str );	
			items << s; break;

    case ectn_FROZEN :
			s = new QStandardItem(
          IconHelper::getIconFrozen(), "_Frozen");
			icons << s;
			s = new QStandardItem( str );	
			items << s; break;

    case ectn_INTERNAL_ERROR :
			s = new QStandardItem(
          IconHelper::getIconInternalError(), "_Error");
			icons << s;
			s = new QStandardItem( str );	
			items << s; break;

    case ectn_INFERIOR_VERSION :
			s = new QStandardItem(
          IconHelper::getIconInferior(), "_Inferior");
			icons << s;
			s = new QStandardItem( str );    			
			items << s; break;

    case ectn_SUPERIOR_VERSION:
			s = new QStandardItem(
          IconHelper::getIconSuperior(), "_Superior");
			icons << s;
			s = new QStandardItem( str );    						
			items << s; break;

    case ectn_OTHER_VERSION:
			s = new QStandardItem(
          IconHelper::getIconOtherVersion(), "_OtherVersion");
			icons << s;
			s = new QStandardItem( str );    						
			items << s; break;

    case ectn_OTHER_ARCH:
      s = new QStandardItem(
          IconHelper::getIconOtherArch(), "_OtherArch");
      icons << s;
      s = new QStandardItem( str );
      items << s; break;

    case ectn_INSTALLED :
			s = new QStandardItem(
          IconHelper::getIconInstalled(), "_Installed");
			icons << s;
			s = new QStandardItem( str );    						
			items << s; break;

    case ectn_NOT_INSTALLED :
			s = new QStandardItem(
          IconHelper::getIconNotInstalled(), "_Not installed");
			icons << s;
			s = new QStandardItem( str );    						

      s->setForeground(Qt::darkGray);

      items << s; break;

		default: 
			s = new QStandardItem(
          IconHelper::getIconInternalError(), "_Error");
			icons << s;
			s = new QStandardItem( str );    						
			items << s;	    	
		}    	
	}
	
  m_modelPackage->appendColumn(icons);
  m_modelPackage->appendColumn(items);
  m_modelPackage->sort(1);
  m_modelPackage->setHorizontalHeaderLabels(QStringList() << "" << tr("Name"));

	if (items.size()>=1){
		if (items.size()>1) dockPackages->setWindowTitle(tr("%1 Packages in Directory").arg(
				QString::number(items.size())));
		else dockPackages->setWindowTitle(tr("%1 Package in Directory").arg(QString::number(items.size())));  	
	}
  else dockPackages->setWindowTitle(ctn_LABEL_TREEVIEW_PACKAGES);

	tvPackage->setColumnWidth(0, 24);	
	tvPackage->header()->setSortIndicator( m_PackageListOrderedCol, m_PackageListSortOrder );	
	tvPackage->sortByColumn( m_PackageListOrderedCol, m_PackageListSortOrder );
  tvPackage->header()->setSectionsMovable(false);
  tvPackage->header()->setSectionResizeMode( QHeaderView::Fixed );
  if (leFilterPackage->text() != "") reapplyPackageFilter();

  QModelIndex mi;
  if (m_modelPackage->rowCount() > 0) {
    if ((preserveSelected) && (bkpPkg != "") && (bkpDir == getSelectedDirectory())){
      QList<QStandardItem*> lsi =
          m_modelPackage->findItems( bkpPkg, Qt::MatchExactly, ctn_PACKAGE_NAME );
      if (lsi.count() == 1){
        mi = lsi.at(0)->index();
        mi = m_proxyModelPackage->mapFromSource(mi);
      }
      else
        mi = m_proxyModelPackage->index(0, 0);
    }
    else
      mi = m_proxyModelPackage->index(0, 0);

    tvPackage->setCurrentIndex(mi);
		tvPackage->scrollTo(mi);
		tvPackage->setFocus();
  }
  else selectInstalledPackage();

  bkpDir = getSelectedDirectory();
  if (gRi) delete gRi;
}

void MainWindowImpl::selectInstalledPackage(){
  tvInstalledPackages->selectionModel()->clear();

  if (m_modelPackage->rowCount() == 0){
    QModelIndex mi = m_proxyModelInstalledPackages->index(0, 0);
    tvInstalledPackages->scrollTo(mi, QAbstractItemView::PositionAtCenter);
    return;
  }

  QDir d(m_modelDir->filePath(tvDir->currentIndex()));
	d.setFilter(QDir::Files);	
	QModelIndex mi;

	QList<SelectedPackage> lt = getSelectedPackage();
	if (lt.count() == 0){
    QModelIndex mi = m_proxyModelInstalledPackages->index(0, 0);
		tvInstalledPackages->scrollTo(mi, QAbstractItemView::PositionAtCenter);
    return; //Is this right?
	} 
	
  for( SelectedPackage sp: lt ){
    Result res = Package::getStatus(sp.getFileName());
		if (res.getInstalledPackage().size() > 0){
			QList<QStandardItem*> l = 
          m_modelInstalledPackages->findItems( res.getInstalledPackage(), Qt::MatchStartsWith, ctn_PACKAGE_NAME );

			if (l.count() > 0){
				QStandardItem* aux = l[0];

				mi = 	aux->index();
        mi = m_proxyModelInstalledPackages->mapFromSource(mi);
        if (!m_proxyModelInstalledPackages->hasIndex(mi.row(), mi.column())) return;
			}
      else mi = m_proxyModelInstalledPackages->index(0, 0);

      tvInstalledPackages->scrollTo(mi, QAbstractItemView::PositionAtCenter);
      QModelIndex maux = m_proxyModelInstalledPackages->index(mi.row(), 0);
			tvInstalledPackages->selectionModel()->setCurrentIndex(mi, QItemSelectionModel::Select);
			tvInstalledPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);
    }
		else if (res.getInstalledPackage().size() == 0){
      QModelIndex mi = m_proxyModelInstalledPackages->index(0, 0);
      //if (m_modelPackage->rowCount()<=1)
      tvInstalledPackages->scrollTo(mi, QAbstractItemView::PositionAtCenter);
		} 
	}
}

void MainWindowImpl::positionInInstalledPkgList( const QString& pkg ){
  if (actionHideRightView->isChecked()) actionHideRightView->trigger();
  if (!actionMinimizeLowerView->isChecked() && !actionNormalView->isChecked()){
    actionNormalView->setChecked(true);
    normalView();
  }

  tvInstalledPackages->setFocus();
	tvInstalledPackages->selectionModel()->clear();	
  QList<QStandardItem*> l = m_modelInstalledPackages->findItems( pkg, Qt::MatchStartsWith, ctn_PACKAGE_NAME );
	QStandardItem* aux;
	QModelIndex mi;

	if (l.count() > 0){
		aux = l[0];
		mi = 	aux->index();
    mi = m_proxyModelInstalledPackages->mapFromSource(mi);
    if (!m_proxyModelInstalledPackages->hasIndex(mi.row(), mi.column())) return;
	}
  else return;

	tvInstalledPackages->scrollTo(mi, QAbstractItemView::PositionAtCenter);		
  QModelIndex maux = m_proxyModelInstalledPackages->index(mi.row(), 0);
	tvInstalledPackages->selectionModel()->setCurrentIndex(mi, QItemSelectionModel::Select);
	tvInstalledPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);
}

void MainWindowImpl::positionInPkgList( const QString &dir, const QString &pkg ){
  tvPackage->selectionModel()->clear();

  //Let's change the directory first
  if (dir != dockDirectories->windowTitle()){
    QModelIndex index = m_modelDir->setRootPath(dir);
    tvDir->setCurrentIndex(index);
    tvDir->scrollTo(index);

    showPackagesInDirectory(false);
    tvPackage->selectionModel()->clear();
  }

  QList<QStandardItem*> l = m_modelPackage->findItems( pkg, Qt::MatchExactly, ctn_PACKAGE_NAME);
  QStandardItem* aux;
  QModelIndex mi;

  if (l.count() > 0){
    aux = l[0];
    mi = aux->index();
    mi = m_proxyModelPackage->mapFromSource(mi);
    if (!m_proxyModelPackage->hasIndex(mi.row(), mi.column())) return;
  }
  else return;

  tvPackage->setCurrentIndex(mi);
  tvPackage->scrollTo(mi);
  tvPackage->selectionModel()->setCurrentIndex(mi, QItemSelectionModel::Select);

  if (!actionMinimizeLowerView->isChecked() && !actionNormalView->isChecked()){
    actionNormalView->setChecked(true);
    normalView();
  }
}

void MainWindowImpl::positionInPkgFileList( const QString &fileName, const QString &directory ){
  QTreeView *t = twTODO->currentWidget()->findChild<QTreeView*>("tvPkgFileList");

  if ( t != 0){
    conditionalGotoNormalView();
    QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(t->model());
    QList<QStandardItem*> fileList = sim->findItems(fileName, Qt::MatchContains|Qt::MatchRecursive, 0);

    for(QStandardItem *it: fileList){
      if(it->icon().pixmap(QSize(22,22)).toImage() == IconHelper::getIconFolder().pixmap(QSize(22,22)).toImage()) continue;

      QString fullPath = showFullPathOfObject(it->parent()->index());
      if (it->text() == fileName && fullPath == directory){
        t->setCurrentIndex(it->index());
        t->scrollTo(t->currentIndex());
        showFullPathOfObject(it->index());

        break;
      }
    }
  }
}

QList<SelectedPackage> MainWindowImpl::getSelectedPackage(){
	QList<SelectedPackage> lsp;
  QDir d(getSelectedDirectory());
	d.setFilter(QDir::Files);	
	QList<QIcon> licons;
	int c=0;

  for(QModelIndex item: tvPackage->selectionModel()->selectedRows(ctn_PACKAGE_ICON)){
    QModelIndex mi = m_proxyModelPackage->mapToSource(item);
    QStandardItem *si = m_modelPackage->item( mi.row(), ctn_PACKAGE_ICON );
		if (si != 0) 
			licons << si->icon();
	}

  for(QModelIndex item: tvPackage->selectionModel()->selectedRows(ctn_PACKAGE_NAME)){
    QModelIndex mi = m_proxyModelPackage->mapToSource(item);
    QStandardItem *si = m_modelPackage->item( mi.row(), ctn_PACKAGE_NAME );
		if (si != 0){ 
			lsp << SelectedPackage(d.absolutePath(), si->text(), licons[c]);
			c++;
		}
	}

	return lsp;		
}

QList<SelectedPackage> MainWindowImpl::getSelectedInstalledPackage(){
	QList<SelectedPackage> lsp;
  for(QModelIndex item: tvInstalledPackages->selectionModel()->selectedIndexes()){
    if ( item.column() == ctn_PACKAGE_NAME ){
      lsp << SelectedPackage(ctn_PACKAGES_DIR, m_proxyModelInstalledPackages->data(item, 0).toString());
		}
	}

	return lsp;	
}

void MainWindowImpl::showPackageInfo(){
	tvPackage->repaint(tvPackage->rect());
	QCoreApplication::processEvents();	

	QList<SelectedPackage> lsp = getSelectedPackage();
	if (lsp.count() > 1) return;
  for(SelectedPackage sp: lsp){
    if ( sp.getIcon().pixmap(QSize(22,22)).toImage() == IconHelper::getIconRPM().pixmap(QSize(22,22)).toImage() ) continue;

		CPUIntensiveComputing *ri = new CPUIntensiveComputing();
		QString sb = Package::getInformation(sp.getCompleteFileName(), false);
		delete ri;

    if ((sb != 0) && (sb.size() > 0))
      QMessageBox::information(this, tr("Package %1 info").arg(sp.getFileName()), sb);
		else QMessageBox::critical(this, tr("Package %1 info").arg(sp.getFileName()), tr("Sorry, no info provided!"));    
	}
}

void MainWindowImpl::showInstalledPackageInfo(){
	tvInstalledPackages->repaint(tvInstalledPackages->rect());
	QCoreApplication::processEvents();	
	QList<SelectedPackage> lsp = getSelectedInstalledPackage();

  for(SelectedPackage sp: lsp){
		CPUIntensiveComputing *ri = new CPUIntensiveComputing();
		QString sb = Package::getInformation(sp.getCompleteFileName(), true);
		delete ri;

		if ((sb != 0) && (sb.size() > 0)){
			QMessageBox::information(this, tr("Package %1 info").arg(sp.getFileName()), "<html>" + sb + "</html>");
		}
		else QMessageBox::critical(this, tr("Package %1 info").arg(sp.getFileName()), tr("Sorry, no info provided!"));    
	}
}

//Returns true if the package (installed or not) has a content
bool MainWindowImpl::execPackageContent(){
  CPUIntensiveComputing ri;
  QStringList res = m_fw->result();
  QString package = res[0];
  res.removeAt(0);

  if ( !res.isEmpty() )
    createTabPkgFileList(package, res);
  else{
    ri.restoreDefaultCursor();
    while (QApplication::overrideCursor() and QApplication::overrideCursor()->shape()==Qt::WaitCursor)
      ri.restoreDefaultCursor();

    QMessageBox::critical(this, tr("Package %1").arg(package), tr("This package seems corrupted!"));
  }
  return !res.isEmpty();
}

QStringList _showPackageContent(const QString& completeFileName, bool b){
	QStringList sl;
  sl << completeFileName << Package::getContents(completeFileName, b);
	return sl;
}

//Shows the Package (installed or not) list of files and returns true if the package has a content
bool MainWindowImpl::showPackageContent(){
  tvPackage->repaint(tvPackage->rect());
  QCoreApplication::processEvents();
  CPUIntensiveComputing *ri = new CPUIntensiveComputing;
  QFuture<QStringList> f;
	bool alreadyExists=false;
  bool res=false;

	if (tvInstalledPackages->hasFocus()){
    for( SelectedPackage sp: getSelectedInstalledPackage() ){
			alreadyExists = false;

			//First we check if there's not a tab opened with this pkgName contents
      for (int c=2; c<twTODO->count(); c++){
				if (twTODO->tabText(c) == ("&" + sp.getFileName())){
          twTODO->setCurrentIndex ( c );
          alreadyExists = true;
          break;
        }
      }
      if (alreadyExists) continue;

      QString pkg(sp.getCompleteFileName());
      f = run(_showPackageContent, pkg, true);
      m_fw->setFuture(f);
      m_fw->waitForFinished();
      QCoreApplication::processEvents();
      res=execPackageContent();
    }
  }
	else{
    for( SelectedPackage sp: getSelectedPackage() ){
			alreadyExists = false;

      if (Package::getStatus(sp.getFileName()).getClassification() == ectn_RPM){
        transformRPMinTGZ(sp.getCompleteFileName());
        res=true;
        continue;
      }

			//First we check if there's not a tab opened with this pkgName contents
      for (int c=2; c<twTODO->count(); c++){
				if (twTODO->tabText(c) == ("&" + sp.getFileName())){
          twTODO->setCurrentIndex ( c );
          alreadyExists = true;
          break;
        }
      }
      if (alreadyExists) continue;

      if (sp.getIcon().pixmap(QSize(22,22)).toImage() !=
          IconHelper::getIconBinary().pixmap(QSize(22,22)).toImage()){
        QString pkg(sp.getCompleteFileName());
        f = run(_showPackageContent, pkg, false);
        m_fw->setFuture(f);
        m_fw->waitForFinished();
        QCoreApplication::processEvents();
        res=execPackageContent();
      }
      else{
        delete ri;
        ri = 0;
        _openSnapshotOfInstalledPackages(sp.getCompleteFileName());
        res=true;
      }
		}			
	}
  if (alreadyExists) res = true;
  if (res==true) conditionalGotoNormalView();

  if (ri) delete ri;
  return res;
}

void MainWindowImpl::reapplyPackageFilter(){
  bool isFilterPackageSelected = leFilterPackage->hasFocus();
  QString search = Package::parseSearchString(leFilterPackage->text());
  QRegExp regExp(search, Qt::CaseInsensitive, QRegExp::RegExp);

  m_proxyModelPackage->setFilterRegExp(regExp);
  int numPkgs = m_proxyModelPackage->rowCount();

	if (leFilterPackage->text() != ""){
    if (numPkgs > 0) leFilterPackage->setFoundStyle();
    else leFilterPackage->setNotFoundStyle();
	}
	else{
    leFilterPackage->initStyleSheet();;
    showPackagesInDirectory();
	}

  if (numPkgs >= 1)
    dockPackages->setWindowTitle(tr("%1 (%2) Packages in Directory").arg(QString::number(numPkgs)).
        arg(QString::number(tvPackage->selectionModel()->selectedRows().count())));
  /*else if (numPkgs == 1)
    dockPackages->setWindowTitle(tr("1 Package in Directory"));*/
	else
		dockPackages->setWindowTitle(tr("0 Packages in Directory"));

	tvPackage->selectionModel()->clear();
  QModelIndex mi = m_proxyModelPackage->index(0, 0);
	tvPackage->setCurrentIndex(mi);
	tvPackage->scrollTo(mi);

  selectInstalledPackage();

  if (isFilterPackageSelected) leFilterPackage->setFocus();
  m_proxyModelPackage->sort(m_PackageListOrderedCol, m_PackageListSortOrder);
}

void MainWindowImpl::reapplyInstalledPackagesFilter(){
  QString search = Package::parseSearchString(leFilterInstalledPackages->text());

  //Do we have an empty search string?
  if (search == "\\S*$"){
    delete m_modelInstalledPackages;
    delete m_proxyModelInstalledPackages;
    initializeInstalledPackagesTreeView();

    disconnect(tvInstalledPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(tvInstalledPackagesSelectionChanged(QItemSelection,QItemSelection)));

    connect(tvInstalledPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(tvInstalledPackagesSelectionChanged(QItemSelection,QItemSelection)));
  }

  QRegExp regExp(search, Qt::CaseInsensitive, QRegExp::RegExp);
  m_proxyModelInstalledPackages->setFilterRegExp(regExp);
  int numInstPkgs = m_proxyModelInstalledPackages->rowCount();

	if (leFilterInstalledPackages->text() != ""){
    if (numInstPkgs > 0) leFilterInstalledPackages->setFoundStyle();
    else leFilterInstalledPackages->setNotFoundStyle();
	}
	else{
    leFilterInstalledPackages->initStyleSheet();
	}

  if (numInstPkgs >= 1)
    dockInstalledPackages->setWindowTitle(tr("%1 (%2) Packages Installed").arg(QString::number(numInstPkgs)).
        arg(QString::number(tvInstalledPackages->selectionModel()->selectedRows().count())));
  /*else if (numInstPkgs == 1)
    dockInstalledPackages->setWindowTitle(tr("1 Package Installed"));*/
	else
		dockInstalledPackages->setWindowTitle(tr("No Installed Package found!"));

  m_proxyModelInstalledPackages->sort(m_InstalledPackageListOrderedCol, m_InstalledPackageListSortOrder);
  selectInstalledPackage();
}

void MainWindowImpl::clearFilterPackage(){
  leFilterPackage->clear();
  leFilterPackage->setFocus();
}

void MainWindowImpl::clearFilterInstalledPackage(){
  leFilterInstalledPackages->clear();
  leFilterInstalledPackages->setFocus();
}
