
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
