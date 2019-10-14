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
    json/json_reader.cpp \
    json/json_value.cpp \
    json/json_writer.cpp \
    src/util.cpp \
    src/wsutil.cpp \
    src/tmutil.cpp \
    src/fsutil.cpp \
    src/Path.cpp \
    src/mtutil.cpp \
    src/TxtWriter.cpp \
    src/zipDecompress.cpp \
    src/SQLiteDB.cpp \
    src/jsonutil.cpp \
    ../../zlib-1.2.11/contrib/minizip/unzip.c \
    ../../zlib-1.2.11/contrib/minizip/ioapi.c \

HEADERS +=\
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
    ../../inc/util/util.h \
    ../../inc/util/wsutil.h \
    ../../inc/util/tmutil.h \
    ../../inc/util/mtlock.h \
    ../../inc/util/mtutil.h \
    ../../inc/util/fsutil.h \
    ../../inc/util/Path.h \
    ../../inc/util/TxtWriter.h \
    ../../inc/util/IDB.h \
    ../../inc/util/SQLiteDB.h \
    ../../inc/util/jsonutil.h

win32 {
SOURCES += \
    src/wintimer.cpp \
    src/winfsutil.cpp \
    src/winfsdlg.cpp

HEADERS +=\
    ../../inc/util/winfsutil.h \
    ../../inc/util/wintimer.h \
    ../../inc/util/winfsdlg.h
}

DEFINES += __UtilPrj IOAPI_NO_64

INCLUDEPATH += ../../inc/util \
    ../../zlib-1.2.11

win32 {
    LIBS += -lcomdlg32 -lgdi32
    LIBS += ../../bin/zlib1.dll

    platform = win
    DESTDIR = ../../bin

    target.path = ../../../XMusic/bin
    INSTALLS += target
} else {
    LIBS    += -lz

    android {
        platform = android
        DESTDIR = ../../../XMusic/libs/armeabi-v7a
    } else: macx {
        platform = mac
        DESTDIR = ../../bin/mac

        target.path = ../../../XMusic/bin/mac
        INSTALLS += target
    } else: ios {
        platform = ios
        DESTDIR = ../../../build/ioslib
    }
}

build_dir = ../../../build/xutil/$$platform

MOC_DIR = $$build_dir
RCC_DIR = $$build_dir
UI_DIR = $$build_dir
OBJECTS_DIR = $$build_dir
