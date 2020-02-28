
#include "util.h"

void tmutil::getCurrentTime(int& nHour, int& nMinute)
{
    tagTM tm;
    timeToTM(time(0), tm);

    nHour = tm.tm_hour;
    nMinute = tm.tm_min;
}

bool tmutil::timeToTM(time_t tTime, tagTM& tm)
{
	struct tm _tm;

#if __windows
    if (localtime_s(&_tm, &tTime))
	{
		return false;
	}
#else
    if (localtime_r(&tTime, &_tm))
    {
        return L"";
    }
#endif

	tm = _tm;

	return true;
}

static wstring _formatTime(const tm& atm, const wstring& strFormat)
{
	wchar_t lpBuff[64];
	memzero(lpBuff);
	if (!wcsftime(lpBuff, sizeof(lpBuff), strFormat.c_str(), &atm))
	{
		return L"";
	}

	return lpBuff;
}

static string _formatTime(const tm& atm, const string& strFormat)
{
	char lpBuff[64];
	memzero(lpBuff);
	if (!strftime(lpBuff, sizeof(lpBuff), strFormat.c_str(), &atm))
	{
		return "";
	}

	return lpBuff;
}

template <class T>
inline static T _formatTimeT(const T& strFormat, time32_t tTime)
{
    struct tm _tm;
#if __windows
    if (_localtime32_s(&_tm, &tTime))
	{
        return T();
	}
#else
    if (localtime_r(&tTime, &_tm))
    {
        return T();
    }
#endif

    return _formatTime(_tm, strFormat);
}

wstring tmutil::formatTime(const wstring& strFormat, time32_t tTime)
{
    return _formatTimeT(strFormat, tTime);
}

string tmutil::formatTime(const string& strFormat, time32_t tTime)
{
    return _formatTimeT(strFormat, tTime);
}

#if __windows
template <class T>
inline static T _formatTime64T(const T& strFormat, time64_t tTime)
{
    tm atm;
    if (_localtime64_s(&atm, &tTime))
    {
        return T();
    }

    return _formatTime(atm, strFormat);
}

wstring tmutil::formatTime(const wstring& strFormat, time64_t tTime)
{
    return _formatTime64T(strFormat, tTime);
}

string tmutil::formatTime(const string& strFormat, time64_t tTime)
{
    return _formatTime64T(strFormat, tTime);
}

time64_t tmutil::transFileTime(unsigned long dwLowDateTime, unsigned long dwHighDateTime)
{
	ULARGE_INTEGER ui;
	ui.LowPart = dwLowDateTime;
	ui.HighPart = dwHighDateTime;

	return (ui.QuadPart - 116444736000000000) / 10000000;
}
#endif
