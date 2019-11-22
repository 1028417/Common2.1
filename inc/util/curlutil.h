
#pragma once

using CB_CURLDownload = std::function<size_t(char *ptr, size_t size, size_t nmemb)>;

class __UtilExt curlutil
{
public:
    int initCurl(std::string& strVerInfo);
    int curlPerform(const std::string& strURL, const CB_CURLDownload& cb, int& nRepCode, std::string& strErrMsg);
    int curlPerform(const std::string& strURL, const CB_CURLDownload& cb);

    int curlToolPerform(const std::list<std::string>& lstParams);
    int curlToolPerform(const std::string& strURL, const CB_CURLDownload& cb);
};
