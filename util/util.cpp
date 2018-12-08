
#include <util.h>

#include <locale>
static const char *g_lplocaleName_CN = "Chinese_china";
static const locale g_locale_CN = locale(g_lplocaleName_CN);
static const collate<wchar_t>& g_collate_CN = use_facet<collate<wchar_t> >(g_locale_CN);

void util::sys2tm(const SYSTEMTIME& sysTime, tm& atm)
{
	atm.tm_year = sysTime.wYear - 1900;     // tm_year is 1900 based
	atm.tm_mon = sysTime.wMonth - 1;        // tm_mon is 0 based
	atm.tm_mday = sysTime.wDay;
	atm.tm_hour = sysTime.wHour;
	atm.tm_min = sysTime.wMinute;
	atm.tm_sec = sysTime.wSecond;
	atm.tm_isdst = -1;
}

void util::tm2sys(const tm& atm, SYSTEMTIME& sysTime)
{
	sysTime.wYear = atm.tm_year + 1900;     // tm_year is 1900 based
	sysTime.wMonth = atm.tm_mon + 1;        // tm_mon is 0 based
	sysTime.wDay = atm.tm_mday;
	sysTime.wHour = atm.tm_hour;
	sysTime.wMinute = atm.tm_min;
	sysTime.wSecond = atm.tm_sec;
}

static void _getCurrentTime(tm& atm)
{
	time_t time(time(0));
	(void)_localtime64_s(&atm, &time);
}

void util::getCurrentTime(int& nHour, int& nMinute)
{
	tm atm;
	_getCurrentTime(atm);

	nHour = atm.tm_hour;
	nMinute = atm.tm_min;
}

void util::getCurrentTime(SYSTEMTIME& sysTime)
{
	tm atm;
	_getCurrentTime(atm);

	tm2sys(atm, sysTime);
}

static wstring _FormatTime(const tm& atm, const wstring& strFormat)
{
	wchar_t lpBuff[24];
	memset(lpBuff, 0, sizeof lpBuff);
	if (!wcsftime(lpBuff, sizeof lpBuff, strFormat.c_str(), &atm))
	{
		return L"";
	}

	return lpBuff;
}

wstring util::FormatTime(const FILETIME& fileTime, const wstring& strFormat)
{
	SYSTEMTIME sysTime;
	SYSTEMTIME localTime;
	if (!FileTimeToSystemTime(&fileTime, &sysTime)
		|| !SystemTimeToTzSpecificLocalTime(nullptr, &sysTime, &localTime))
	{
		return L"";
	}

	tm atm;
	atm.tm_year = localTime.wYear - 1900;     // tm_year is 1900 based
	atm.tm_mon = localTime.wMonth - 1;        // tm_mon is 0 based
	atm.tm_mday = localTime.wDay;
	atm.tm_hour = localTime.wHour;
	atm.tm_min = localTime.wMinute;
	atm.tm_sec = 0;
	atm.tm_isdst = -1;

	return _FormatTime(atm, strFormat);
}

wstring util::FormatTime(time_t time, const wstring& strFormat)
{
	if (0 == time)
	{
		return L"";
	}

	tm atm;
	if (_localtime64_s(&atm, &time) != 0)
	{
		return L"";
	}

	return _FormatTime(atm, strFormat);
}

void util::trim(wstring& strText, wchar_t chr)
{
	strText.erase(0, strText.find_first_not_of(chr));
	strText.erase(strText.find_last_not_of(chr) + 1);
}

void util::ltrim(wstring& strText, wchar_t chr)
{
	strText.erase(0, strText.find_first_not_of(chr));
}

void util::rtrim(wstring& strText, wchar_t chr)
{
	strText.erase(strText.find_last_not_of(chr) + 1);
}

wstring util::trim(const wstring& strText, wchar_t chr)
{
	wstring strRet(strText);
	trim(strRet, chr);
	return strRet;
}

wstring util::ltrim(const wstring& strText, wchar_t chr)
{
	wstring strRet(strText);
	ltrim(strRet, chr);
	return strRet;
}

wstring util::rtrim(const wstring& strText, wchar_t chr)
{
	wstring strRet(strText);
	rtrim(strRet, chr);
	return strRet;
}

void util::SplitString(const wstring& strText, char cSplitor, vector<wstring>& vecRet, bool bTrim)
{
	wstring::size_type startPos = 0;
	while (true)
	{
		wstring::size_type pos = strText.find(cSplitor, startPos);
		if (wstring::npos == pos)
		{
			break;
		}

		if (pos > startPos)
		{
			wstring strSub = strText.substr(startPos, pos - startPos);
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
			wstring strTail = strText.substr(startPos);
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

bool util::StrCompareIgnoreCase(const wstring& str1, const wstring& str2)
{
	return 0 == _wcsicmp(str1.c_str(), str2.c_str());
}

int util::StrFindIgnoreCase(const wstring& str, const wstring& strToFind)
{
	if (str.size() < strToFind.size())
	{
		return -1;
	}

	wstring::size_type pos = StrLowerCase(str).find(StrLowerCase(strToFind));
	if (wstring::npos == pos)
	{
		return -1;
	}

	return pos;
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
