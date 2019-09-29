#-------------------------------------------------
#
# Project created by QtCreator 2019-03-27T06:30:38
#
#-------------------------------------------------

QT       -= core

TARGET = xutil
TEMPLATE = lib

QMAKE_CXXFLAGS += -std=c++1y #gnu++1y

SOURCES += \
    sqlite/shell.c \
    sqlite/sqlite3.c \
    src/wsutil.cpp \
    src/tmutil.cpp \
    src/fsutil.cpp \
    src/Path.cpp \
    src/SQLiteDB.cpp \
    src/winfsutil.cpp \
    src/wintimer.cpp \
    src/mtutil.cpp \
    src/TxtWriter.cpp \
    json/json_reader.cpp \
    json/json_value.cpp \
    json/json_writer.cpp \
    src/util.cpp

HEADERS +=\
    ../../inc/util/fsutil.h \
    ../../inc/util/IDB.h \
    ../../inc/util/mtutil.h \
    ../../inc/util/Path.h \
    ../../inc/util/SQLiteDB.h \
    ../../inc/util/util.h \
    ../../inc/util/winfsutil.h \
    ../../inc/util/wintimer.h \
    ../../inc/util/TxtWriter.h \
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
    ../../inc/util/wsutil.h \
    ../../inc/util/mtlock.h

DEFINES += __UtilPrj

INCLUDEPATH += ../../inc/util

win32 {
    LIBS += ../../bin/zlib1.dll

    DESTDIR = ../../bin

    target.path = ../../../XMusic/bin
    INSTALLS += target
} else {
    LIBS    += -lz

    android {
        DESTDIR = ../../../XMusic/libs/armeabi-v7a
    } else: macx {
        DESTDIR = ../../bin/mac

        target.path = ../../../XMusic/bin/mac
        INSTALLS += target
    } else: ios {
        DESTDIR = ../../../build/ioslib
    }
}

MOC_DIR = ../../../build/xutil
RCC_DIR = ../../../build/xutil
UI_DIR = ../../../build/xutil
OBJECTS_DIR = ../../../build/xutil
