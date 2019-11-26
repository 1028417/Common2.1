
#include <curl/curl.h>

#include "util.h"

extern "C" int curlInit();
extern "C" int curltool_main(int argc, char *argv[], void *lpWriteCB, void *lpWriteData);

CURLSH *g_curlShare = NULL;

int curlutil::initCurl(string& strVerInfo)
{
    curl_version_info_data *p = curl_version_info(CURLVERSION_NOW);

    stringstream ss;

    ss << "age: " << p->age;

    if (p->version) ss << "\nversion: " << p->version;
    ss << "\nversion_num: " << p->version_num;

    if (p->host) ss << "\nhost: " << p->host;

    ss << "\nfeatures: " << p->features;
    ss << "\nASYNCHDNS: " << (p->features&CURL_VERSION_ASYNCHDNS);

    if (p->ssl_version) ss << "\nssl_version: " << p->ssl_version;
    ss << "\nssl_version_num: " << p->ssl_version_num;

    if (p->libz_version) ss << "\nlibz_version: " << p->libz_version;

    if (p->protocols)
    {
        for (auto protocols = p->protocols; *protocols; protocols++)
        {
            ss << "\nprotocols: " << *protocols;
        }
    }

    if (p->ares) ss << "\nares: " << p->ares;
    ss << "\nares_num: " << p->ares_num;

    if (p->libidn) ss << "\nlibidn: " << p->libidn;

    ss << "\niconv_ver_num: " << p->iconv_ver_num;

    if (p->libssh_version) ss << "\nlibssh_version: " << p->libssh_version;

    ss << "\nbrotli_ver_num: " << p->brotli_ver_num;

    if (p->brotli_version) ss << "\nbrotli_version: " << p->brotli_version;

    ss << "\nghttp2_ver_num: " << p->nghttp2_ver_num;

    if (p->nghttp2_version) ss << "\nnghttp2_version: " << p->nghttp2_version;

    if (p->quic_version) ss << "\nquic_version: " << p->quic_version;

    strVerInfo.append(ss.str());

    if (!curlInit())
    {
        return false;
    }

    g_curlShare = curl_share_init();
    if (NULL == g_curlShare)
    {
        return false;
    }

    curl_share_setopt(g_curlShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
    curl_share_setopt(g_curlShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    curl_share_setopt(g_curlShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
    curl_share_setopt(g_curlShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);
    curl_share_setopt(g_curlShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_PSL);

    return true;
}

void freeCurl()
{
    curl_share_cleanup(g_curlShare);
    g_curlShare = NULL;

    curl_global_cleanup();
}

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    CB_CURLDownload& cb = *(CB_CURLDownload*)userdata;
    return cb(ptr, size, nmemb);
}

struct tagCurlOpt
{
    tagCurlOpt(bool t_bShare, long t_dnsCacheTimeout)
        : bShare(t_bShare)
        , dnsCacheTimeout(t_dnsCacheTimeout)
    {
    }

    bool bShare;

    long dnsCacheTimeout;

    string strUserAgent;

    unsigned long maxRedirect = 0;

    unsigned long timeout = 0;
    unsigned long connectTimeout = 0;

    unsigned long lowSpeedLimit = 0;
    unsigned long lowSpeedLimitTime = 0;

    unsigned long maxSpeedLimit = 0;

    unsigned long keepAliveInterval = 0;
    unsigned long keepAliveIdl = 0;
};

/*
1.设置进度条用curl_easy_setopt(curlhandle, CURLOPT_XFERINFOFUNCTION, progress_callback);需要注意的是CURLOPT_XFERINFOFUNCTION所对应的回调函数参数必须是curl_off_t，不能使double；但CURLOPT_PROGRESSFUNCTION所对应的回调函数必须是double，不能输curl_off_t。如果形参设置不对，会导致数据出现异常。官方建议使用CURLOPT_XFERINFOFUNCTION，它是CURLOPT_PROGRESSFUNCTION的替代品。
2.在设置curl_easy_setopt(curlhandle, CURLOPT_RESUME_FROM_LARGE, resume_position)参数resume_position必须是curl_off_t，不能使int或double，否则会导致数据异常
*/

static void _initCurl(CURL* curl, const tagCurlOpt& curlOpt)
{
    if (curlOpt.bShare)
    {
        curl_easy_setopt(curl, CURLOPT_SHARE, g_curlShare);
    }

    curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, curlOpt.dnsCacheTimeout);

    //curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);

    //curl_easy_setopt(curl, CURLOPT_NETRC, (long)CURL_NETRC_IGNORED);

    if (!curlOpt.strUserAgent.empty())
    {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, curlOpt.strUserAgent.c_str());
    }

/*#if !__winvc
#ifndef USE_ARES
    // 规避多线程DNS解析超时崩溃的bug
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
#endif
#endif*/

    if (curlOpt.maxRedirect > 0)
    {
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, curlOpt.maxRedirect);
    }

    if (curlOpt.timeout > 0)
    {
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, curlOpt.timeout);
    }
    if (curlOpt.connectTimeout > 0)
    {
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, curlOpt.connectTimeout);
    }

    if (curlOpt.lowSpeedLimit > 0)
    {
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, curlOpt.lowSpeedLimit);
        auto lowSpeedLimitTime = MAX(1L, curlOpt.lowSpeedLimitTime);
        curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, lowSpeedLimitTime);
    }

    if (curlOpt.maxSpeedLimit > 0)
    {
        curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, curlOpt.maxSpeedLimit);
    }

    if (curlOpt.keepAliveInterval > 0)
    {
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, curlOpt.keepAliveInterval);

        auto keepAliveIdl = 0 == curlOpt.keepAliveIdl ? curlOpt.keepAliveInterval : curlOpt.keepAliveIdl;
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, keepAliveIdl);
    }

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // 当前必須
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    //curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYPEER, 1L);

    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2)
/*#if __windows
    curl_easy_setopt(curl, CURLOPT_CAPATH, strutil::wstrToStr(fsutil::workDir()).c_str());
    curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
#else
    curl_easy_setopt(curl, CURLOPT_CAPATH, "/sdcard/XMusic/.xmusic/");
    curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
#endif*/

    //curl_easy_setopt(curl, CURLOPT_SSLVERSION, 3L);
    //curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);
}

int curlutil::curlPerform(const string& strUrl, const CB_CURLDownload& cb, string& strErrMsg)
{
    CURL* curl = curl_easy_init();

    tagCurlOpt curlOpt(false, 0);
    curlOpt.strUserAgent = "curl/7.66.0";

    curlOpt.connectTimeout = 3;

    curlOpt.lowSpeedLimit = 1024;
    curlOpt.lowSpeedLimitTime = 5;

    _initCurl(curl, curlOpt);

    //CURLOPT_RESUME_FROM_LARGE

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (void*)write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&cb);

    //curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
    //curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, _curlProgress);
    //curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, (void*)this);

    int res = curl_easy_perform(curl);
    if (0 == res)
    {
        long nRepCode = 0; // mac64位用int会崩溃
        (void)curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &nRepCode);
        if (nRepCode != 200)
        {
            res = 0-(int)nRepCode;
        }
    }
    else
    {
         if (CURLE_ABORTED_BY_CALLBACK != res)
         {
             auto pErrMsg = curl_easy_strerror((CURLcode)res);
             if (pErrMsg)
             {
                 strErrMsg = pErrMsg;
             }
         }
    }

    //curl_easy_reset(curl)
    curl_easy_cleanup(curl);

    return res;
}

int curlutil::curlPerform(const string& strURL, const CB_CURLDownload& cb)
{
    string strErrMsg;
    return curlPerform(strURL, cb, strErrMsg);
    (void)strErrMsg;
}

//curl_easy_pause(__curlptr, CURLPAUSE_RECV);
//curl_easy_pause(__curlptr, CURLPAUSE_RECV_CONT);

int _callCURLTool(const list<string>& lstParams
                         , void *lpWriteCB = NULL, void *lpWriteData = NULL)
{
    vector<char *> argv{(char*)"curl", (char*)"-k"};
    for (const auto& strPara : lstParams)
    {
        argv.push_back((char*)strPara.c_str());
    }

    static mutex s_mutex;
    s_mutex.lock();
    int nRet = curltool_main(argv.size(), &argv.front(), lpWriteCB, lpWriteData);
    s_mutex.unlock();
    return nRet;
}

int curlutil::curlToolPerform(const list<string>& lstParams)
{
    return _callCURLTool(lstParams);
}

int curlutil::curlToolPerform(const string& strURL, const CB_CURLDownload& cb)
{
    list<string> lstParams(1, strURL);
    return _callCURLTool(lstParams, (void*)write_callback, (void*)&cb);
}

int CDownloader::syncDownload(const string& strUrl, UINT uRetryTime, const CB_Downloader& cb)
{
    _clear();

    auto fn = [&](char *ptr, size_t size, size_t nmemb)->size_t {
        if (!m_bStatus)
        {
            return 0;
        }

        size *= nmemb;
        byte_t *pData = new byte_t[size];
        memcpy(pData, ptr, size);

        if (cb)
        {
            cb(pData, size);
        }

        m_mtxDataLock.lock();
        m_lstData.emplace_back(pData, size);
        m_uDataSize += size;
        m_uSumSize += size;
        m_mtxDataLock.unlock();

        if (m_uDataSize > 4e7)
        {
            mtutil::usleep(50);
        }

        return size;
    };

    m_bStatus = true;

    int nCurlCode = 0;
    for (UINT uIdx = 0; uIdx <= uRetryTime; uIdx++)
    {
        nCurlCode = curlutil::curlPerform(strUrl, fn);
        if (0 == nCurlCode)
        {
            break;
        }

        if (m_uSumSize > 0)
        {
            break;
        }

        if (!m_bStatus)
        {
            break;
        }
    }

    /*if (nCurlCode != 0)
    {
        if (m_bStatus)
        {
            g_logger << "curlToolPerform fail: " >> nCurlCode;
        }
    }*/

    m_bStatus = false;

    return nCurlCode;
}

int CDownloader::syncDownload(const string& strUrl, CByteBuffer& bbfData, UINT uRetryTime)
{
    int nRet = syncDownload(strUrl, uRetryTime);
    if (0 == nRet)
    {
        (void)_getAllData(bbfData);
    }
    return nRet;
}

int CDownloader::syncDownload(const string& strUrl, CCharBuffer& cbfRet, UINT uRetryTime)
{
    int nRet = syncDownload(strUrl, uRetryTime);
    if (0 == nRet)
    {
        (void)_getAllData(cbfRet);
    }
    return nRet;
}

void CDownloader::asyncDownload(const string& strUrl, UINT uRetryTime, const CB_Downloader& cb)
{
    m_thread.start([=](){
        (void)syncDownload(strUrl, uRetryTime, cb);
    });
}

int CDownloader::getData(byte_t *pBuff, size_t buffSize)
{
    mutex_lock lock(m_mtxDataLock);
    if (m_lstData.empty())
    {
        if (!m_bStatus)
        {
            return -1;
        }

        return 0;
    }

    size_t uRet = 0;
    while (true)
    {
        auto& prData = m_lstData.front();
        byte_t *pData = prData.first;
        size_t& size = prData.second;
        if (size <= buffSize)
        {
            memcpy(pBuff, pData, size);
            pBuff += size;
            buffSize -= size;

            uRet += size;

            m_uDataSize -= size;

            delete pData;
            m_lstData.pop_front();

            if (0 == buffSize || m_lstData.empty())
            {
                break;
            }
        }
        else
        {
            memcpy(pBuff, pData, buffSize);
            uRet += buffSize;

            size -= buffSize;
            memcpy(pData, pData+buffSize, size);

            m_uDataSize -= buffSize;

            break;
        }
    }

    return uRet;
}

void CDownloader::cancel()
{
    m_bStatus = false;

    _clear();

    m_thread.cancel();
}

void CDownloader::_clear()
{
    mutex_lock lock(m_mtxDataLock);

    for (auto& prData : m_lstData)
    {
        delete prData.first;
    }
    m_lstData.clear();

    m_uDataSize = 0;
    m_uSumSize = 0;
}

