#-------------------------------------------------
#
# Project created by QtCreator 2019-03-27T06:30:38
#
#-------------------------------------------------

QT       -= gui

TARGET = xutil
TEMPLATE = lib

QMAKE_CXXFLAGS += -std=c++11  #c++1y  #gnu++1y

DEFINES += QT_DEPRECATED_WARNINGS  __UtilPrj

DEFINES += TIXML_USE_STL

DEFINES += HAVE_BZIP2

!win32 {
    DEFINES += IOAPI_NO_64 #for zlib

    #安卓无效，只能用lseek64；ios、mac也不需要，因为long是64位的
    #DEFINES += _FILE_OFFSET_BITS=64 #会导致<fstream>编译报错
    #gnu的ftello等系列函数用宏-D_FILE_OFFSET_BITS 64指定off_t，用_LARGEFILE_SOURCE来开启ftello64
    #DEFINES += _LARGEFILE_SOURCE  _LARGEFILE64_SOURCE
    #？？DEFINES += __USE_FILE_OFFSET64  __USE_LARGEFILE64  _LARGEFILE64_SOURCE
}

DEFINES += BUILDING_LIBCURL  USE_OPENSSL  HAVE_OPENSSL \
            CARES_BUILDING_LIBRARY #USE_ARES #安卓有问题  #USE_IPV6

DEFINES += HAVE_UNISTD_H #for curl7.76

#DEFINES += JSON_DLL_BUILD #告警太多
win32 {
DEFINES += JSON_API=__declspec(dllexport)
} else {
DEFINES += JSON_API=__attribute__((visibility("default")))
}

zlib_dir = ../../3rd/zlib-1.2.11
bzip2_dir = ../../3rd/bzip2-1.0.6

curl_dir = ../../3rd/curl-7.67.0
cares_dir = ../../3rd/c-ares-1.15.0

#json_dir = ../../3rd/jsoncpp-1.8.0
json_dir = ../../3rd/jsoncpp-1.9.4

tinyxml_dir = ../../3rd/tinyxml-2.6.2

sqlite_dir = ../../3rd/sqlite-3.35.5 # ../../3rd/sqlite-3.28.0

INCLUDEPATH += ../../inc/util \
    $$zlib_dir $$bzip2_dir \
#
    $$curl_dir/include $$curl_dir/lib $$curl_dir/src \
    $$cares_dir \
#
    $$tinyxml_dir \
#
    $$sqlite_dir

android {
INCLUDEPATH += ../../3rd/openssl-1.1.0f/include
} else {
INCLUDEPATH += ../../3rd/openssl-1.1.0h/include
}

mac {
XMusicDir = ../../../XMusic
} else {
XMusicDir = ..\..\..\XMusic
}

win32 {
    LIBS += -lgdi32  -lcomdlg32  -lole32

    LIBS += -lws2_32 -lwldap32 \
            $$PWD/../../3rd/openssl-1.1.0h/lib/libcrypto.lib \
            $$PWD/../../3rd/openssl-1.1.0h/lib/libssl.lib

    platform = win
    DESTDIR = ..\..\bin

    QMAKE_POST_LINK += copy /Y $$DESTDIR\xutil.dll $$XMusicDir\bin && \
        copy /Y $$DESTDIR\libxutil.a $$XMusicDir\bin
} else {
    DEFINES += HAVE_CONFIG_H

    LIBS += -lcrypto -lssl

android {
    LIBS += -L../../libs/armeabi-v7a

    INCLUDEPATH += $$curl_dir/lib/curl_config_android

    platform = android
    DESTDIR = ..\..\libs\armeabi-v7a
    #QMAKE_POST_LINK += copy /Y $$DESTDIR\libxutil.so $$XMusicDir\libs\armeabi-v7a
} else: macx {
    LIBS += -L../../libs/mac  #-lnghttp2  -lz

    INCLUDEPATH += $$curl_dir/lib/curl_config_mac

    platform = mac
    DESTDIR = ../../bin/mac

    QMAKE_POST_LINK += cp -f $$DESTDIR/libxutil*.dylib $$XMusicDir/bin/mac/
} else: ios {
    INCLUDEPATH += $$curl_dir/lib/curl_config_mac

    platform = ios
    DESTDIR = ../../../build/ioslib
}
}

#CONFIG += debug_and_release
CONFIG(debug, debug|release) {
BuildDir = xutild
} else {
BuildDir = xutil
}
BuildDir = ../../../build/$$BuildDir/$$platform

MOC_DIR = $$BuildDir
RCC_DIR = $$BuildDir
UI_DIR = $$BuildDir
OBJECTS_DIR = $$BuildDir


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
    ../../inc/sstl/*.h \
    ../../inc/util/ziputil.h

win32: SOURCES += cpp/winfsutil.cpp  cpp/winfsdlg.cpp

SOURCES += \
    cpp/util.cpp \
    cpp/tmutil.cpp \
    cpp/strutil.cpp \
    cpp/timerutil.cpp \
    cpp/ThreadGroup.cpp \
    cpp/fsutil.cpp \
    cpp/Path.cpp \
    cpp/TxtWriter.cpp \
#
    cpp/jsonutil.cpp \
    $$json_dir/src/lib_json/json_reader.cpp \ #    $$json_dir/json_reader.cpp \
    $$json_dir/src/lib_json/json_value.cpp \ #    $$json_dir/json_value.cpp \
    $$json_dir/src/lib_json/json_writer.cpp \ #    $$json_dir/json_writer.cpp \
#
    cpp/xmlutil.cpp \
    $$tinyxml_dir/tinyxml.cpp \
    $$tinyxml_dir/tinyxmlerror.cpp \
    $$tinyxml_dir/tinyxmlparser.cpp \
    #$$tinyxml_dir/tinystr.cpp \
#
    cpp/SQLiteDB.cpp \
    $$sqlite_dir/shell.c \
    $$sqlite_dir/sqlite3.c \
#
    cpp/ziputil.cpp \
    $$zlib_dir/contrib/minizip/zip.c \
    $$zlib_dir/contrib/minizip/unzip.c \
    $$zlib_dir/contrib/minizip/ioapi.c \
    $$zlib_dir/*.c \
#
    $$bzip2_dir/blocksort.c \
    $$bzip2_dir/bzcompress.c \
    $$bzip2_dir/bzip2.c \
    $$bzip2_dir/bzlib.c \
    $$bzip2_dir/crctable.c \
    $$bzip2_dir/decompress.c \
    $$bzip2_dir/huffman.c \
    $$bzip2_dir/randtable.c \
#
    cpp/curlutil.cpp \
    $$curl_dir/lib/vauth/*.c \
    $$curl_dir/lib/vtls/*.c \
    $$curl_dir/lib/vquic/*.c \
    $$curl_dir/lib/vssh/*.c \
    $$curl_dir/lib/*.c \
    $$curl_dir/src/*.c

macx {} else: ios {} else {
SOURCES += \
    $$cares_dir/ares__close_sockets.c \
    $$cares_dir/ares__get_hostent.c \
    $$cares_dir/ares__read_line.c \
    $$cares_dir/ares__timeval.c \
    $$cares_dir/ares_cancel.c \
    $$cares_dir/ares_create_query.c \
    $$cares_dir/ares_data.c \
    $$cares_dir/ares_destroy.c \
    $$cares_dir/ares_expand_name.c \
    $$cares_dir/ares_free_hostent.c \
    $$cares_dir/ares_free_string.c \
    $$cares_dir/ares_gethostbyname.c \
    $$cares_dir/ares_getsock.c \
    $$cares_dir/ares_init.c \
    $$cares_dir/ares_library_init.c\
    $$cares_dir/ares_llist.c \
    $$cares_dir/ares_nowarn.c \
    $$cares_dir/ares_options.c \
    $$cares_dir/ares_parse_a_reply.c \
    $$cares_dir/ares_parse_aaaa_reply.c \
    $$cares_dir/ares_process.c \
    $$cares_dir/ares_query.c \
    $$cares_dir/ares_search.c \
    $$cares_dir/ares_send.c \
    $$cares_dir/ares_strerror.c \
    $$cares_dir/ares_timeout.c \
    $$cares_dir/ares_version.c \
    $$cares_dir/bitncmp.c \
    $$cares_dir/inet_net_pton.c \
    $$cares_dir/ares_strdup.c \
    $$cares_dir/ares_getnameinfo.c \
    $$cares_dir/ares_gethostbyaddr.c \
    $$cares_dir/_inet_ntop.c \
    $$cares_dir/ares_parse_ptr_reply.c \
    $$cares_dir/ares_android.c \
    $$cares_dir/ares_strsplit.c \
# just for win32???
    $$cares_dir/ares_expand_string.c \
    $$cares_dir/ares_fds.c \
    $$cares_dir/ares_getenv.c \
    $$cares_dir/ares_getopt.c \
    $$cares_dir/ares_mkquery.c \
    $$cares_dir/ares_parse_mx_reply.c \
    $$cares_dir/ares_parse_naptr_reply.c \
    $$cares_dir/ares_parse_ns_reply.c \
    $$cares_dir/ares_parse_soa_reply.c \
    $$cares_dir/ares_parse_srv_reply.c \
    $$cares_dir/ares_parse_txt_reply.c \
    $$cares_dir/ares_platform.c \
    $$cares_dir/ares_strcasecmp.c \
    $$cares_dir/ares_writev.c \
    $$cares_dir/windows_port.c
}
