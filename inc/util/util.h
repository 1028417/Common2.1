
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

#include "wsutil.h"

#include "tmutil.h"

#include "fsutil.h"

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
