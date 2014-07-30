DEFINES += ECLIBRUS_VERSION="\\\"1.3\\\""

HEADERS += mainwindow.h \
    settings.h \
    eclibwebview.h \
    browsertab.h \
    librarynam.h \
    eclibreply.h \
    eclibrenderers.h \
    aboutdialog.h \
    preferencesdialog.h \
    devices.h \
    devicesconfigdialog.h \
    addnewdevicedialog.h \
    managebooksondevicedialog.h \
    mountnotifier.h \
    exportbooksthread.h \
    exportbooksprogress.h \
    db.h

SOURCES += main.cpp \
    mainwindow.cpp \
    settings.cpp \
    eclibwebview.cpp \
    browsertab.cpp \
    librarynam.cpp \
    eclibreply.cpp \
    eclibrenderers.cpp \
    aboutdialog.cpp \
    preferencesdialog.cpp \
    devices.cpp \
    devicesconfigdialog.cpp \
    addnewdevicedialog.cpp \
    managebooksondevicedialog.cpp \
    mountnotifier.cpp \
    exportbooksthread.cpp \
    exportbooksprogress.cpp \
    db.cpp

FORMS += aboutdialog.ui \
    devicesconfigdialog.ui \
    addnewdevicedialog.ui \
    managebooksondevicedialog.ui \
    preferencesdialog.ui \
    exportbooksprogress.ui 

CODECFORTR = UTF-8
TRANSLATIONS = ../translations/eclibrus_ru.ts

QT += webkit widgets webkitwidgets xml sql network

CONFIG(release, release|debug) {
    message(RELEASE)
    DEFINES += QT_NO_DEBUG_OUTPUT
}


POST_TARGETDEPS += ../fb2/libfb2.a
INCLUDEPATH += ../
RESOURCES = ../resources/application.qrc
TARGET = eclibrus
DESTDIR = ../
LIBS += -L../fb2 -lfb2 -L../quazip -lquazip -lz

linux-g++-32 {
    LIBS += -lblkid
}

linux-g++-64 {
    LIBS += -lblkid
}

macx {
    LIBS += -L/usr/local/lib
    INCLUDEPATH += /usr/local/include
    ICON = ../resources/icons/eclibrus.icns
}

unix {
    #VARIABLES
    isEmpty(PREFIX) {
    PREFIX = /usr
    }

    BINDIR = $$PREFIX/bin
    DATADIR =$$PREFIX/share

    DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"

    target.path = $$BINDIR

    desktop.path = $$DATADIR/applications
    desktop.files += ../eclibrus.desktop

    icon16.path = $$DATADIR/icons/hicolor/16x16/apps/
    icon16.files = ../resources/icons/16x16/Eclibrus.png

    icon32.path = $$DATADIR/icons/hicolor/32x32/apps/
    icon32.files = ../resources/icons/32x32/Eclibrus.png

    icon48.path = $$DATADIR/icons/hicolor/48x48/apps/
    icon48.files = ../resources/icons/48x48/Eclibrus.png

    icon64.path = $$DATADIR/icons/hicolor/64x64/apps/
    icon64.files = ../resources/icons/64x64/Eclibrus.png

    INSTALLS += target desktop icon16 icon32 icon48


}
