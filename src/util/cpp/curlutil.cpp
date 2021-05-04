
#include <curl/curl.h>

#include "util.h"

extern "C" int curltool_init(curl_version_info_data **lpCurlInfo);
extern "C" int curltool_main(int argc, char *argv[], void (*pfnHook)(CURL *curl));

//有问题，各种崩溃 static CURLSH *g_curlShare = NULL;

int curlutil::initCurl(string& strVerInfo)
{
    curl_version_info_data *p = NULL;
    int nRet = curltool_init(&p); //curl_global_init(CURL_GLOBAL_DEFAULT);
    if (CURLE_OK != nRet)
    {
        return nRet;
    }

    if (NULL == p)
    {
        return -1;
    }

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
        ss << "\nprotocols:";
        for (auto protocols = p->protocols; *protocols; protocols++)
        {
            ss << ' ' << *protocols;
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

    /*g_curlShare = curl_share_init();
    if (g_curlShare)
    {
        curl_share_setopt(g_curlShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
        curl_share_setopt(g_curlShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
        curl_share_setopt(g_curlShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
        curl_share_setopt(g_curlShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);
        curl_share_setopt(g_curlShare, CURLSHOPT_SHARE, CURL_LOCK_DATA_PSL);
    }*/

    return CURLE_OK;
}

void curlutil::freeCurl()
{
    /*if (g_curlShare)
    {
        curl_share_cleanup(g_curlShare);
        g_curlShare = NULL;
    }*/

    curl_global_cleanup();
}

static void _initCurl(CURL* curl, const tagCurlOpt& curlOpt)
{
    /*if (curlOpt.bShare)
    {
        curl_easy_setopt(curl, CURLOPT_SHARE, g_curlShare);
    }*/

    //curl_easy_setopt(curl, CURLOPT_DNS_CACHE_TIMEOUT, curlOpt.dnsCacheTimeout);

    if (curlOpt.maxRedirect > 0)
    {
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, curlOpt.maxRedirect);
    }

/*#if !__winvc
#ifndef USE_ARES
    // 规避多线程DNS解析超时崩溃的bug
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
#endif
#endif*/

    if (curlOpt.connectTimeout > 0)
    {
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, curlOpt.connectTimeout);
    }
    if (curlOpt.timeout > 0)
    {
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, curlOpt.timeout);
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

    /*if (curlOpt.keepAliveIdl > 0)
    {
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, curlOpt.keepAliveIdl);
        if (curlOpt.keepAliveInterval > 0)
        {
            curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, curlOpt.keepAliveInterval);
        }
    }*/

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // 当前必須
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    //curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYPEER, 1L);

    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2)

    /*curl_easy_setopt(curl, CURLOPT_CAPATH, fsutil::workDir().c_str());
#if __windows
    curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
#else
    curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
#endif*/

    //curl_easy_setopt(curl, CURLOPT_SSLVERSION, 3L);
    //curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);

    //curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);

    //curl_easy_setopt(curl, CURLOPT_NETRC, (long)CURL_NETRC_IGNORED);

    if (!curlOpt.strUserAgent.empty())
    {
        curl_easy_setopt(curl, CURLOPT_USERAGENT, curlOpt.strUserAgent.c_str());
    }
}

static size_t curl_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    CB_CURL& cb = *(CB_CURL*)userdata;
    return cb(ptr, size, nmemb);
}

static int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                             curl_off_t ultotal, curl_off_t ulnow)
{
    (void)ultotal;
    (void)ulnow;

    CB_CURLProgress& cb = *(CB_CURLProgress*)clientp;
    return cb(dltotal, dlnow);
}

static int _curlDownload(CURL *curl, const string& strUrl, CB_CURL& cbWrite, CB_CURLProgress& cbProgress, curl_off_t *total = NULL)
{
    //curl_easy_setopt(curlInfo.curl, CURLOPT_RESUME_FROM_LARGE, resume_position)参数resume_position必须是curl_off_t，不能使int或double，否则会导致数据异常

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (void*)curl_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&cbWrite);

    if (cbProgress)
    {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, (void*)progress_callback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, (void*)&cbProgress);
    }

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

    if (total)
    {
        (void)curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, total);
    }

    //curl_easy_reset(curl)
    curl_easy_cleanup(curl);

    return res;
}

int curlutil::_curlDownload(const tagCurlOpt& curlOpt, const string& strUrl, CB_CURL& cbWrite, CB_CURLProgress& cbProgress)
{
    auto curl = curl_easy_init();
    _initCurl(curl, curlOpt);
    return ::_curlDownload(curl, strUrl, cbWrite, cbProgress);
}

// 适合下载短文件，可以忽略cbfProfile->append的开销
int curlutil::_curlDownload(const tagCurlOpt& curlOpt, const string& strUrl, CCharBuffer& cbfProfile
                 , CB_CURLProgress& cbProgress)
{
    return _curlDownload(curlOpt, strUrl, [&](char *ptr, size_t size, size_t nmemb){
        size *= nmemb;
        cbfProfile->append(ptr, size);
        return size;
    }, cbProgress);
}

string curlutil::getCurlErrMsg(UINT uCurlCode)
{
    auto pErrMsg = curl_easy_strerror((CURLcode)uCurlCode);
    if (pErrMsg)
    {
        return pErrMsg;
    }

    return "";
}

//curl_easy_pause(__curlptr, CURLPAUSE_RECV);
//curl_easy_pause(__curlptr, CURLPAUSE_RECV_CONT);

/*https使用curl时可加参数“-k”，对应代码：
curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0);
curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0);
作用是接受服务器的ssl证书而不管合不合法，参数等同于“--insecure”*/

static tagCURlToolHook *g_pCURLToolHook = NULL;

static void tool_perform_hook(CURL *curl)
{
    if (NULL == g_pCURLToolHook)
    {
        return;
    }

    if (g_pCURLToolHook->fnProgress)
    {
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, (void*)progress_callback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, (void*)&g_pCURLToolHook->fnProgress);
    }

    if (g_pCURLToolHook->fnWrite)
    {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (void*)curl_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&g_pCURLToolHook->fnWrite);
    }
    else if (g_pCURLToolHook->fnRead)
    {
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, (void*)curl_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, (void*)&g_pCURLToolHook->fnRead);
    }
}

int curlutil::curlToolPerform(const list<string>& lstParams, tagCURlToolHook *pCURLToolHook)
{
     vector<char *> argv{(char*)"curl", (char*)"-k"};
     for (const auto& strPara : lstParams)
     {
         argv.push_back((char*)strPara.c_str());
     }

     static mutex s_mutex;
     s_mutex.lock();

     g_pCURLToolHook = pCURLToolHook;

     int nRet = curltool_main(argv.size(), &argv.front(), tool_perform_hook);

     s_mutex.unlock();
     return nRet;
}

int CCurlDownload::syncDownload(const string& strUrl, UINT uRetryTime, CB_DownloadProgress cbProgress)
{
    m_bStatus = true;
    int nRet = _syncDownload(strUrl, uRetryTime, cbProgress);
    m_bStatus = false;
    return nRet;
}

int CCurlDownload::_syncDownload(const string& strUrl, UINT uRetryTime, CB_DownloadProgress cbProgress)
{
    auto fnProgress = [&](int64_t dltotal, int64_t dlnow){
        if (!m_bStatus)
        {
            return -1;
        }

        if (cbProgress)
        {
            if (!cbProgress(m_beginTime, dltotal, dlnow))
            {
                m_bStatus = false;
                return -1;
            }
        }

        return 0;
    };

    auto cbWrite = [&](char *ptr, size_t size, size_t nmemb)->size_t {
        if (!m_bStatus)
        {
            return 0;
        }

        size *= nmemb;
        if (!_onRecv(ptr, size))
        {
            m_bStatus = false;
            return 0;
        }

        m_uRecvSize += size;
        return size;
    };

    for (UINT uIdx = 0; uIdx <= uRetryTime; uIdx++)
    {
        m_nCurlCode = 0;
        m_uTotalSize = 0; //http头传过来的预期长度
        m_uRecvSize = 0; //已接收长度
        clear();
        //m_strErrMsg.clear();

        m_beginTime = time(NULL);

        auto curl = curl_easy_init();
        _initCurl(curl, m_curlOpt);
        curl_off_t total = 0;
        m_nCurlCode = ::_curlDownload(curl, strUrl, cbWrite, fnProgress, &total);
        if (total > 0)
        {
            m_uTotalSize = (uint64_t)total;
        }

        if (0 == m_nCurlCode)
        {
            break;
        }

        if (!fnProgress(0,0)) //!m_bStatus)
        {
            //clear(); //统一不清，cancel函数也不清
            break;
        }

        //if (m_nCurlCode > 0) m_strErrMsg = curlutil::getCurlErrMsg((UINT)m_nCurlCode);

        if (m_uRecvSize > 0)
        {
            break;
        }
    }

    return m_nCurlCode;
}

void CCurlDownload::asyncDownload(const string& strUrl, UINT uRetryTime, cfn_void_t<int> cbFinish
                                  , CB_DownloadProgress cbProgress)
{
    m_thread.start([=]{
        m_bStatus = true;
        int nRet = _syncDownload(strUrl, uRetryTime, cbProgress);
        if (cbFinish)
        {
            cbFinish(nRet);
        }
        m_bStatus = false;
    });
}

uint64_t CCurlDownload::cancel()
{
    m_bStatus = false;

    m_thread.cancel();

    return m_uRecvSize;
}

bool CDownloader::_onRecv(char *ptr, size_t size)
{
    m_mtxDataLock.lock();
    m_lstData.emplace_back(ptr, size);
    m_uDataSize += size;
    m_mtxDataLock.unlock();

    return true;
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
        auto& strData = m_lstData.front();
        size_t size = strData.size();
        if (size <= buffSize)
        {
            memcpy(pBuff, strData.c_str(), size);
            pBuff += size;
            buffSize -= size;

            uRet += size;

            m_uDataSize -= size;

            m_lstData.pop_front();

            if (0 == buffSize || m_lstData.empty())
            {
                break;
            }
        }
        else
        {
            memcpy(pBuff, strData.c_str(), buffSize);
            uRet += buffSize;

            m_uDataSize -= buffSize;
            strData.erase(0, buffSize);

            break;
        }
    }

    return uRet;
}

/*void CDownloader::cutData(uint64_t uPos)
{
    m_mtxDataLock.lock();
    uint64_t uReadPos = m_uRecvSize - m_uDataSize;
    if (uPos <= uReadPos)
    {
        m_mtxDataLock.unlock();
        return;
    }

    uint64_t uCutSize = uPos - uReadPos;
    TD_ByteBuffer buff(uCutSize);

    m_mtxDataLock.unlock();
    (void)getData(buff);
}*/

void CDownloader::clear()
{
    m_mtxDataLock.lock();

    m_lstData.clear();
    m_uDataSize = 0;

    m_mtxDataLock.unlock();
}
