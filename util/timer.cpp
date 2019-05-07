
#include "util.h"

#include "timer.h"

struct tagTimerInfo
{
	bool bPending = false;

	CB_Timer cb;
};

static map<UINT, tagTimerInfo> g_mapTimer;
static NS_mtutil::CCSLock g_lckTimer;

void __stdcall TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	CB_Timer cb;
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
			timerutil::killTimer(idEvent);
			return;
		}
	}

	g_lckTimer.lock();
	g_mapTimer[idEvent].bPending = false;
	g_lckTimer.unlock();
}

UINT_PTR timerutil::setTimer(UINT uElapse, const CB_Timer& cb)
{
	UINT_PTR idEvent = ::SetTimer(NULL, 0, uElapse, TimerProc);

	resetTimer(idEvent, cb);

	return idEvent;
}

void timerutil::resetTimer(UINT_PTR idEvent, const CB_Timer& cb)
{
	g_lckTimer.lock();
	auto& TimerInfo = g_mapTimer[idEvent];
	TimerInfo.bPending = false;
	TimerInfo.cb = cb;
	g_lckTimer.unlock();
}

void timerutil::killTimer(UINT_PTR idEvent)
{
	::KillTimer(NULL, idEvent);

	g_lckTimer.lock();
	(void)g_mapTimer.erase(idEvent);
	g_lckTimer.unlock();
}

bool CTimer::_onTimer()
{
	if (!m_cb())
	{
		m_idTimer = 0;
		return false;
	}

	return true;
}

void CTimer::_set(const CB_Timer& cb, UINT uElapse)
{
	m_cb = cb;

	if (0 == uElapse)
	{
		return;
	}

	if (0 == m_idTimer || uElapse != m_uElapse)
	{
		m_uElapse = uElapse;

		auto fn = [=]() {
			return _onTimer();
		};

		if (0 != m_idTimer)
		{
			timerutil::resetTimer(m_idTimer, fn);
		}
		else
		{
			m_idTimer = timerutil::setTimer(uElapse, fn);
		}
	}
}

void CTimer::set(UINT uElapse, const CB_Timer& cb)
{
	_set(cb, uElapse);
}

void CTimer::set(const CB_Timer& cb)
{
	_set(cb);
}

void CTimer::kill()
{
	if (0 != m_idTimer)
	{
		timerutil::killTimer(m_idTimer);
		m_idTimer = 0;
	}
}
