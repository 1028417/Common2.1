
#pragma once

#ifdef _MSC_VER
#pragma warning(disable: 4251)
#pragma warning(disable: 4275)
#endif

#include <stdint.h>
#include <stddef.h>

using UINT = unsigned int;
using time64_t = int64_t;

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
	
#define CP_ACP 0
	static string WStrToStr(const wstring&str, UINT CodePage = CP_ACP);
	static wstring StrToWStr(const string&str, UINT CodePage = CP_ACP);
	
	static bool IsUTF8Str(const string& strText);

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

	time_t m_tCreateTime = 0;
	time_t m_tModifyTime = 0;
};

#include "fsutil.h"

#include "Path.h"

#include "SQLiteDB.h"

#ifdef _MSC_VER
#include <Windows.h>

#include "mtutil.h"

#include "winfsutil.h"

#include "fsdlg.h"

#include "wintimer.h"
#include "wintime.h"

#include "ProFile.h"
#endif
