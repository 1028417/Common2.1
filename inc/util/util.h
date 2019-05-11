
#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef _MSC_VER
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
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
#include <string>
#include <sstream>

#include <list>
#include <vector>
#include <set>
#include <map>

#include <algorithm>

#include <functional>
using fn_voidvoid = function<void()>;

#include <thread>
#include <future>

#include <fstream>

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

class __UtilExt util
{
public:
	static bool toTM(time64_t time, tagTM& tm);

	static void getCurrentTime(int& nHour, int& nMinute);

	static wstring formatTime(const wstring& strFormat, time64_t t_time=-1);

	static bool checkWChar(const wstring& str);

	static wstring& trim(wstring& strText, wchar_t chr = ' ');
	static wstring trim(const wstring& strText, wchar_t chr = ' ');

	static wstring& ltrim(wstring& strText, wchar_t chr = ' ');
	static wstring ltrim(const wstring& strText, wchar_t chr = ' ');

	static wstring& rtrim(wstring& strText, wchar_t chr = ' ');
	static wstring rtrim(const wstring& strText, wchar_t chr = ' ');
	
	static void SplitString(const wstring& strText, wchar_t wcSplitor, vector<wstring>& vecRet, bool bTrim=false);

	static int StrCompareUseCNCollate(const wstring& lhs, const wstring& rhs);

	static bool StrMatchIgnoreCase(const wstring& str1, const wstring& str2);

	//static int StrFindIgnoreCase(const wstring& str, const wstring& strToFind);

	static void LowerCase(wstring& str);
	static wstring StrLowerCase(const wstring& str);
	
	static void UpperCase(wstring& str);
	static wstring StrUpperCase(const wstring& str);
	
	static bool CheckUTF8(const string& str);
	static wstring UTF8ToWS(const string& str);
	static string WSToUTF8(const wstring& str);

	static string WSToAsc(const wstring& str);
	static wstring AscToWS(const string& str);

	static wstring SToWS(const string& str);

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
		return util::StrCompareUseCNCollate(lhs, rhs)<0;
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
#include <Windows.h>

#include "mtutil.h"

#include "winfsutil.h"

#include "fsdlg.h"

#include "wintimer.h"

#include "ProFile.h"
#endif
