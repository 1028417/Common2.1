#-------------------------------------------------
#
# Project created by QtCreator 2019-03-27T06:30:38
#
#-------------------------------------------------

QT       -= gui

TARGET = xutil
TEMPLATE = lib

QMAKE_CXXFLAGS += -std=c++11 #c++1y #gnu++1y

DEFINES += QT_DEPRECATED_WARNINGS __UtilPrj

DEFINES += TIXML_USE_STL

DEFINES += HAVE_BZIP2

#for zlib-miniZip
!win32: DEFINES += IOAPI_NO_64
#_FILE_OFFSET_BITS=64 #加载这里<fstream>编译报错
#_LARGEFILE_SOURCE _LARGEFILE64_SOURCE
#gnu的ftello等系列函数一般可以用宏-D_FILE_OFFSET_BITS 64指定off_t的类型来控制函数是否支持2G以上的文件,也可以用_LARGEFILE_SOURCE来支持ftello64位之类

DEFINES += BUILDING_LIBCURL  USE_OPENSSL  HAVE_OPENSSL \
            CARES_BUILDING_LIBRARY #USE_ARES #安卓有问题  #USE_IPV6

DEFINES += HAVE_UNISTD_H #for curl7.76

INCLUDEPATH += ../../inc/util \
    ../../3rd/zlib-1.2.11 \
    ../../3rd/bzip2-1.0.6 \
#
    ../../3rd/curl/include ../../3rd/curl/lib ../../3rd/curl/src \
    ../../3rd/c-ares

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

    INCLUDEPATH += ../../3rd/curl/lib/curl_config_android

    platform = android
    DESTDIR = ..\..\libs\armeabi-v7a
    #QMAKE_POST_LINK += copy /Y $$DESTDIR\libxutil.so $$XMusicDir\libs\armeabi-v7a
} else: macx {
    LIBS += -L../../libs/mac  #-lnghttp2  -lz

    INCLUDEPATH += ../../3rd/curl/lib/curl_config_mac

    platform = mac
    DESTDIR = ../../bin/mac

    QMAKE_POST_LINK += cp -f $$DESTDIR/libxutil*.dylib $$XMusicDir/bin/mac/
} else: ios {
    INCLUDEPATH += ../../3rd/curl/lib/curl_config_mac

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
    cpp/ziputil.cpp \
    ../../3rd/zlib-1.2.11/contrib/minizip/zip.c \
    ../../3rd/zlib-1.2.11/contrib/minizip/unzip.c \
    ../../3rd/zlib-1.2.11/contrib/minizip/ioapi.c \
    ../../3rd/zlib-1.2.11/*.c \
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
    ../../3rd/curl/lib/vauth/*.c \
    ../../3rd/curl/lib/vtls/*.c \
    ../../3rd/curl/lib/vquic/*.c \
    ../../3rd/curl/lib/vssh/*.c \
    ../../3rd/curl/lib/*.c \
    ../../3rd/curl/src/*.c

macx {} else: ios {} else {
SOURCES += \
    ../../3rd/c-ares/ares__close_sockets.c \
    ../../3rd/c-ares/ares__get_hostent.c \
    ../../3rd/c-ares/ares__read_line.c \
    ../../3rd/c-ares/ares__timeval.c \
    ../../3rd/c-ares/ares_cancel.c \
    ../../3rd/c-ares/ares_create_query.c \
    ../../3rd/c-ares/ares_data.c \
    ../../3rd/c-ares/ares_destroy.c \
    ../../3rd/c-ares/ares_expand_name.c \
    ../../3rd/c-ares/ares_free_hostent.c \
    ../../3rd/c-ares/ares_free_string.c \
    ../../3rd/c-ares/ares_gethostbyname.c \
    ../../3rd/c-ares/ares_getsock.c \
    ../../3rd/c-ares/ares_init.c \
    ../../3rd/c-ares/ares_library_init.c\
    ../../3rd/c-ares/ares_llist.c \
    ../../3rd/c-ares/ares_nowarn.c \
    ../../3rd/c-ares/ares_options.c \
    ../../3rd/c-ares/ares_parse_a_reply.c \
    ../../3rd/c-ares/ares_parse_aaaa_reply.c \
    ../../3rd/c-ares/ares_process.c \
    ../../3rd/c-ares/ares_query.c \
    ../../3rd/c-ares/ares_search.c \
    ../../3rd/c-ares/ares_send.c \
    ../../3rd/c-ares/ares_strerror.c \
    ../../3rd/c-ares/ares_timeout.c \
    ../../3rd/c-ares/ares_version.c \
    ../../3rd/c-ares/bitncmp.c \
    ../../3rd/c-ares/inet_net_pton.c \
    ../../3rd/c-ares/ares_strdup.c \
    ../../3rd/c-ares/ares_getnameinfo.c \
    ../../3rd/c-ares/ares_gethostbyaddr.c \
    ../../3rd/c-ares/_inet_ntop.c \
    ../../3rd/c-ares/ares_parse_ptr_reply.c \
    ../../3rd/c-ares/ares_android.c \
    ../../3rd/c-ares/ares_strsplit.c \
# just for win32???
    ../../3rd/c-ares/ares_expand_string.c \
    ../../3rd/c-ares/ares_fds.c \
    ../../3rd/c-ares/ares_getenv.c \
    ../../3rd/c-ares/ares_getopt.c \
    ../../3rd/c-ares/ares_mkquery.c \
    ../../3rd/c-ares/ares_parse_mx_reply.c \
    ../../3rd/c-ares/ares_parse_naptr_reply.c \
    ../../3rd/c-ares/ares_parse_ns_reply.c \
    ../../3rd/c-ares/ares_parse_soa_reply.c \
    ../../3rd/c-ares/ares_parse_srv_reply.c \
    ../../3rd/c-ares/ares_parse_txt_reply.c \
    ../../3rd/c-ares/ares_platform.c \
    ../../3rd/c-ares/ares_strcasecmp.c \
    ../../3rd/c-ares/ares_writev.c \
    ../../3rd/c-ares/windows_port.c
}
