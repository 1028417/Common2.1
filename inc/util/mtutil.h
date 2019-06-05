#pragma once

#include "util.h"

#ifndef __ANDROID__
#include <Windows.h>
#endif

#include <mutex>
#include <condition_variable>

#include <thread>

//#include <future>

class __UtilExt mtutil
{
#ifndef __ANDROID__
public:
	static bool apcWakeup(HANDLE hThread, const fn_voidvoid& fn = NULL)
	{
		return 0 != QueueUserAPC(APCFunc, hThread, fn ? (ULONG_PTR)&fn : 0);
	}

	static bool apcWakeup(DWORD dwThreadID, const fn_voidvoid& fn = NULL)
	{
		HANDLE hThread = OpenThread(PROCESS_ALL_ACCESS, FALSE, dwThreadID);
		return apcWakeup(hThread, fn);
	}

	static bool poolStart(const fn_voidvoid& fn)
	{
		return TRUE == QueueUserWorkItem(UserWorkItemProc, fn ? (PVOID)&fn : 0, WT_EXECUTEDEFAULT);
	}

private:
	static VOID WINAPI APCFunc(ULONG_PTR dwParam)
	{
		auto  pfn = (const fn_voidvoid *)dwParam;
		if (pfn && *pfn)
		{
			(*pfn)();
		}
	}

	static DWORD WINAPI UserWorkItemProc(LPVOID lpPara)
	{
		auto  pfn = (const fn_voidvoid *)lpPara;
		if (pfn && *pfn)
		{
			(*pfn)();
		}

		return 0;
	}
#endif

public:
	template <typename CB>
	static void start(const CB& cb)
    {
#ifdef __ANDROID__
        std::thread(cb).detach();
#else
        std::thread(cb).detach();
#endif
    }

	inline static void usleep(UINT uMS = 0)
	{
		std::this_thread::sleep_for(chrono::milliseconds(uMS));
	}

	inline static void yield()
    {
        std::this_thread::yield();
    }
};

class __UtilExt CThreadGroup
{
public:
	CThreadGroup()
		//: m_CancelEvent(TRUE)
	{
	}

private:
	vector<BOOL> m_vecThreadStatus;

	volatile bool m_bPause = false;

	bool m_bCancelEvent = false; // CWinEvent m_CancelEvent;

public:
	using CB_WorkThread = function<void(UINT uThreadIndex)>;
	void start(UINT uThreadCount, const CB_WorkThread& cb, bool bBlock);

	bool checkCancel();

protected:
	void pause(bool bPause = true);

	void cancel();

	UINT getActiveCount();
};

template <typename T, typename R>
class CMultiTask
{
public:
	CMultiTask()
	{
	}

private:
	vector<R> m_vecResult;

public:
	using CB_SubTask = const function<bool(UINT uTaskIdx, T&, R&)>&;
	static void start(ArrList<T>& alTask, vector<R>& vecResult, UINT uThreadCount, CB_SubTask cb)
	{
		if (0 == uThreadCount)
		{
			uThreadCount = 1;
		}

		vecResult.resize(uThreadCount);

		CThreadGroup ThreadGroup;
		ThreadGroup.start(uThreadCount, [&](UINT uThreadIndex) {
			bool bCancelFlag = false;
			for (UINT uTaskIdx = uThreadIndex; uTaskIdx < alTask.size(); uTaskIdx += uThreadCount)
			{
				alTask.get(uTaskIdx, [&](T& task) {
					if (!cb(uTaskIdx, task, vecResult[uThreadIndex]))
					{
						bCancelFlag = true;
					}
				});
				if (bCancelFlag)
				{
					break;
				}
			}
		}, true);
	}

	static void start(ArrList<T>& alTask, UINT uThreadCount, const function<bool(UINT uTaskIdx, T&)>& cb)
	{
		vector<R> vecResult;
		start(alTask, vecResult, uThreadCount, [&](UINT uTaskIdx, T& task, R&) {
			return cb(uTaskIdx, task);
		});
	}

	vector<R>& start(ArrList<T>& alTask, UINT uThreadCount, CB_SubTask cb)
	{
		start(alTask, m_vecResult, uThreadCount, [&](UINT uTaskIdx, T& task, R& result) {
			return cb(uTaskIdx, task, result);
		});

		return m_vecResult;
	}
};

class __UtilExt CCondVar
{
public:
	CCondVar(bool bInitialState=false)
		: m_bState(bInitialState)
	{
	}

private:
	condition_variable m_condvar;

	mutex m_mtx;
	
	bool m_bState = false;

public:
	void wait()
	{
		std::unique_lock<mutex> lock(m_mtx);
		if (!m_bState)
		{
=			m_condvar.wait(lock);
		}
		m_bState = false;
	}

	void set()
	{
		std::unique_lock<mutex> lock(m_mtx);
		m_bState = true;
		m_condvar.notify_one();
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
