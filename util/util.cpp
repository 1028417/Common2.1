﻿
#include "util.h"

#include <locale>
//static const locale g_locale_CN("Chinese_china");
static const locale g_locale_CN("");
static const collate<wchar_t>& g_collate_CN = use_facet<collate<wchar_t> >(g_locale_CN);

#define CP_ACP                    0           // default to ANSI code page
#define CP_UTF8                   65001       // UTF-8 translation

bool util::checkWChar(const wstring& str)
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

wstring& util::trim(wstring& strText, wchar_t chr)
{
	strText.erase(0, strText.find_first_not_of(chr));
	strText.erase(strText.find_last_not_of(chr) + 1);
	return strText;
}

wstring util::trim(const wstring& strText, wchar_t chr)
{
	wstring strRet(strText);
	trim(strRet, chr);
	return strRet;
}

wstring& util::ltrim(wstring& strText, wchar_t chr)
{
	strText.erase(0, strText.find_first_not_of(chr));
	return strText;
}

wstring util::ltrim(const wstring& strText, wchar_t chr)
{
	wstring strRet(strText);
	ltrim(strRet, chr);
	return strRet;
}

wstring& util::rtrim(wstring& strText, wchar_t chr)
{
	strText.erase(strText.find_last_not_of(chr) + 1);
	return strText;
}

wstring util::rtrim(const wstring& strText, wchar_t chr)
{
	wstring strRet(strText);
	rtrim(strRet, chr);
	return strRet;
}

void util::SplitString(const wstring& strText, wchar_t wcSplitor, vector<wstring>& vecRet, bool bTrim)
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

int util::StrCompareUseCNCollate(const wstring& lhs, const wstring& rhs)
{
	return g_collate_CN.compare(lhs.c_str(), lhs.c_str() + lhs.size()
		, rhs.c_str(), rhs.c_str() + rhs.size());
}

bool util::StrMatchIgnoreCase(const wstring& str1, const wstring& str2)
{
	return 0 == _wcsicmp(str1.c_str(), str2.c_str());
}

//int util::StrFindIgnoreCase(const wstring& str, const wstring& strToFind)
//{
//	if (str.size() < strToFind.size())
//	{
//		return -1;
//	}
//
//	size_t pos = StrLowerCase(str).find(StrLowerCase(strToFind));
//	if (wstring::npos == pos)
//	{
//		return -1;
//	}
//
//	return pos;
//}

void util::LowerCase(wstring& str)
{
	(void)::_wcslwr_s((TCHAR *)str.c_str(), str.size() + 1);
}

wstring util::StrLowerCase(const wstring& str)
{
	wstring strTemp = str;
	LowerCase(strTemp);
	return strTemp;
}

void util::UpperCase(wstring& str)
{
	(void)::_wcsupr_s((TCHAR *)str.c_str(), str.size() + 1);
}

wstring util::StrUpperCase(const wstring& str)
{
	wstring strTemp = str;
	UpperCase(strTemp);
	return str;
}

string util::WStrToStr(const wstring&str, bool bUTF8)
{
	UINT CodePage = bUTF8 ? CP_UTF8 : CP_ACP;

	if (str.empty())
	{
		return "";
	}

	int buffSize = WideCharToMultiByte(CodePage,
		0,
		str.c_str(),
		-1,
		NULL,
		0,
		NULL,
		NULL);
	vector<char> vecBuff(buffSize + 1);
	char* pBuff = &vecBuff.front();// new char[buffSize + 1];
	memset((void*)pBuff, 0, vecBuff.size());
	::WideCharToMultiByte(CodePage,
		0,
		str.c_str(),
		-1,
		pBuff,
		buffSize,
		NULL,
		NULL);
	string strRet = pBuff;
	//delete[] pBuff;
	return strRet;
}

wstring util::StrToWStr(const string&str, bool bUTF8)
{
	UINT CodePage = bUTF8 ? CP_UTF8 : CP_ACP;

	if (str.empty())
	{
		return L"";
	}

	int buffSize = ::MultiByteToWideChar(CodePage,
		0,
		str.c_str(),
		-1,
		NULL,
		0);

	vector<wchar_t> vecBuff(buffSize + 1);
	wchar_t * pBuff = &vecBuff.front();// new wchar_t[buffSize + 1];
	memset(pBuff, 0, sizeof(pBuff[0]) * (buffSize + 1));
	::MultiByteToWideChar(CodePage,
		0,
		str.c_str(),
		-1,
		(LPWSTR)pBuff,
		buffSize);
	wstring strRet = (wchar_t*)pBuff;
	//delete pBuff;
	return strRet;
}

bool util::IsUTF8Str(const string& strText)
{
	if (strText.empty())
	{
		return false;
	}

	const char* str = strText.c_str();

	UINT nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
	unsigned char chr = *str;
	bool bAllAscii = true;
	for (UINT uIndex = 0; str[uIndex] != '\0'; ++uIndex) {
		chr = *(str + uIndex);
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
