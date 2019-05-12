#pragma once

#include "util.h"

#ifdef __ANDROID__
//#define _GLIBCXX_HAS_GTHREADS
//#define _GLIBCXX_USE_C99_STDINT_TR1

//#define _GLIBCXX_USE_NANOSLEEP
#else
#include <Windows.h>
#endif

#include <thread>

#include <mutex>

#include <future>

class __UtilExt mtutil
{
#ifndef __ANDROID__
public:
	static void apcWakeup(DWORD dwThreadID, const fn_voidvoid& fn = NULL)
	{
		HANDLE hThread = OpenThread(PROCESS_ALL_ACCESS, FALSE, dwThreadID);

		QueueUserAPC(APCFunc, hThread, fn ? (ULONG_PTR)&fn : 0);
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

	template <typename T, typename R>
	static void startMultiTask(ArrList<T>& alTask, UINT uThreadCount, vector<R>& vecResult
		, const function<bool(UINT uTaskIdx, T&, R&)>& cb)
	{
		if (0 == uThreadCount)
		{
			uThreadCount = 1;
		}

		vecResult.resize(uThreadCount);

		bool bCancelFlag = false;

		vector<thread> vecThread(uThreadCount);
		for (UINT uThreadIdx = 0; uThreadIdx < vecThread.size(); uThreadIdx++)
		{
			vecThread[uThreadIdx] = thread([&, uThreadIdx]() {
				for (UINT uTaskIdx = uThreadIdx; uTaskIdx < alTask.size(); uTaskIdx += uThreadCount)
				{
					alTask.get(uTaskIdx, [&](T& task) {
						if (!cb(uTaskIdx, task, vecResult[uThreadIdx]))
						{
							bCancelFlag = true;
						}
					});
					if (bCancelFlag)
					{
						break;
					}
				}
			});
		}

		for (auto& thr : vecThread)
		{
			thr.join();
		}
	}

	template <typename T>
	static void startMultiTask(ArrList<T>& alTask, UINT uThreadCount, const function<bool(UINT uTaskIdx, T&)>& cb)
	{
		vector<BOOL> vecResult;
		startMultiTask<T, BOOL>(alTask, uThreadCount, vecResult, [&](UINT uTaskIdx, T& task, BOOL&) {
			return cb(uTaskIdx, task);
		});
	}
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
	using CB_SubTask = function<bool(UINT uTaskIdx, T&, R&)>;

	vector<R>& start(ArrList<T>& alTask, UINT uThreadCount, const CB_SubTask& cb=NULL)
	{
		mtutil::startMultiTask(alTask, uThreadCount, m_vecResult, cb);

		return m_vecResult;
	}

	vector<R>& start(ArrList<T>& alTask, UINT uThreadCount)
	{
		startMultiTask(alTask, uThreadCount, [&](UINT uTaskIdx, T& task, R& result) {
			return onTask(uTaskIdx, task, result);
		});

		return m_vecResult;
	}

private:
	virtual bool onTask(UINT uTaskIdx, T&, R&) { return false; }
};

class __UtilExt CCondVar : public condition_variable
{
public:
	CCondVar()
	{
	}

	void wait()
	{
		std::unique_lock<mutex> lock(m_mtx);
		condition_variable::wait(lock);
	}

	void notify()
	{
		std::unique_lock<mutex> lock(m_mtx);
		condition_variable::notify_all();
	}

private:
	mutex m_mtx;
};

#ifndef __ANDROID__
class __UtilExt CWinEvent
{
public:
	CWinEvent(BOOL bManualReset)
	{
		m_hEvent = ::CreateEvent(NULL, bManualReset, FALSE, NULL);
	}

	~CWinEvent()
	{
		(void)::CloseHandle(m_hEvent);
	}

	bool check()
	{
		return WAIT_OBJECT_0 == ::WaitForSingleObject(m_hEvent, 0);
	}

	bool wait(DWORD dwTimeout = INFINITE)
	{
		return WAIT_OBJECT_0 == ::WaitForSingleObject(m_hEvent, dwTimeout);
	}

	bool notify()
	{
		return TRUE == SetEvent(m_hEvent);
	}

	bool reset()
	{
		return TRUE == ResetEvent(m_hEvent);
	}

private:
	HANDLE m_hEvent = INVALID_HANDLE_VALUE;
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

public:
	void lock()
	{
		EnterCriticalSection(&m_cs);
	}

	void unlock()
	{
		LeaveCriticalSection(&m_cs);
	}

private:
	CRITICAL_SECTION m_cs;
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

class __UtilExt CWorkThread
{
public:
	CWorkThread()
		//: m_CancelEvent(TRUE)
	{
	}

private:
	vector<BOOL> m_vecThreadStatus;

	volatile bool m_bPause = false;

	bool m_bCancelEvent = false; // CWinEvent m_CancelEvent;
		
public:
	using CB_WorkThread = function<void(UINT uWorkThreadIndex)>;

	void Run(const CB_WorkThread& cb, UINT uThreadCount = 1);

	bool CheckCancel();

protected:
	void Pause(bool bPause = true);

	void Cancel();

	UINT GetActiveCount();
};
