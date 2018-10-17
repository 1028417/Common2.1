
#include "stdafx.h"

BOOL util::StrCompareIgnoreCase(const wstring& str1, const wstring& str2)
{
	return 0 == _wcsicmp(str1.c_str(), str2.c_str());
}

int util::StrFind(const wstring& str, const wstring& strToFind, bool bIgnoreCase)
{
	__EnsureReturn(str.size() >= strToFind.size(), -1);

	if (bIgnoreCase)
	{
		wstring::size_type pos = StrLowerCase(str).find(StrLowerCase(strToFind));
		if (wstring::npos == pos)
		{
			return -1;
		}

		return pos;
	}
	else
	{
		wstring::size_type pos = str.find(strToFind);
		if (wstring::npos == pos)
		{
			return -1;
		}

		return pos;
	}
}

//int util::StrLowercaseFind(const wstring& str, const wstring& strToFind)
//{
//	__EnsureReturn(str.size() >= strToFind.size(), -1);
//
//	wstring::size_type pos = StrLowerCase(str).find(strToFind);
//	if (wstring::npos == pos)
//	{
//		return -1;
//	}
//
//	return pos;
//}
//
//int util::StrUppercaseFind(const wstring& str, const wstring& strToFind)
//{
//	__EnsureReturn(str.size() >= strToFind.size(), -1);
//
//	wstring::size_type pos = StrUpperCase(str).find(strToFind);
//	if (wstring::npos == pos)
//	{
//		return -1;
//	}
//
//	return pos;
//}

wstring util::StrLowerCase(const wstring& str)
{
	wstring strTemp = str;
	
	(void)::_wcslwr_s((TCHAR *)strTemp.c_str(), strTemp.size() + 1);

	return strTemp;
}

wstring util::StrUpperCase(const wstring& str)
{
	wstring strTemp = str;

	(void)::_wcsupr_s((TCHAR *)strTemp.c_str(), strTemp.size() + 1);

	return strTemp;
}

string util::WStrToStr(const wstring&str, UINT CodePage)
{
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

wstring util::StrToWStr(const string&str, UINT CodePage)
{
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
	for (UINT i = 0; str[i] != '\0'; ++i) {
		chr = *(str + i);
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
