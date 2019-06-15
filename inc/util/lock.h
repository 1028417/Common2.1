#pragma once

#include <mutex>
#include <condition_variable>

#include <chrono>

template <typename T>
class CStateVar
{
public:
    CStateVar(const T& initState)
    {
        m_state = initState;
    }

private:
    T m_state;

    condition_variable m_condvar;

    mutex m_mtx;

private:
    inline bool _wait(T& retState, int nMs=-1)
    {
        std::unique_lock<mutex> lock(m_mtx);

        if (nMs >= 0)
        {
            if (cv_status::timeout == m_condvar.wait_for(lock, std::chrono::milliseconds(nMs)))
            {
                return false;
            }
        }
        else
        {
            m_condvar.wait(lock);
        }

        retState = m_state;

        return true;
    }

    inline bool _wait(const T& waitState, const T& resetState, int nMs = -1)
    {
        std::unique_lock<mutex> lock(m_mtx);
        if (nMs >= 0)
        {
            if (cv_status::timeout == m_condvar.wait_for(lock, std::chrono::milliseconds(nMs)))
            {
                return false;
            }

            if (m_state != waitState)
            {
                return false;
            }
        }
        else
        {
            while (m_state != waitState)
            {
                m_condvar.wait(lock);
            }
        }

        m_state = resetState;

        return true;
    }

public:
    T check()
    {
        T retState = m_state;
        (void)_wait(retState, 0);
        return retState;
    }

	T wait()
	{
        T retState = m_state;
        (void)_wait(retState);
        return retState;
	}

    bool wait(UINT uMs, T& retState)
    {
        return _wait(retState, uMs);
    }

    void wait(const T& waitState)
    {
        wait(waitState, waitState);
    }

    bool wait(UINT uMs, const T& waitState)
    {
        return wait(uMs, waitState, waitState);
    }

    void wait(const T& waitState, const T& resetState)
    {
        (void)_wait(waitState, resetState);
    }

    bool wait(UINT uMs, const T& waitState, const T& resetState)
    {
        return _wait(waitState, resetState, uMs);
    }

	void set(const T& state, bool bNotifyAll=true)
    {
        std::unique_lock<mutex> lock(m_mtx);
        m_state = state;

        if (bNotifyAll)
        {
            m_condvar.notify_all();
        }
        else
        {
            m_condvar.notify_one();
        }
    }
};

class __UtilExt CCondVar : private CStateVar<bool>
{
public:
    CCondVar(bool bInitState=false)
        : CStateVar(bInitState)
	{
	}

public:
    bool check()
    {
        return CStateVar::check();
    }

    void wait(bool bReset=true)
	{
        if (bReset)
        {
            CStateVar::wait(true, false);
        }
        else
        {
            CStateVar::wait(true);
        }
    }

    bool wait(UINT uMs, bool bReset=true)
    {
        if (bReset)
        {
            return CStateVar::wait(uMs, true, false);
        }
        else
        {
            return CStateVar::wait(uMs, true);
        }
    }

	void set(bool bNotifyAll=true)
	{
        CStateVar::set(true, bNotifyAll);
	}
};

#ifndef __ANDROID__
class __UtilExt CWinEvent
{
public:
	CWinEvent(BOOL bManualReset=FALSE, BOOL bInitialState=FALSE)
	{
		m_hEvent = ::CreateEvent(NULL, bManualReset, bInitialState, NULL);
	}

	~CWinEvent()
	{
		(void)::CloseHandle(m_hEvent);
	}

private:
	HANDLE m_hEvent = INVALID_HANDLE_VALUE;

public:
	bool wait(DWORD dwTimeout = INFINITE)
	{
		return WAIT_OBJECT_0 == ::WaitForSingleObject(m_hEvent, dwTimeout);
	}

	bool check()
	{
		return wait(0);
	}

	bool set()
	{
		return TRUE == ::SetEvent(m_hEvent);
	}

	bool reset()
	{
		return TRUE == ::ResetEvent(m_hEvent);
	}
};

class __UtilExt CCSLock
{
public:
	CCSLock()
	{
		InitializeCriticalSection(&m_cs);
	}

	~CCSLock()
	{
		DeleteCriticalSection(&m_cs);
	}

private:
	CRITICAL_SECTION m_cs;

public:
	void lock()
	{
		EnterCriticalSection(&m_cs);
	}

	void unlock()
	{
		LeaveCriticalSection(&m_cs);
	}
};

class __UtilExt CCASLock
{
public:
	CCASLock()
	{
	}

private:
#ifdef _MSC_VER
	volatile char m_lockFlag = 0;

#pragma intrinsic(_InterlockedCompareExchange8, _InterlockedExchange8)
#define __CompareExchange _InterlockedCompareExchange8
#else
	volatile long m_lockFlag = 0;
#define	__CompareExchange InterlockedCompareExchange
#endif

	bool _lock(UINT uRetryTimes = 0, UINT uSleepTime = 0)
	{
		while (__CompareExchange(&m_lockFlag, 1, 0))
		{
			if (0 != uRetryTimes && 0 == --uRetryTimes)
			{
				return false;
			}

			if (0 == uSleepTime)
			{
				mtutil::yield();
			}
			else
			{
				mtutil::usleep(uSleepTime);
			}
		}

		return true;
	}

public:
	bool try_lock(UINT uRetryTimes = 1)
	{
		return _lock(uRetryTimes, 0);
	}

	void lock(UINT uSleepTime = 0)
	{
		_lock(0, uSleepTime);
	}

	void unlock()
	{
		//(void)__CompareExchange(&m_lockFlag, 0);
		m_lockFlag = 0;
	}
};
#endif
