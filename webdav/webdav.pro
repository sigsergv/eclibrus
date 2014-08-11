TEMPLATE = lib

QT += network
CONFIG += staticlib

TARGET = webdav
#INCLUDEPATH += .

HEADERS += qwebdav.h eventloop.h propfindparser.h qwebdav_types.h
SOURCES += qwebdav.cpp eventloop.cpp propfindparser.cpp

CONFIG(release, release|debug) {
    message(RELEASE)
    DEFINES += QT_NO_DEBUG_OUTPUT
}
