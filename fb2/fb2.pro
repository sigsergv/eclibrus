TEMPLATE = lib

QT += xml
CONFIG += staticlib

HEADERS += parser.h \
    fb2.h

SOURCES += parser.cpp \
    fb2.cpp

CONFIG(release, release|debug) {
    message(RELEASE)
    DEFINES += QT_NO_DEBUG_OUTPUT
}

INCLUDEPATH += ../
LIBS += -L../quazip -lquazip
