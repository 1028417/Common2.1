#pragma once

#include <mutex>
#include <condition_variable>

#include <chrono>

using mutex_lock = std::unique_lock<mutex>;

class __UtilExt CMtxLock : public mutex
{
public:
    CMtxLock() {}

public:
    void lock_to(const fn_voidvoid& cb)
    {
        mutex_lock lock(*this);

        cb();
    }

    template <typename T>
    T lock_to(const std::function<T(void)>& cb)
    {
        mutex_lock lock(*this);

        return cb();
    }
};

template <typename T>
class TSignal
{
public:
    TSignal(const T& value) :
        m_value(value)
    {
    }

private:
    mutex m_mutex;

	T m_value;

    condition_variable m_condVar;

private:
	using CB_CheckSignal = const std::function<bool(const T& value)>&;
    inline bool _wait(CB_CheckSignal cbCheck, T& value, int nMs = -1)
    {
		mutex_lock lock(m_mutex);

        if (nMs >= 0)
        {
			if (!cbCheck(m_value))
			{
                if (cv_status::timeout == m_condVar.wait_for(lock, std::chrono::milliseconds(nMs)))
                {
                    return false;
                }

				if (!cbCheck(m_value))
                {
                    return false;
                }
            }
        }
        else
        {
            while (!cbCheck(m_value))
            {
                m_condVar.wait(lock);
            }
        }

        value = m_value;

        return true;
    }

public:
    T wait(CB_CheckSignal cbCheck)
    {
        T value;
        memset(&value, 0, sizeof value);

        (void)_wait(cbCheck, value);
    
        return value;
    }

    bool wait_for(CB_CheckSignal cbCheck, UINT uMs, T& value)
    {
        return _wait(cbCheck, value, uMs);
    }

    void set(const T& value)
    {
		mutex_lock lock(m_mutex);
        
        m_value = value;
        
        m_condVar.notify_one();
    }

    void reset(const T& value)
    {
		mutex_lock lock(m_mutex);

        m_value = value;
        
        m_condVar.notify_all();
    }
};

class CSignal : private TSignal<bool>
{
public:
    CSignal(bool bInitValue)
        : TSignal(bInitValue)
    {
    }

    void wait()
    {
        TSignal::wait([](bool bValue) {
			return bValue;
		});
    }

    bool wait_for(UINT uMs)
    {
        bool bValue = false;
        if (!TSignal::wait_for([](bool bValue) {
			return bValue;
		}, uMs, bValue))
        {
            return false;
        }

        return bValue;
    }

    void set()
    {
        TSignal::set(true);
    }

    void reset()
    {
        TSignal::reset(false);
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

	bool notify()
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