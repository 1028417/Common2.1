
#ifndef __ANDROID__

#include "util.h"

#include <Windows.h>

struct tagTimerInfo
{
	bool bPending = false;

	CB_WinTimer cb;
};

static map<UINT, tagTimerInfo> g_mapTimer;
static CCASLock g_lckTimer;

void CALLBACK TimerProc(HWND, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    (void)uMsg;
    (void)dwTime;

	CB_WinTimer cb;
	g_lckTimer.lock();
	auto itr = g_mapTimer.find(idEvent);
	if (itr != g_mapTimer.end())
	{
		tagTimerInfo& TimerInfo = itr->second;
		if (TimerInfo.bPending)
		{
			g_lckTimer.unlock();
			return;
		}

		TimerInfo.bPending = true;
		cb = TimerInfo.cb;
	}
	g_lckTimer.unlock();

	if (cb)
	{
		if (!cb())
		{
			wintimer::killTimer(idEvent);
			return;
		}
	}

	g_lckTimer.lock();
	g_mapTimer[idEvent].bPending = false;
	g_lckTimer.unlock();
}

UINT_PTR wintimer::setTimer(UINT uElapse, const CB_WinTimer& cb)
{
	UINT_PTR idEvent = ::SetTimer(NULL, 0, uElapse, TimerProc);
	
	g_lckTimer.lock();
	auto& TimerInfo = g_mapTimer[idEvent];
	TimerInfo.bPending = false;
	TimerInfo.cb = cb;
	g_lckTimer.unlock();

	return idEvent;
}

void wintimer::killTimer(UINT_PTR idEvent)
{
	::KillTimer(NULL, idEvent);

	g_lckTimer.lock();
	(void)g_mapTimer.erase(idEvent);
	g_lckTimer.unlock();
}

bool CWinTimer::_onTimer()
{
	if (!m_cb())
	{
		m_idTimer = 0;
		m_uElapse = 0;
		return false;
	}

	return true;
}

void CWinTimer::_set(const CB_WinTimer& cb, UINT uElapse)
{
	m_cb = cb;

	if (0 == uElapse || uElapse == m_uElapse)
	{
		return;
	}

	kill();

	m_idTimer = wintimer::setTimer(uElapse, [=]() {
		return _onTimer();
	});
	m_uElapse = uElapse;
}

void CWinTimer::set(UINT uElapse, const CB_WinTimer& cb)
{
	_set(cb, uElapse);
}

void CWinTimer::set(const CB_WinTimer& cb)
{
	_set(cb);
}

void CWinTimer::kill()
{
	if (0 != m_idTimer)
	{
		wintimer::killTimer(m_idTimer);
		m_idTimer = 0;
	}
	m_uElapse = 0;
}

#endif
