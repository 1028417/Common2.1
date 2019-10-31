#-------------------------------------------------
#
# Project created by QtCreator 2019-03-27T06:30:38
#
#-------------------------------------------------

QT       += core

TARGET = xutil
TEMPLATE = lib

QMAKE_CXXFLAGS += -std=c++11 #c++1y #gnu++1y

HEADERS += \
    ../../inc/util/util.h \
    ../../inc/util/tmutil.h \
    ../../inc/util/buffer.h \
    ../../inc/util/strutil.h \
    ../../inc/util/timerutil.h \
    ../../inc/util/mtlock.h \
    ../../inc/util/mtutil.h \
    ../../inc/util/fsutil.h \
    ../../inc/util/fstream.h \
    ../../inc/util/TxtWriter.h \
    ../../inc/util/Path.h \
    ../../inc/util/winfsdlg.h \
    ../../inc/util/winfsutil.h \
    ../../inc/util/IDB.h \
    ../../inc/util/SQLiteDB.h \
    ../../inc/util/jsonutil.h \
    ../../inc/util/xmlutil.h \
#
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
    ../../inc/util/zutil.h

SOURCES += \
    cpp/util.cpp \
    cpp/tmutil.cpp \
    cpp/strutil.cpp \
    cpp/timerutil.cpp \
    cpp/mtutil.cpp \
    cpp/fsutil.cpp \
    cpp/Path.cpp \
    cpp/TxtWriter.cpp \
#
    cpp/jsonutil.cpp \
    ../../3rd/json/json_reader.cpp \
    ../../3rd/json/json_value.cpp \
    ../../3rd/json/json_writer.cpp \
#
    cpp/xmlutil.cpp \
    ../../3rd/tinyxml/tinyxml.cpp \
    ../../3rd/tinyxml/tinyxmlerror.cpp \
    ../../3rd/tinyxml/tinyxmlparser.cpp \
    #../../3rd/tinyxml/tinystr.cpp \
#
    cpp/SQLiteDB.cpp \
    ../../3rd/sqlite/shell.c \
    ../../3rd/sqlite/sqlite3.c \
#
    ../../3rd/zlib-1.2.11/contrib/minizip/unzip.c \
    ../../3rd/zlib-1.2.11/contrib/minizip/ioapi.c \
    cpp/zutil.cpp

win32: SOURCES += cpp/winfsutil.cpp  cpp/winfsdlg.cpp

INCLUDEPATH += ../../inc/util \
    ../../3rd/zlib-1.2.11

DEFINES += __UtilPrj  IOAPI_NO_64  TIXML_USE_STL

win32 {
    LIBS += -lgdi32  -lcomdlg32  -lole32 \
        ../../bin/zlib1.dll

    platform = win
    DESTDIR = ../../bin

    target.path = ../../../XMusic/bin
    INSTALLS += target
} else {
    LIBS += -lz

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
