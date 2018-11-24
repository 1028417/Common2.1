
#pragma once

#include <SDKDDKVer.h>

#include <string>
#include <sstream>

#include <list>
#include <vector>
#include <set>
#include <map>

#include <algorithm>

#include <functional>

#include <thread>
#include <future>

#include <stdlib.h>

#include <fstream>

#include <afxcmn.h>			// MFC 对 Windows 公共控件的支持

using namespace std;

#ifdef __CommonPrj
#define __CommonExt __declspec(dllexport)
#else
#define __CommonExt __declspec(dllimport)
#endif

#ifdef _DEBUG
#define __DllFile(_x) (_x + string("d.dll"))
#else
#define __DllFile(_x) (_x + string(".dll"))
#endif

enum class E_TrackMouseEvent
{
	LME_MouseMove
	, LME_MouseHover
	, LME_MouseLeave
};

using CB_TrackMouseEvent = function<void(E_TrackMouseEvent eMouseEvent, const CPoint& point)>;

#define	__BackSlant L'\\'

#define __Ensure(x) \
	if (!(x)) \
	{ \
		return; \
	}

#define __Assert(x) \
	if (!(x)) \
	{ \
		ASSERT(FALSE); \
		return; \
	}

#define __EnsureReturn(x, y) \
	if (!(x)) \
	{ \
		return y; \
	}

#define __AssertReturn(x, y) \
	if (!(x)) \
	{ \
		ASSERT(FALSE); \
		return y; \
	}


#define __EnsureContinue(x) \
	if (!(x)) \
	{ \
		continue; \
	}

#define __AssertContinue(x) \
	if (!(x)) \
	{ \
		ASSERT(FALSE); \
		continue; \
	}

#define __EnsureBreak(x) \
	if (!(x)) \
	{ \
		break; \
	}

#define __AssertBreak(x) \
	if (!(x)) \
	{ \
		ASSERT(FALSE); \
		break; \
	}

#define DECLARE_SINGLETON(_CLASS) \
	public: \
		static _CLASS& inst() \
		{ \
			static _CLASS inst; \
			return inst; \
		} \
\
	private: \
		_CLASS();

#include <util.h>

#include <App.h>

#include <MainWnd.h>

#include <DockView.h>

#include <Page.h>


#include <IDB.h>

#include <SQLiteDB.h>


#include <ProFile.h>

#include <fsutil.h>

#include <Guide.h>

#include <ListCtrl.h>

#include <TreeCtrl.h>

#include <PathCtrl.h>

#include <LogList.h>

#include <DragDrop.h>

#include <WorkThread.h>

#include <ProgressDlg.h>
