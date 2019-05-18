#-------------------------------------------------
#
# Project created by QtCreator 2019-03-27T06:30:38
#
#-------------------------------------------------

QT       -= core
#android: QT += androidextras

TARGET = util
TEMPLATE = lib

SOURCES += \
    sqlite/shell.c \
    sqlite/sqlite3.c \
    wsutil.cpp \
    tmutil.cpp \
    fsutil.cpp \
    Path.cpp \
    SQLiteDB.cpp \
    winfsutil.cpp \
    wintimer.cpp \
    mtutil.cpp

HEADERS +=\
    sqlite/sqlite3.h \
    sqlite/sqlite3ext.h \
    ../../inc/util/fsdlg.h \
    ../../inc/util/fsutil.h \
    ../../inc/util/IDB.h \
    ../../inc/util/mtutil.h \
    ../../inc/util/Path.h \
    ../../inc/util/SQLiteDB.h \
    ../../inc/util/util.h \
    ../../inc/util/winfsutil.h \
    ../../inc/util/wintimer.h \
    ../../inc/sstl/_check.h \
    ../../inc/sstl/_define.h \
    ../../inc/sstl/_util.h \
    ../../inc/sstl/ArrList.h \
    ../../inc/sstl/PairList.h \
    ../../inc/sstl/PtrArray.h \
    ../../inc/sstl/ptrcontainer.h \
    ../../inc/sstl/SArray.h \
    ../../inc/sstl/SContainer.h \
    ../../inc/sstl/SList.h \
    ../../inc/sstl/SMap.h \
    ../../inc/sstl/SSet.h \
    ../../inc/sstl/sstl.h \
    ../../inc/util/tmutil.h \
    ../../inc/util/wsutil.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DEFINES += __UtilPrj

INCLUDEPATH += \
    ../../inc \
    ../../inc/util

DESTDIR = $$PWD/../../../Player3.1/2Player

CONFIG += c++11
