
#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef _MSC_VER
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)

#else
#include <QString>
#define __QStr(wstr) QString::fromStdWString(wstr)

#ifndef QT_NO_DEBUG
#define _DEBUG
#endif
#endif

#ifdef __ANDROID__
	#define __dllexport
	#define __dllimport

#else
	#define __dllexport __declspec(dllexport)
	#define __dllimport __declspec(dllimport)
#endif

#ifdef __UtilPrj
#define __UtilExt __dllexport
#else
#define __UtilExt __dllimport
#endif

#define cauto const auto

using time32_t = long;
using time64_t = int64_t;

#ifndef UINT
using UINT = unsigned int;
#endif

#ifndef BOOL
using BOOL = int;
#endif

#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#define __Ensure(x) \
	if (!(x)) \
	{ \
		return; \
	}

#define __EnsureReturn(x, y) \
	if (!(x)) \
	{ \
		return y; \
	}

#define __EnsureContinue(x) \
	if (!(x)) \
	{ \
		continue; \
	}

#define __EnsureBreak(x) \
	if (!(x)) \
	{ \
		break; \
	}

using namespace std;

#include <string>
#include <sstream>

#include <list>
#include <vector>
#include <set>
#include <map>

#include <algorithm>

#include <functional>
using fn_voidvoid = function<void()>;

#include "sstl/sstl.h"
using namespace NS_SSTL;

struct tagTM : tm
{
	tagTM()
	{
	}

	tagTM& operator = (const tm& _tm)
	{
		memcpy(this, &_tm, sizeof(_tm));

		tm_year += 1900;     // tm_year is 1900 based
		tm_mon += 1;        // tm_mon is 0 based

		return *this;
	}
};

class __UtilExt timeutil
{
public:
	static void getCurrentTime(int& nHour, int& nMinute);

	static bool timeToTM(time32_t tTime, tagTM& tm);
	static wstring formatTime(const wstring& strFormat, time32_t tTime = -1);

#ifndef __ANDROID__
	static bool time64ToTM(time64_t tTime, tagTM& tm);
	static wstring formatTime64(const wstring& strFormat, time64_t tTime = -1);
#endif
};

class __UtilExt wstrutil
{
public:
	static const wchar_t wchSpace = L' ';

public:
	static bool checkWChar(const wstring& str);

	template <typename T>
	inline static wstring fromNum(const T& num)
	{
#ifdef __ANDROID__
		return QString::number(num).toStdWString();
#else
		return std::to_wstring(num);
#endif
	}

	static void trim(wstring& strText, wchar_t chr = ' ');
	static wstring trim_r(const wstring& strText, wchar_t chr = ' ');

	static void ltrim(wstring& strText, wchar_t chr = ' ');
	static wstring ltrim_r(const wstring& strText, wchar_t chr = ' ');

	static void rtrim(wstring& strText, wchar_t chr = ' ');
	static wstring rtrim_r(const wstring& strText, wchar_t chr = ' ');
	
	static void split(const wstring& strText, wchar_t wcSplitor, vector<wstring>& vecRet, bool bTrim=false);

	static int compareUseCNCollate(const wstring& lhs, const wstring& rhs);

	static bool matchIgnoreCase(const wstring& str1, const wstring& str2);

	//static int findIgnoreCase(const wstring& str, const wstring& strToFind);

	static void lowerCase(wstring& str);
	static wstring lowerCase_r(const wstring& str);
	
	static void upperCase(wstring& str);
	static wstring upperCase_r(const wstring& str);

	static void replace(wstring& str, wchar_t chrFind, wchar_t chrReplace);
	static void replace(wstring& str, const wstring& strFindChars, wchar_t chrReplace);

	static wstring fromUTF8(const string& str);
	static string toUTF8(const wstring& str);

	static string toStr(const wstring& str);
	static wstring fromStr(const string& str, bool bCheckUTF8=false);

	template <typename T>
	static wstring ContainerToStr(const T& container, const wstring& strSplitor)
	{
		wstringstream ssResult;
        for (typename T::const_iterator it = container.begin(); ; )
		{
			ssResult << *it;
			
			it++;
			__EnsureBreak(it != container.end());

			ssResult << strSplitor;
		}

		return ssResult.str();
	}
};

struct __UtilExt tagCNSortor
{
	bool operator()(const wstring& lhs, const wstring& rhs) const
	{
		return wstrutil::compareUseCNCollate(lhs, rhs)<0;
	}
};

struct tagFileInfo
{
	tagFileInfo()
	{
	}

	tagFileInfo(const tagFileInfo& FileInfo)
	{
		*this = FileInfo;
	}

	bool m_bDir = false;

	wstring m_strName;

	unsigned long m_uFileSize = 0;

	time64_t m_tCreateTime = 0;
	time64_t m_tModifyTime = 0;
};

#include "fsutil.h"

#include "Path.h"

#include "SQLiteDB.h"

#ifndef __ANDROID__
#include "winfsutil.h"

#include "fsdlg.h"

#include "wintimer.h"

#include "ProFile.h"
#endif

#include "mtutil.h"
