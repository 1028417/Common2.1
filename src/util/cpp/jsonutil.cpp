
#include "util.h"

bool jsonutil::load(const wstring& strFile, JValue& jRoot, bool bStrictRoot)
{
    string strData;
    if (!fsutil::loadTxt(strFile, strData))
    {
        return false;
    }

    Json::Features features;
    features.strictRoot_ = bStrictRoot;
    if (!JReader(features).parse(strData, jRoot, false))
    {
        return false;
    }

    return true;
}

bool jsonutil::get(const JValue& jValue, wstring& strRet)
{
    if (jValue.isNull())
    {
        return false;
    }

    strRet = strutil::toWstr(jValue.asString(), true);

    return true;
}

bool jsonutil::get(const JValue& jValue, string& strRet)
{
    if (jValue.isNull())
    {
        return false;
    }

    strRet = jValue.asString();

    return true;
}

bool jsonutil::get(const JValue& jValue, bool& bRet)
{
    if (jValue.isNull())
    {
        return false;
    }

    bRet = jValue.asBool();

    return true;
}

bool jsonutil::get(const JValue& jValue, int& nRet)
{
    if (jValue.isNull())
    {
        return false;
    }

    nRet = jValue.asInt();

    return true;
}

bool jsonutil::get(const JValue& jValue, UINT& uRet)
{
    if (jValue.isNull())
    {
        return false;
    }

    uRet = jValue.asUInt();

    return true;
}
