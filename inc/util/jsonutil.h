#pragma once

#include "json/json.h"

using JValue = Json::Value;
using JReader = Json::Reader;
using JFastWriter = Json::FastWriter;
using JStyledWriter = Json::StyledWriter;

class __UtilExt jsonutil
{
public:
	static bool loadData(const string& strData, JValue& jRoot, bool bStrictRoot = true)
	{
		Json::Features features;
		features.strictRoot_ = bStrictRoot;
		return JReader(features).parse(strData, jRoot, false);
	}

	static bool loadFile(Instream& ins, JValue& jRoot, bool bStrictRoot = true)
	{
		string strText;
        CTxtReader().read(ins, strText);

		return loadData(strText, jRoot, bStrictRoot);
	}
	static bool loadFile(const wstring& strFile, JValue& jRoot, bool bStrictRoot = true)
	{
		IFStream ifs(strFile);
		__EnsureReturn(ifs, false);
		return loadFile(ifs, jRoot, bStrictRoot);
	}

	static bool writeFile(const JValue& jRoot, const wstring& strFile, bool bStyled=false)
	{
		CUTF8TxtWriter TxtWriter;
		if (!TxtWriter.open(strFile, true))
		{
			return false;
		}

		if (bStyled)
		{
			return TxtWriter.write(Json::StyledWriter().write(jRoot));
		}
		else
		{
			return TxtWriter.write(Json::FastWriter().write(jRoot));
		}
	}

    static bool get(const JValue& jValue, wstring& strRet);

    static bool get(const JValue& jValue, string& strRet);

    static bool get(const JValue& jValue, bool& bRet);

    static bool get(const JValue& jValue, int& nRet);

    static bool get(const JValue& jValue, UINT& uRet);

    template <typename T>
    static bool getArray(const JValue& jValue, list<T>& lstRet)
    {
        if (!jValue.isArray())
        {
            return false;
        }

        T value;
        for (UINT uIdx = 0; uIdx < jValue.size(); uIdx++)
        {
            if (get(jValue[uIdx], value))
            {
                lstRet.push_back(value);
            }
        }

        return true;
    }
};