
#include "util.h"

void tmutil::getCurrentTime(int& nHour, int& nMinute)
{
    tagTM tm;
    timeToTM((time32_t)time(0), tm);

    nHour = tm.tm_hour;
    nMinute = tm.tm_min;
}

bool tmutil::timeToTM(time32_t tTime, tagTM& tm)
{
	struct tm _tm;

#ifdef __ANDROID__
    if (localtime_r(&tTime, &_tm))
    {
        return L"";
    }
#else
    if (_localtime32_s(&_tm, &tTime))
	{
		return false;
	}
#endif

	tm = _tm;

	return true;
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

wstring tmutil::formatTime(const wstring& strFormat, time32_t tTime)
{
    if (-1 == tTime)
	{
        tTime = (time32_t)time(0);
	}

    struct tm _tm;

#ifdef __ANDROID__
    if (localtime_r(&tTime, &_tm))
    {
        return L"";
    }
#else
    if (_localtime32_s(&_tm, &tTime))
	{
		return L"";
	}
#endif

    return _formatTime(_tm, strFormat);
}

#ifndef __ANDROID__
bool tmutil::time64ToTM(time64_t time, tagTM& tm)
{
    struct tm _tm;
    if (_localtime64_s(&_tm, &time))
    {
        return false;
    }

    tm = _tm;

    return true;
}

wstring tmutil::formatTime64(const wstring& strFormat, time64_t tTime)
{
    if (-1 == tTime)
	{
        tTime = (time64_t)time(0);
	}

	tm atm;
    if (_localtime64_s(&atm, &tTime))
	{
		return L"";
	}

	return _formatTime(atm, strFormat);
}
#endif
