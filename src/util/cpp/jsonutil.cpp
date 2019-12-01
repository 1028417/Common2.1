
#include "util.h"

bool jsonutil::get(const JValue& jValue, wstring& strRet)
{
    if (jValue.isNull())
    {
        return false;
    }

	cauto str = jValue.asString();
	if (strutil::checkUtf8(str))
	{
		strRet = strutil::fromUtf8(str);
	}
	else
	{
		strRet = strutil::toWstr(str);
	}

    return true;
}

bool jsonutil::get(const JValue& jValue, string& strRet)
{
    if (!jValue.isNull())
    {
        strRet = jValue.asString();
        return true;
    }

    return false;
}

bool jsonutil::get(const JValue& jValue, bool& bRet)
{
    if (!jValue.isNull())
    {
        bRet = jValue.asBool();
        return true;
    }

    return false;
}

bool jsonutil::get(const JValue& jValue, int& nRet)
{
    if (!jValue.isNull())
    {
        nRet = jValue.asInt();
        return true;
    }

    return false;
}

bool jsonutil::get(const JValue& jValue, unsigned int& uRet)
{
    if (jValue.isUInt())
    {
        uRet = jValue.asUInt();
        return true;
    }

    return false;
}

#if !__winvc
#define sscanf_s sscanf
#endif

bool jsonutil::getHex(const JValue& jValue, unsigned int& uRet)
{
    string strValue;
    if (!get(jValue, strValue))
    {
        return false;
    }

    return 1 == sscanf_s(strValue.c_str(), "%X", &uRet);
}

#ifdef JSON_HAS_INT64
bool jsonutil::get(const JValue& jValue, Json::Value::Int64& nRet)
{
    if (!jValue.isNull())
    {
        nRet = jValue.asInt64();
        return true;
    }

    return false;
}

bool jsonutil::get(const JValue& jValue, Json::Value::UInt64& uRet)
{
    if (jValue.isUInt() || jValue.isUInt64())
    {
        uRet = jValue.asUInt64();
        return true;
    }

    return false;
}
#endif
