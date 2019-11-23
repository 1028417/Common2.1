
#include <curl/curl.h>

#include "util.h"

extern "C" int curlInit();
extern "C" int curltool_main(int argc, char *argv[], void *lpWriteCB, void *lpWriteData);

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

    return curlInit();
}

static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    CB_CURLDownload& cb = *(CB_CURLDownload*)userdata;
    return cb(ptr, size, nmemb);
}

static void _initCurl(CURL* curl)
{
    //curl_setopt(curl, CURLOPT_NOSIGNAL, 1); // 规避多线程下dns解析超时崩溃的bug
    // 当前curl库使用c-ares(C library for asynchronous DNS requests)来做名字解析.

    //curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 3000);
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 0);

    //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    //curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50);

    //curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
    //curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, _curlProgress);
    //curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, (void*)this);

    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // 必須
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    //curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYPEER, 1L);

    //curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);

    //curl_easy_setopt(curl, CURLOPT_NETRC, (long)CURL_NETRC_IGNORED);
    static string strUserAgent = "curl/7.66.0";
    curl_easy_setopt(curl, CURLOPT_USERAGENT, strUserAgent.c_str());

/*#if __windows
    curl_easy_setopt(curl, CURLOPT_CAPATH, strutil::wstrToStr(fsutil::workDir()).c_str());
    curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
#else
    curl_easy_setopt(curl, CURLOPT_CAPATH, "/sdcard/XMusic/.xmusic/");
    curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
#endif*/
}

int curlutil::curlPerform(const string& strUrl, const CB_CURLDownload& cb, string& strErrMsg)
{
    CURL* curl = curl_easy_init();
    _initCurl(curl);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, (void*)write_callback);

    curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&cb);

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

int CDownloader::syncDownload(const string& strUrl, const CB_Downloader& cb)
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

    int nCurlCode = curlutil::curlPerform(strUrl, fn);
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

int CDownloader::syncDownload(const string& strUrl, CByteBuffer& bbfData)
{
    int nRet = syncDownload(strUrl);
    if (0 == nRet)
    {
        (void)_getAllData(bbfData);
    }
    return nRet;
}

int CDownloader::syncDownload(const string& strUrl, CCharBuffer& cbfRet)
{
    int nRet = syncDownload(strUrl);
    if (0 == nRet)
    {
        (void)_getAllData(cbfRet);
    }
    return nRet;
}

void CDownloader::asyncDownload(const string& strUrl, const CB_Downloader& cb)
{
    m_thread.start([=](){
        (void)syncDownload(strUrl, cb);
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

