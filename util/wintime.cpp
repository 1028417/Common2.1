
#include "util.h"

#include "wintime.h"

time_t wintime::transFileTime(const FILETIME& ft)
{
	ULARGE_INTEGER ui;
	ui.LowPart = ft.dwLowDateTime;
	ui.HighPart = ft.dwHighDateTime;

	return ((LONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);
}

bool wintime::toSysTime(time64_t time, SYSTEMTIME& sysTime)
{
	tm atm;
	if (0 != _localtime64_s(&atm, &time))
	{
		return false;
	}

	toSysTime(atm, sysTime);

	return true;
}

void wintime::toSysTime(const tm& atm, SYSTEMTIME& sysTime)
{
	sysTime.wYear = atm.tm_year + 1900;     // tm_year is 1900 based
	sysTime.wMonth = atm.tm_mon + 1;        // tm_mon is 0 based
	sysTime.wDay = atm.tm_mday;
	sysTime.wHour = atm.tm_hour;
	sysTime.wMinute = atm.tm_min;
	sysTime.wSecond = atm.tm_sec;
}

void wintime::getCurrentTime(int& nHour, int& nMinute)
{
	SYSTEMTIME sysTime;
	toSysTime(time(0), sysTime);
	
	nHour = sysTime.wHour;
	nMinute = sysTime.wMinute;
}

wstring wintime::getCurrentTime()
{
	SYSTEMTIME sysTime;
	toSysTime(time(0), sysTime);

	wchar_t pszRet[64];
	memset(pszRet, 0, sizeof pszRet);
	
	wsprintf(pszRet, L"%u.%02u.%02u_%02u.%02u.%02u"
		, sysTime.wYear, sysTime.wMonth, sysTime.wDay
		, sysTime.wHour, sysTime.wMinute, sysTime.wSecond);

	return pszRet;
}

static wstring _formatTime(const tm& atm, const wstring& strFormat)
{
	wchar_t lpBuff[24];
	memset(lpBuff, 0, sizeof lpBuff);
	if (!wcsftime(lpBuff, sizeof lpBuff, strFormat.c_str(), &atm))
	{
		return L"";
	}

	return lpBuff;
}

wstring wintime::formatTime(const FILETIME& fileTime, const wstring& strFormat)
{
	return formatTime(transFileTime(fileTime), strFormat);
	
	/*SYSTEMTIME sysTime;
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

	return _FormatTime(atm, strFormat);*/
}

wstring wintime::formatTime(time64_t time, const wstring& strFormat)
{
	if (0 == time)
	{
		return L"";
	}

	tm atm;
	if (0 != _localtime64_s(&atm, &time))
	{
		return L"";
	}

	return _formatTime(atm, strFormat);
}
