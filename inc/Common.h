
#pragma once

#include <SDKDDKVer.h>

#include <afxcmn.h>			// MFC 对 Windows 公共控件的支持

#include "util/util.h"

#ifdef __CommonPrj
#define __CommonExt __dllexport
#else
#define __CommonExt __dllimport
#endif

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

#define __Assert(x) \
	if (!(x)) \
	{ \
		ASSERT(FALSE); \
		return; \
	}

#define __AssertReturn(x, y) \
	if (!(x)) \
	{ \
		ASSERT(FALSE); \
		return y; \
	}

#define __AssertContinue(x) \
	if (!(x)) \
	{ \
		ASSERT(FALSE); \
		continue; \
	}

#define __AssertBreak(x) \
	if (!(x)) \
	{ \
		ASSERT(FALSE); \
		break; \
	}

enum class E_TrackMouseEvent
{
	LME_MouseMove
	, LME_MouseHover
	, LME_MouseLeave
};

using CB_TrackMouseEvent = function<void(E_TrackMouseEvent, const CPoint&)>;

#include "App.h"

#include "MainWnd.h"

#include "DockView.h"

#include "Page.h"

#include "Guard.h"

#include "TreeCtrl.h"

#include "ListCtrl.h"

#include "PathCtrl.h"

#include "DragDrop.h"

#include "ProgressDlg.h"

#include "DialogT.h"
