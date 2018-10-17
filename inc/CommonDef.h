
#pragma once

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

using namespace std;

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
