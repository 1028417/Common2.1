
#pragma once

class __UtilExt wintime
{
public:
	static time_t transFileTime(const FILETIME& ft);

	static bool toSysTime(time64_t time, SYSTEMTIME& sysTime);
	static void toSysTime(const tm& atm, SYSTEMTIME& sysTime);

	static void getCurrentTime(int& nHour, int& nMinute);
	static wstring getCurrentTime();

	static wstring formatTime(const FILETIME& fileTime, const wstring& strFormat);
	static wstring formatTime(time64_t time, const wstring& strFormat);
};
