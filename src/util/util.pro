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

!win32: DEFINES += IOAPI_NO_64  # for zlib-miniZip

DEFINES += BUILDING_LIBCURL  USE_OPENSSL  HAVE_OPENSSL \
            CARES_BUILDING_LIBRARY  #USE_ARES #安卓有问题
            #USE_IPV6

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
    QMAKE_POST_LINK += copy /Y $$DESTDIR\libxutil.so $$XMusicDir\libs\armeabi-v7a
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
    ../../3rd/curl/lib/socketpair.h \
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
    ../../inc/util/ziputil.h \
#
    ../../3rd/curl/include/curl/curl.h \
    ../../3rd/curl/include/curl/curlver.h \
    ../../3rd/curl/include/curl/easy.h \
    ../../3rd/curl/include/curl/mprintf.h \
    ../../3rd/curl/include/curl/multi.h \
    ../../3rd/curl/include/curl/stdcheaders.h \
    ../../3rd/curl/include/curl/system.h \
    ../../3rd/curl/include/curl/typecheck-gcc.h \
    ../../3rd/curl/include/curl/urlapi.h \
#
    ../../3rd/curl/lib/altsvc.h \
    ../../3rd/curl/lib/amigaos.h \
    ../../3rd/curl/lib/arpa_telnet.h \
    ../../3rd/curl/lib/asyn.h \
    ../../3rd/curl/lib/config-amigaos.h \
    ../../3rd/curl/lib/config-dos.h \
    ../../3rd/curl/lib/config-mac.h \
    ../../3rd/curl/lib/config-os400.h \
    ../../3rd/curl/lib/config-plan9.h \
    ../../3rd/curl/lib/config-riscos.h \
    ../../3rd/curl/lib/config-symbian.h \
    ../../3rd/curl/lib/config-tpf.h \
    ../../3rd/curl/lib/config-vxworks.h \
    ../../3rd/curl/lib/config-win32.h \
    ../../3rd/curl/lib/config-win32ce.h \
    ../../3rd/curl/lib/conncache.h \
    ../../3rd/curl/lib/connect.h \
    ../../3rd/curl/lib/content_encoding.h \
    ../../3rd/curl/lib/cookie.h \
    ../../3rd/curl/lib/curl_addrinfo.h \
    ../../3rd/curl/lib/curl_base64.h \
    ../../3rd/curl/lib/curl_ctype.h \
    ../../3rd/curl/lib/curl_des.h \
    ../../3rd/curl/lib/curl_endian.h \
    ../../3rd/curl/lib/curl_fnmatch.h \
    ../../3rd/curl/lib/curl_get_line.h \
    ../../3rd/curl/lib/curl_gethostname.h \
    ../../3rd/curl/lib/curl_gssapi.h \
    ../../3rd/curl/lib/curl_hmac.h \
    ../../3rd/curl/lib/curl_ldap.h \
    ../../3rd/curl/lib/curl_md4.h \
    ../../3rd/curl/lib/curl_md5.h \
    ../../3rd/curl/lib/curl_memory.h \
    ../../3rd/curl/lib/curl_memrchr.h \
    ../../3rd/curl/lib/curl_multibyte.h \
    ../../3rd/curl/lib/curl_ntlm_core.h \
    ../../3rd/curl/lib/curl_ntlm_wb.h \
    ../../3rd/curl/lib/curl_path.h \
    ../../3rd/curl/lib/curl_printf.h \
    ../../3rd/curl/lib/curl_range.h \
    ../../3rd/curl/lib/curl_rtmp.h \
    ../../3rd/curl/lib/curl_sasl.h \
    ../../3rd/curl/lib/curl_sec.h \
    ../../3rd/curl/lib/curl_setup.h \
    ../../3rd/curl/lib/curl_setup_once.h \
    ../../3rd/curl/lib/curl_sha256.h \
    ../../3rd/curl/lib/curl_sspi.h \
    ../../3rd/curl/lib/curl_threads.h \
    ../../3rd/curl/lib/curlx.h \
    ../../3rd/curl/lib/dict.h \
    ../../3rd/curl/lib/doh.h \
    ../../3rd/curl/lib/dotdot.h \
    ../../3rd/curl/lib/easyif.h \
    ../../3rd/curl/lib/escape.h \
    ../../3rd/curl/lib/file.h \
    ../../3rd/curl/lib/fileinfo.h \
    ../../3rd/curl/lib/formdata.h \
    ../../3rd/curl/lib/ftp.h \
    ../../3rd/curl/lib/ftplistparser.h \
    ../../3rd/curl/lib/getinfo.h \
    ../../3rd/curl/lib/gopher.h \
    ../../3rd/curl/lib/hash.h \
    ../../3rd/curl/lib/hostcheck.h \
    ../../3rd/curl/lib/hostip.h \
    ../../3rd/curl/lib/http.h \
    ../../3rd/curl/lib/http2.h \
    ../../3rd/curl/lib/http_chunks.h \
    ../../3rd/curl/lib/http_digest.h \
    ../../3rd/curl/lib/http_negotiate.h \
    ../../3rd/curl/lib/http_ntlm.h \
    ../../3rd/curl/lib/http_proxy.h \
    ../../3rd/curl/lib/if2ip.h \
    ../../3rd/curl/lib/imap.h \
    ../../3rd/curl/lib/inet_ntop.h \
    ../../3rd/curl/lib/inet_pton.h \
    ../../3rd/curl/lib/llist.h \
    ../../3rd/curl/lib/memdebug.h \
    ../../3rd/curl/lib/mime.h \
    ../../3rd/curl/lib/multihandle.h \
    ../../3rd/curl/lib/multiif.h \
    ../../3rd/curl/lib/netrc.h \
    ../../3rd/curl/lib/non-ascii.h \
    ../../3rd/curl/lib/nonblock.h \
    ../../3rd/curl/lib/parsedate.h \
    ../../3rd/curl/lib/pingpong.h \
    ../../3rd/curl/lib/pop3.h \
    ../../3rd/curl/lib/progress.h \
    ../../3rd/curl/lib/psl.h \
    ../../3rd/curl/lib/quic.h \
    ../../3rd/curl/lib/rand.h \
    ../../3rd/curl/lib/rtsp.h \
    ../../3rd/curl/lib/select.h \
    ../../3rd/curl/lib/sendf.h \
    ../../3rd/curl/lib/setopt.h \
    ../../3rd/curl/lib/setup-os400.h \
    ../../3rd/curl/lib/setup-vms.h \
    ../../3rd/curl/lib/share.h \
    ../../3rd/curl/lib/sigpipe.h \
    ../../3rd/curl/lib/slist.h \
    ../../3rd/curl/lib/smb.h \
    ../../3rd/curl/lib/smtp.h \
    ../../3rd/curl/lib/sockaddr.h \
    ../../3rd/curl/lib/socks.h \
    ../../3rd/curl/lib/speedcheck.h \
    ../../3rd/curl/lib/splay.h \
    ../../3rd/curl/lib/ssh.h \
    ../../3rd/curl/lib/strcase.h \
    ../../3rd/curl/lib/strdup.h \
    ../../3rd/curl/lib/strerror.h \
    ../../3rd/curl/lib/strtok.h \
    ../../3rd/curl/lib/strtoofft.h \
    ../../3rd/curl/lib/system_win32.h \
    ../../3rd/curl/lib/telnet.h \
    ../../3rd/curl/lib/tftp.h \
    ../../3rd/curl/lib/timeval.h \
    ../../3rd/curl/lib/transfer.h \
    ../../3rd/curl/lib/url.h \
    ../../3rd/curl/lib/urlapi-int.h \
    ../../3rd/curl/lib/urldata.h \
    ../../3rd/curl/lib/vauth/digest.h \
    ../../3rd/curl/lib/vauth/ntlm.h \
    ../../3rd/curl/lib/vauth/vauth.h \
    ../../3rd/curl/lib/vtls/gskit.h \
    ../../3rd/curl/lib/vtls/gtls.h \
    ../../3rd/curl/lib/vtls/mbedtls.h \
    ../../3rd/curl/lib/vtls/mesalink.h \
    ../../3rd/curl/lib/vtls/nssg.h \
    ../../3rd/curl/lib/vtls/openssl.h \
    ../../3rd/curl/lib/vtls/polarssl.h \
    ../../3rd/curl/lib/vtls/polarssl_threadlock.h \
    ../../3rd/curl/lib/vtls/schannel.h \
    ../../3rd/curl/lib/vtls/sectransp.h \
    ../../3rd/curl/lib/vtls/vtls.h \
    ../../3rd/curl/lib/vtls/wolfssl.h \
    ../../3rd/curl/lib/warnless.h \
    ../../3rd/curl/lib/wildcard.h \
    ../../3rd/curl/lib/x509asn1.h

win32: SOURCES += cpp/winfsutil.cpp  cpp/winfsdlg.cpp

SOURCES += \
    ../../3rd/curl/lib/socketpair.c \
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
#
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

SOURCES += \
    ../../3rd/curl/lib/altsvc.c \
    ../../3rd/curl/lib/amigaos.c \
    ../../3rd/curl/lib/asyn-ares.c \
    ../../3rd/curl/lib/asyn-thread.c \
    ../../3rd/curl/lib/base64.c \
    ../../3rd/curl/lib/conncache.c \
    ../../3rd/curl/lib/connect.c \
    ../../3rd/curl/lib/content_encoding.c \
    ../../3rd/curl/lib/cookie.c \
    ../../3rd/curl/lib/curl_addrinfo.c \
    ../../3rd/curl/lib/curl_ctype.c \
    ../../3rd/curl/lib/curl_des.c \
    ../../3rd/curl/lib/curl_endian.c \
    ../../3rd/curl/lib/curl_fnmatch.c \
    ../../3rd/curl/lib/curl_get_line.c \
    ../../3rd/curl/lib/curl_gethostname.c \
    ../../3rd/curl/lib/curl_gssapi.c \
    ../../3rd/curl/lib/curl_memrchr.c \
    ../../3rd/curl/lib/curl_multibyte.c \
    ../../3rd/curl/lib/curl_ntlm_core.c \
    ../../3rd/curl/lib/curl_ntlm_wb.c \
    ../../3rd/curl/lib/curl_path.c \
    ../../3rd/curl/lib/curl_range.c \
    ../../3rd/curl/lib/curl_rtmp.c \
    ../../3rd/curl/lib/curl_sasl.c \
    ../../3rd/curl/lib/curl_sspi.c \
    ../../3rd/curl/lib/curl_threads.c \
    ../../3rd/curl/lib/dict.c \
    ../../3rd/curl/lib/doh.c \
    ../../3rd/curl/lib/dotdot.c \
    ../../3rd/curl/lib/easy.c \
    ../../3rd/curl/lib/escape.c \
    ../../3rd/curl/lib/file.c \
    ../../3rd/curl/lib/fileinfo.c \
    ../../3rd/curl/lib/formdata.c \
    ../../3rd/curl/lib/ftp.c \
    ../../3rd/curl/lib/ftplistparser.c \
    ../../3rd/curl/lib/getenv.c \
    ../../3rd/curl/lib/getinfo.c \
    ../../3rd/curl/lib/gopher.c \
    ../../3rd/curl/lib/hash.c \
    ../../3rd/curl/lib/hmac.c \
    ../../3rd/curl/lib/hostasyn.c \
    ../../3rd/curl/lib/hostcheck.c \
    ../../3rd/curl/lib/hostip.c \
    ../../3rd/curl/lib/hostip4.c \
    ../../3rd/curl/lib/hostip6.c \
    ../../3rd/curl/lib/hostsyn.c \
    ../../3rd/curl/lib/http.c \
    ../../3rd/curl/lib/http2.c \
    ../../3rd/curl/lib/http_chunks.c \
    ../../3rd/curl/lib/http_digest.c \
    ../../3rd/curl/lib/http_negotiate.c \
    ../../3rd/curl/lib/http_ntlm.c \
    ../../3rd/curl/lib/http_proxy.c \
    ../../3rd/curl/lib/idn_win32.c \
    ../../3rd/curl/lib/if2ip.c \
    ../../3rd/curl/lib/imap.c \
    ../../3rd/curl/lib/inet_ntop.c \
    ../../3rd/curl/lib/inet_pton.c \
    ../../3rd/curl/lib/krb5.c \
    ../../3rd/curl/lib/ldap.c \
    ../../3rd/curl/lib/llist.c \
    ../../3rd/curl/lib/md4.c \
    ../../3rd/curl/lib/md5.c \
    ../../3rd/curl/lib/memdebug.c \
    ../../3rd/curl/lib/mime.c \
    ../../3rd/curl/lib/mprintf.c \
    ../../3rd/curl/lib/multi.c \
    ../../3rd/curl/lib/netrc.c \
    ../../3rd/curl/lib/non-ascii.c \
    ../../3rd/curl/lib/nonblock.c \
    ../../3rd/curl/lib/nwlib.c \
    ../../3rd/curl/lib/nwos.c \
    ../../3rd/curl/lib/openldap.c \
    ../../3rd/curl/lib/parsedate.c \
    ../../3rd/curl/lib/pingpong.c \
    ../../3rd/curl/lib/pop3.c \
    ../../3rd/curl/lib/progress.c \
    ../../3rd/curl/lib/psl.c \
    ../../3rd/curl/lib/rand.c \
    ../../3rd/curl/lib/rtsp.c \
    ../../3rd/curl/lib/security.c \
    ../../3rd/curl/lib/select.c \
    ../../3rd/curl/lib/sendf.c \
    ../../3rd/curl/lib/setopt.c \
    ../../3rd/curl/lib/sha256.c \
    ../../3rd/curl/lib/share.c \
    ../../3rd/curl/lib/slist.c \
    ../../3rd/curl/lib/smb.c \
    ../../3rd/curl/lib/smtp.c \
    ../../3rd/curl/lib/socks.c \
    ../../3rd/curl/lib/socks_gssapi.c \
    ../../3rd/curl/lib/socks_sspi.c \
    ../../3rd/curl/lib/speedcheck.c \
    ../../3rd/curl/lib/splay.c \
    ../../3rd/curl/lib/strcase.c \
    ../../3rd/curl/lib/strdup.c \
    ../../3rd/curl/lib/strerror.c \
    ../../3rd/curl/lib/strtok.c \
    ../../3rd/curl/lib/strtoofft.c \
    ../../3rd/curl/lib/system_win32.c \
    ../../3rd/curl/lib/telnet.c \
    ../../3rd/curl/lib/tftp.c \
    ../../3rd/curl/lib/timeval.c \
    ../../3rd/curl/lib/transfer.c \
    ../../3rd/curl/lib/url.c \
    ../../3rd/curl/lib/urlapi.c \
    ../../3rd/curl/lib/vauth/cleartext.c \
    ../../3rd/curl/lib/vauth/cram.c \
    ../../3rd/curl/lib/vauth/digest.c \
    ../../3rd/curl/lib/vauth/digest_sspi.c \
    ../../3rd/curl/lib/vauth/krb5_gssapi.c \
    ../../3rd/curl/lib/vauth/krb5_sspi.c \
    ../../3rd/curl/lib/vauth/ntlm.c \
    ../../3rd/curl/lib/vauth/ntlm_sspi.c \
    ../../3rd/curl/lib/vauth/oauth2.c \
    ../../3rd/curl/lib/vauth/spnego_gssapi.c \
    ../../3rd/curl/lib/vauth/spnego_sspi.c \
    ../../3rd/curl/lib/vauth/vauth.c \
    ../../3rd/curl/lib/version.c \
    ../../3rd/curl/lib/vtls/gskit.c \
    ../../3rd/curl/lib/vtls/gtls.c \
    ../../3rd/curl/lib/vtls/mbedtls.c \
    ../../3rd/curl/lib/vtls/mesalink.c \
    ../../3rd/curl/lib/vtls/nss.c \
    ../../3rd/curl/lib/vtls/openssl.c \
    ../../3rd/curl/lib/vtls/polarssl.c \
    ../../3rd/curl/lib/vtls/polarssl_threadlock.c \
    ../../3rd/curl/lib/vtls/schannel.c \
    ../../3rd/curl/lib/vtls/schannel_verify.c \
    ../../3rd/curl/lib/vtls/sectransp.c \
    ../../3rd/curl/lib/vtls/vtls.c \
    ../../3rd/curl/lib/vtls/wolfssl.c \
    ../../3rd/curl/lib/warnless.c \
    ../../3rd/curl/lib/wildcard.c \
    ../../3rd/curl/lib/x509asn1.c

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
