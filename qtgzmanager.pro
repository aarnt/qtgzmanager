TEMPLATE = app

QT += gui \
    widgets \
    core \
    network

CONFIG += qt \
   debug \
   console \
   warn_on

INCLUDEPATH += src
DESTDIR += bin
OBJECTS_DIR += build
MOC_DIR += build
UI_DIR += build

HEADERS += src/mainwindowimpl.h \
    src/finddialogimpl.h \
    src/package.h \
    src/qdnddirmodel.h \
    src/qdndstandarditemmodel.h \
    src/simplestatusbar.h \
    src/tvpackagesitemdelegate.h \
    src/unixcommand.h \
    src/argumentlist.h \
    src/settingsmanager.h \
    src/updater.h \
    src/updaterthread.h \
    src/setupdialog.h \
    src/strconstants.h \
    src/tabwidget.h \
    src/searchlineedit.h \
    src/searchbar.h \
    src/QtSolutions/qtsingleapplication.h \
    src/QtSolutions/qtlocalpeer.h \
    src/packagecontroller.h \
    src/uihelper.h \
    src/wmhelper.h

SOURCES += src/main.cpp \
    src/finddialogimpl.cpp \
    src/package.cpp \
    src/qdnddirmodel.cpp \
    src/qdndstandarditemmodel.cpp \
    src/simplestatusbar.cpp \
    src/tvpackagesitemdelegate.cpp \
    src/unixcommand.cpp \
    src/argumentlist.cpp \
    src/settingsmanager.cpp \
    src/mainwindowimpl.cpp \
    src/mainwindowimpl_events.cpp \
    src/mainwindowimpl_packageactions.cpp \
    src/mainwindowimpl_initialization.cpp \
    src/mainwindowimpl_updater.cpp \
    src/mainwindowimpl_searchbar.cpp \
    src/updater.cpp \
    src/updaterthread.cpp \
    src/setupdialog.cpp \
    src/tabwidget.cpp \
    src/searchlineedit.cpp \
    src/searchbar.cpp \
    src/QtSolutions/qtsingleapplication.cpp \
    src/QtSolutions/qtlocalpeer.cpp \
    src/packagecontroller.cpp \
    src/wmhelper.cpp

FORMS += ui/mainwindow.ui \
    ui/finddialog.ui \
    ui/setupdialog.ui

RESOURCES += \
    resources.qrc

TRANSLATIONS += resources/translations/qtgzmanager_pt_BR.ts \
    resources/translations/qtgzmanager_it_IT.ts \
    resources/translations/qtgzmanager_de_DE.ts \
    resources/translations/qtgzmanager_pl_PL.ts \
    resources/translations/qtgzmanager_fa_IR.ts \
    resources/translations/qtgzmanager_el_GR.ts

# these seem to be needed for building without linking problems on multilib...
#contains(QT_ARCH, x86_64): {
#    QMAKE_PREFIX_SHLIB = lib64
#    QMAKE_PREFIX_STATICLIB = lib64
#    QMAKE_LIBDIR_OPENGL_ES1 = $$QMAKE_LIBDIR_OPENGL
#    QMAKE_LIBDIR_OPENGL_ES2 = $$QMAKE_LIBDIR_OPENGL
#}
