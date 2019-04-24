#pragma once

#ifdef _MSC_VER
#pragma warning(disable: 4275)
#endif

#include <mutex>
using namespace std;

namespace NS_mtutil
{
	template <typename T, typename R>
	void startMultiTask(ArrList<T>& alTask, UINT uThreadCount, vector<R>& vecResult
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
	void startMultiTask(ArrList<T>& alTask, UINT uThreadCount, const function<bool(UINT uTaskIdx, T&)>& cb)
	{
		vector<BOOL> vecResult;
		startMultiTask<T, BOOL>(alTask, uThreadCount, vecResult, [&](UINT uTaskIdx, T& task, BOOL&) {
			return cb(uTaskIdx, task);
		});
	}

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
			startMultiTask(alTask, uThreadCount, m_vecResult, cb);

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
	
	class __UtilExt CAsync
	{
	public:
		CAsync() {}

		bool async()
		{
			return TRUE == QueueUserWorkItem(cbQueueUserWorkItem, this, WT_EXECUTEDEFAULT);
		}

	private:
		virtual void onAsync() {};

		static DWORD WINAPI cbQueueUserWorkItem(LPVOID lpPara)
		{
			if (NULL != lpPara)
			{
				((CAsync*)lpPara)->onAsync();
			}

			return 0;
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
			return TRUE==SetEvent(m_hEvent);
		}

		bool reset()
		{
			return TRUE == ResetEvent(m_hEvent);
		}

	private:
		HANDLE m_hEvent = INVALID_HANDLE_VALUE;
	};

	class __UtilExt CCASLock
	{
	public:
		CCASLock()
		{
		}
		
	private:
		char m_lockFlag = 0;

	private:
#pragma intrinsic(_InterlockedCompareExchange8, _InterlockedExchange8)
		bool _lock(UINT uRetryTimes=0, UINT uSleepTime=0)
		{
			while (_InterlockedCompareExchange8(&m_lockFlag, 1, 0))
			{
				if (0 != uRetryTimes && 0 == --uRetryTimes)
				{
					return false;
				}

				if (0 == uSleepTime)
				{
					this_thread::yield();
				}
				else
				{
					::Sleep(uSleepTime);
				}
			}

			return true;
		}

	public:
		bool try_lock(UINT uRetryTimes=1)
		{
			return _lock(uRetryTimes, 0);
		}

		void lock(UINT uSleepTime = 0)
		{
			_lock(0, uSleepTime);
		}

		void unlock()
		{
			//(void)_InterlockedExchange8(&m_lockFlag, 0);
			m_lockFlag = 0;
		}
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
}
