﻿
#include "util.h"

#include <locale>
//static const locale g_locale_CN("Chinese_china");
static const locale g_locale_CN("");
static const collate<wchar_t>& g_collate_CN = use_facet<collate<wchar_t> >(g_locale_CN);

bool wsutil::checkWChar(const wstring& str)
{
	for (auto& wchr : str)
	{
		if (wchr > 255)
		{
			return true;
		}
	}

	return false;
}

void wsutil::trim(wstring& strText, wchar_t chr)
{
	strText.erase(0, strText.find_first_not_of(chr));
	strText.erase(strText.find_last_not_of(chr) + 1);
}

wstring wsutil::trim_r(const wstring& strText, wchar_t chr)
{
	wstring strRet(strText);
	trim(strRet, chr);
	return strRet;
}

void wsutil::ltrim(wstring& strText, wchar_t chr)
{
	strText.erase(0, strText.find_first_not_of(chr));
}

wstring wsutil::ltrim_r(const wstring& strText, wchar_t chr)
{
	wstring strRet(strText);
	ltrim(strRet, chr);
	return strRet;
}

void wsutil::rtrim(wstring& strText, wchar_t chr)
{
	strText.erase(strText.find_last_not_of(chr) + 1);
}

wstring wsutil::rtrim_r(const wstring& strText, wchar_t chr)
{
	wstring strRet(strText);
	rtrim(strRet, chr);
	return strRet;
}

void wsutil::split(const wstring& strText, wchar_t wcSplitor, vector<wstring>& vecRet, bool bTrim)
{
	size_t startPos = 0;
	while (true)
	{
		auto pos = strText.find(wcSplitor, startPos);
		if (wstring::npos == pos)
		{
			break;
		}

		if (pos > startPos)
		{
			auto strSub = strText.substr(startPos, pos - startPos);
			if (bTrim)
			{
				trim(strSub);
				if (!strSub.empty())
				{
					vecRet.push_back(strSub);
				}
			}
			else
			{
				vecRet.push_back(strSub);
			}
		}
		
		startPos = pos + 1;
	}

	if (startPos < strText.size())
	{
		if (bTrim)
		{
			auto strTail = strText.substr(startPos);
			trim(strTail);

			if (!strTail.empty())
			{
				vecRet.push_back(strTail);
			}
		}
		else
		{
			vecRet.push_back(strText.substr(startPos));
		}
	}
}

int wsutil::compareUseCNCollate(const wstring& lhs, const wstring& rhs)
{
	return g_collate_CN.compare(lhs.c_str(), lhs.c_str() + lhs.size()
		, rhs.c_str(), rhs.c_str() + rhs.size());
}

int wsutil::compareIgnoreCase(const wstring& str1, const wstring& str2, size_t size)
{
#ifdef __ANDROID__
    if (0 == size)
    {
        size = MIN(str1.size(), str2.size()) + 1;
    }
    return wcsncmp(str1.c_str(), str2.c_str(), size);

#else
	if (0 != size)
    {
        return _wcsnicmp(str1.c_str(), str2.c_str(), size);
	}
	else
	{
		return _wcsicmp(str1.c_str(), str2.c_str());
	}
#endif
}

bool wsutil::matchIgnoreCase(const wstring& str1, const wstring& str2)
{
	return 0 == compareIgnoreCase(str1, str2);
}

//int wsutil::findIgnoreCase(const wstring& str, const wstring& strToFind)
//{
//	if (str.size() < strToFind.size())
//	{
//		return -1;
//	}
//
//	size_t pos = lowerCase(str).find(lowerCase(strToFind));
//	if (wstring::npos == pos)
//	{
//		return -1;
//	}
//
//	return pos;
//}

void wsutil::lowerCase(wstring& str)
{
#ifdef __ANDROID__
	str = __QStr(str).toLower().toStdWString();
#else
	(void)::_wcslwr_s((wchar_t*)str.c_str(), str.size() + 1);
#endif
}

wstring wsutil::lowerCase_r(const wstring& str)
{
	wstring strRet = str;
	lowerCase(strRet);
	return strRet;
}

void wsutil::upperCase(wstring& str)
{
#ifdef __ANDROID__
	str = __QStr(str).toUpper().toStdWString();
#else
	(void)::_wcsupr_s((wchar_t*)str.c_str(), str.size() + 1);
#endif	
}

wstring wsutil::upperCase_r(const wstring& str)
{
	wstring strRet = str;
	upperCase(strRet);
	return strRet;
}

void wsutil::replace(wstring& str, wchar_t chrFind, wchar_t chrReplace)
{
	for (auto& chr : str)
	{
		if (chrFind == chr)
		{
			chr = chrReplace;
		}
	}
}

void wsutil::replace(wstring& str, const wstring& strFindChars, wchar_t chrReplace)
{
	for (auto& chr : str)
	{
		if (wstring::npos != strFindChars.find(chr))
		{
			chr = chrReplace;
		}
	}
}

#ifdef _MSC_VER
#include <codecvt>
using utf8_convert = std::wstring_convert<std::codecvt_utf8<wchar_t>>;
static utf8_convert g_utf8Convert;
#endif

wstring wsutil::fromUTF8(const string& str)
{
	if (str.empty())
	{
		return L"";
	}

#ifndef _MSC_VER
    return QString::fromUtf8(str.c_str()).toStdWString();
#else
    return g_utf8Convert.from_bytes(str);
#endif
}/*
	int buffSize = ::MultiByteToWideChar(CP_UTF8,
		0,
		str.c_str(),
		-1,
		NULL,
		0);
	
	vector<wchar_t> vecBuff(buffSize + 1);
	wchar_t *pBuff = &vecBuff.front();// new wchar_t[buffSize + 1];
	memset(pBuff, 0, sizeof(pBuff[0]) * (buffSize + 1));
	::MultiByteToWideChar(CP_UTF8,
		0,
		str.c_str(),
		-1,
		pBuff,
		buffSize);
	
	return pBuff;
}*/

string wsutil::toUTF8(const wstring& str)
{
	if (str.empty())
	{
		return "";
	}

#ifndef _MSC_VER
    return __QStr(str).toUtf8().constData();
#else
	return g_utf8Convert.to_bytes(str);
#endif
}/*
    int buffSize = ::WideCharToMultiByte(CP_UTF8,
		0,
		str.c_str(),
		-1,
		NULL,
		0,
		NULL,
		NULL);

	vector<char> vecBuff(buffSize + 1);
	char *pBuff = &vecBuff.front();// new char[buffSize + 1];
	memset((void*)pBuff, 0, vecBuff.size());
	::WideCharToMultiByte(CP_UTF8,
		0,
		str.c_str(),
		-1,
		pBuff,
		buffSize,
		NULL,
		NULL);
	
	return pBuff;
}*/

string wsutil::toStr(const wstring& str)
{
	if (str.empty())
	{
		return "";
	}

	size_t len = 0;
#ifdef __ANDROID__
    len = wcstombs(NULL, str.c_str(), 0);
#else
    if (wcstombs_s(&len, NULL, 0, str.c_str(), 0))
	{
		return "";
	}
#endif
    if (0 == len)
    {
        return "";
    }

	vector<char> vecBuff(len + 1);
	char *pBuff = &vecBuff.front();

#ifdef __ANDROID__
    (void)wcstombs(pBuff, str.c_str(), len);
#else
	if (wcstombs_s(NULL, pBuff, len, str.c_str(), len))
	{
		return "";
	}
#endif

	return pBuff;
}

static bool _checkUTF8(const string& str)
{
	if (str.empty())
	{
		return false;
	}

	const char* pstr = str.c_str();

	UINT nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
	unsigned char chr = *pstr;
	bool bAllAscii = true;
	for (UINT uIndex = 0; pstr[uIndex] != '\0'; ++uIndex) {
		chr = *(pstr + uIndex);
		//判断是否ASCII编码,如果不是,说明有可能是UTF8,ASCII用7位编码,最高位标记为0,0xxxxxxx
		if (nBytes == 0 && (chr & 0x80) != 0) {
			bAllAscii = false;
		}
		if (nBytes == 0) {
			//如果不是ASCII码,应该是多字节符,计算字节数
			if (chr >= 0x80) {
				if (chr >= 0xFC && chr <= 0xFD) {
					nBytes = 6;
				}
				else if (chr >= 0xF8) {
					nBytes = 5;
				}
				else if (chr >= 0xF0) {
					nBytes = 4;
				}
				else if (chr >= 0xE0) {
					nBytes = 3;
				}
				else if (chr >= 0xC0) {
					nBytes = 2;
				}
				else {
					return false;
				}
				nBytes--;
			}
		}
		else {
			//多字节符的非首字节,应为 10xxxxxx
			if ((chr & 0xC0) != 0x80) {
				return false;
			}
			//减到为零为止
			nBytes--;
		}
	}
	//违返UTF8编码规则
	if (nBytes != 0) {
		return false;
	}
	if (bAllAscii) { //如果全部都是ASCII, 也是UTF8
		return true;
	}

	return true;
}

static wstring _fromStr(const string& str)
{
	if (str.empty())
	{
		return L"";
	}

	size_t len = 0;
#ifdef __ANDROID__
	len = mbstowcs(NULL, str.c_str(), 0);
#else
	if (mbstowcs_s(&len, NULL, 0, str.c_str(), 0))
	{
		return L"";
	}
#endif
	if (0 == len)
	{
		return L"";
	}

	vector<wchar_t> vecBuff(len + 1);
	wchar_t *pBuff = &vecBuff.front();

#ifdef __ANDROID__
	(void)mbstowcs(pBuff, str.c_str(), len);
#else
	if (mbstowcs_s(NULL, pBuff, len, str.c_str(), len))
	{
		return L"";
	}
#endif

	return pBuff;
}

wstring wsutil::fromStr(const string& str, bool bCheckUTF8)
{
	if (str.empty())
	{
		return L"";
	}

	if (bCheckUTF8 && _checkUTF8(str))
	{
		return fromUTF8(str);
	}

	return _fromStr(str);
}
