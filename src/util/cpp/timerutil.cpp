
#include "util.h"

#if !__winvc
#include <QTimer>

#else
struct tagTimerInfo
{
	bool bPending = false;

	fn_bool cb;
};
static thread_local map<UINT_PTR, tagTimerInfo> g_mapTimer;

void CALLBACK TimerProc(HWND, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    (void)uMsg;
    (void)dwTime;

	auto itr = g_mapTimer.find(idEvent);
	if (itr != g_mapTimer.end())
	{
		tagTimerInfo& TimerInfo = itr->second;
		if (TimerInfo.bPending)
        {
			return;
		}

        TimerInfo.bPending = true;
        if (!TimerInfo.cb())
        {
            timerutil::killTimer(idEvent);
            return;
        }
    }

    g_mapTimer[idEvent].bPending = false;
}
#endif

__TimerID timerutil::_setTimer(UINT uElapse, cfn_bool cb)
{
#if __winvc
    UINT idEvent = ::SetTimer(NULL, 0, uElapse, TimerProc);

    auto& TimerInfo = g_mapTimer[idEvent];
    TimerInfo.bPending = false;
    TimerInfo.cb = cb;

    return idEvent;
#else

    QTimer::singleShot(uElapse, [=]() {
        if (cb())
        {
            _setTimer(uElapse, cb);
        }
    });
#endif
}

void timerutil::async(UINT uElapse, cfn_void cb)
{
    (void)_setTimer(uElapse, [cb]() {
        cb();
        return false;
    });
}

__TimerID timerutil::setTimer(UINT uElapse, cfn_void cb)
{
	return _setTimer(uElapse, [cb]() {
		cb();
		return true;
	});
}

__TimerID timerutil::setTimerEx(UINT uElapse, cfn_bool cb)
{
	return _setTimer(uElapse, [cb]() {
		return cb();
	});
}

#if __winvc
void timerutil::killTimer(UINT_PTR idEvent)
{
	::KillTimer(NULL, idEvent);

    (void)g_mapTimer.erase(idEvent);
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

void CWinTimer::_set(cfn_bool cb, UINT uElapse)
{
	m_cb = cb;

	if (0 == uElapse || uElapse == m_uElapse)
	{
		return;
	}

	kill();

	m_idTimer = timerutil::setTimerEx(uElapse, [=]() {
		return _onTimer();
	});
	m_uElapse = uElapse;
}

void CWinTimer::set(UINT uElapse, cfn_bool cb)
{
	_set(cb, uElapse);
}

void CWinTimer::set(cfn_bool cb)
{
	_set(cb);
}

void CWinTimer::kill()
{
	if (0 != m_idTimer)
	{
		timerutil::killTimer(m_idTimer);
		m_idTimer = 0;
	}
	m_uElapse = 0;
}
#endif
