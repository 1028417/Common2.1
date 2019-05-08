
#pragma once

#ifdef _MSC_VER
#pragma warning(disable: 4251)
#endif

#include <stdint.h>
#include <stddef.h>

#include <Windows.h>

#ifdef __UtilPrj
#define __UtilExt __declspec(dllexport)
#else
#define __UtilExt __declspec(dllimport)
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

class __UtilExt util
{
public:
	static bool toSysTime(__time64_t time, SYSTEMTIME& sysTime);
	static void toSysTime(const tm& atm, SYSTEMTIME& sysTime);
	
	static void getCurrentTime(int& nHour, int& nMinute);
	static wstring getCurrentTime();

	static time_t FileTimeToTime_t(const FILETIME& ft);

	static wstring FormatTime(const FILETIME& fileTime, const wstring& strFormat);
	static wstring FormatTime(__time64_t time, const wstring& strFormat);

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
	
	static string WStrToStr(const wstring&str, UINT CodePage = CP_ACP);
	static wstring StrToWStr(const string&str, UINT CodePage = CP_ACP);
	
	static bool IsUTF8Str(const string& strText);

	template <class _C, class _V>
	static bool ContainerFind(_C& container, _V value)
	{
		return std::find(container.begin(), container.end(), value) != container.end();
	}

	template <class _C>
	static wstring ContainerToStr(const _C& container, const wstring& strSplitor)
	{
		wstringstream ssResult;
        for (typename _C::const_iterator it = container.begin(); ; )
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

#include "mtutil.h"

#include "fsutil.h"

#include "fsdlg.h"

#include "Path.h"

#include "timer.h"

#include "ProFile.h"

#include "SQLiteDB.h"
