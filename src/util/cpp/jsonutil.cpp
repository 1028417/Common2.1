
#include "util.h"

#include "json/reader.h"
#include "json/writer.h"

bool jsonutil::loadData(const string& strData, JValue& jRoot, bool bStrictRoot)
{
    Json::CharReaderBuilder builder;
	if (bStrictRoot)
	{
		builder["strictRoot"] = true;
	}
	auto reader = builder.newCharReader();
	auto begin = strData.c_str();
    bool bRet = reader->parse(begin, begin + strData.size(), &jRoot, NULL);
	delete reader;
    return bRet;

    /*if (bStrictRoot)
	{
		return Json::Reader(Json::Features::strictMode()).parse(strData, jRoot, false);
	}
	else
	{
		return Json::Reader().parse(strData, jRoot, false);
    }*/
}

string jsonutil::toStr(const JValue& jRoot, bool bStyled)
{
    Json::StreamWriterBuilder builder;
	if (bStyled)
	{
		//builder[] = ;
	}
	auto writer = builder.newStreamWriter();
	stringstream sstream;
	writer->write(jRoot, &sstream);
	delete writer;
    return sstream.str();

    /*if (bStyled)
    {
        return Json::StyledWriter().write(jRoot);
    }
    else
    {
        return Json::FastWriter().write(jRoot);
    }*/
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

bool jsonutil::get(const JValue& jValue, wstring& strRet)
{
	string strValue;
	if (!get(jValue, strValue))
	{
		return false;
	}

	if (strutil::checkUtf8(strValue))
	{
		strRet = strutil::fromUtf8(strValue);
	}
	else
	{
		strRet = strutil::fromGbk(strValue);
	}

	return true;
}

bool jsonutil::get(const JValue& jValue, bool bUtf8, wstring& strRet)
{
    string strValue;
    if (!get(jValue, strValue))
    {
        return false;
    }

	if (bUtf8)
	{
		strRet = strutil::fromUtf8(strValue);
	}
	else
	{
		strRet = strutil::fromGbk(strValue);
	}

    return true;
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
bool jsonutil::get(const JValue& jValue, JValue::Int64& nRet)
{
    if (!jValue.isNull())
    {
        nRet = jValue.asInt64();
        return true;
    }

    return false;
}

bool jsonutil::get(const JValue& jValue, JValue::UInt64& uRet)
{
    if (jValue.isUInt() || jValue.isUInt64())
    {
        uRet = jValue.asUInt64();
        return true;
    }

    return false;
}
#endif
