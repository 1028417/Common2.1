#-------------------------------------------------
#
# Project created by QtCreator 2019-03-27T06:30:38
#
#-------------------------------------------------

QT       -= gui

TARGET = xutil
TEMPLATE = lib

QMAKE_CXXFLAGS += -std=c++11 #c++1y #gnu++1y

HEADERS += \
    ../../inc/util/curlutil.h \
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
    ../../inc/util/ziputil.h

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
    ../../3rd/zlib-1.2.11/adler32.c \
    ../../3rd/zlib-1.2.11/compress.c \
    ../../3rd/zlib-1.2.11/crc32.c \
    ../../3rd/zlib-1.2.11/deflate.c \
    ../../3rd/zlib-1.2.11/gzclose.c \
    ../../3rd/zlib-1.2.11/gzlib.c \
    ../../3rd/zlib-1.2.11/gzread.c \
    ../../3rd/zlib-1.2.11/gzwrite.c \
    ../../3rd/zlib-1.2.11/infback.c \
    ../../3rd/zlib-1.2.11/inffast.c \
    ../../3rd/zlib-1.2.11/inflate.c \
    ../../3rd/zlib-1.2.11/inftrees.c \
    ../../3rd/zlib-1.2.11/trees.c \
    ../../3rd/zlib-1.2.11/uncompr.c \
    ../../3rd/zlib-1.2.11/zutil.c \
    cpp/ziputil.cpp \
#
    ../../3rd/bzip2-1.0.6/blocksort.c \
    ../../3rd/bzip2-1.0.6/bzcompress.c \
    ../../3rd/bzip2-1.0.6/bzip2.c \
    ../../3rd/bzip2-1.0.6/bzlib.c \
    ../../3rd/bzip2-1.0.6/crctable.c \
    ../../3rd/bzip2-1.0.6/decompress.c \
    ../../3rd/bzip2-1.0.6/huffman.c \
    ../../3rd/bzip2-1.0.6/randtable.c \
#
    cpp/curlutil.cpp \
    ../../3rd/curl/lib/curl_ctype.c \
    ../../3rd/curl/lib/nonblock.c \
    ../../3rd/curl/lib/strtoofft.c \
    ../../3rd/curl/lib/warnless.c \
    ../../3rd/curl/src/slist_wc.c \
    ../../3rd/curl/src/tool_binmode.c \
    ../../3rd/curl/src/tool_bname.c \
    ../../3rd/curl/src/tool_cb_dbg.c \
    ../../3rd/curl/src/tool_cb_hdr.c \
    ../../3rd/curl/src/tool_cb_prg.c \
    ../../3rd/curl/src/tool_cb_rea.c \
    ../../3rd/curl/src/tool_cb_see.c \
    ../../3rd/curl/src/tool_cb_wrt.c \
    ../../3rd/curl/src/tool_cfgable.c \
    ../../3rd/curl/src/tool_convert.c \
    ../../3rd/curl/src/tool_dirhie.c \
    ../../3rd/curl/src/tool_doswin.c \
    ../../3rd/curl/src/tool_easysrc.c \
    ../../3rd/curl/src/tool_filetime.c \
    ../../3rd/curl/src/tool_formparse.c \
    ../../3rd/curl/src/tool_getparam.c \
    ../../3rd/curl/src/tool_getpass.c \
    ../../3rd/curl/src/tool_help.c \
    ../../3rd/curl/src/tool_helpers.c \
    ../../3rd/curl/src/tool_homedir.c \
    ../../3rd/curl/src/tool_hugehelp.c \
    ../../3rd/curl/src/tool_libinfo.c \
    ../../3rd/curl/src/tool_main.c \
    ../../3rd/curl/src/tool_metalink.c \
    ../../3rd/curl/src/tool_msgs.c \
    ../../3rd/curl/src/tool_operate.c \
    ../../3rd/curl/src/tool_operhlp.c \
    ../../3rd/curl/src/tool_panykey.c \
    ../../3rd/curl/src/tool_paramhlp.c \
    ../../3rd/curl/src/tool_parsecfg.c \
    ../../3rd/curl/src/tool_progress.c \
    ../../3rd/curl/src/tool_setopt.c \
    ../../3rd/curl/src/tool_sleep.c \
    ../../3rd/curl/src/tool_strdup.c \
    ../../3rd/curl/src/tool_urlglob.c \
    ../../3rd/curl/src/tool_util.c \
    ../../3rd/curl/src/tool_vms.c \
    ../../3rd/curl/src/tool_writeout.c \
    ../../3rd/curl/src/tool_xattr.c

win32: SOURCES += cpp/winfsutil.cpp  cpp/winfsdlg.cpp

INCLUDEPATH += ../../inc/util \
    ../../3rd/zlib-1.2.11 \
    ../../3rd/bzip2-1.0.6 \
    ../../3rd/curl/include ../../3rd/curl/lib ../../3rd/curl/src

DEFINES += __UtilPrj  IOAPI_NO_64  TIXML_USE_STL

win32 {
    LIBS += -lgdi32  -lcomdlg32  -lole32
    LIBS += $$PWD/../../bin/libcurl.dll  -lws2_32

    platform = win
    DESTDIR = ../../bin

    QMAKE_POST_LINK += copy /Y ..\..\bin\xutil.dll ..\..\..\XMusic\bin
} else {
    DEFINES += HAVE_CONFIG_H

android {
    LIBS += -L$$PWD/../../libs/armeabi-v7a  -lcurl

    platform = android
    DESTDIR = ../../../XMusic/libs/armeabi-v7a
} else: macx {
    LIBS += -L$$PWD/../../libs/ios/simulator  -lcurl  -lssl  -lcrypto  -lnghttp2  -lz

    platform = mac
    DESTDIR = ../../bin/mac

    QMAKE_POST_LINK += cp -f ../../bin/mac/libxutil*.dylib ../../../XMusic/bin/mac/
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
