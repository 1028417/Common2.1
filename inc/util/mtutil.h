#pragma once

#pragma warning(disable: 4275)

#include <mutex>
using namespace std;

namespace NS_mtutil
{
	interface IAsyncCallback
	{
		virtual void onAsync() = 0;
	};

	class __UtilExt CAsync
	{
	public:
		static bool async(IAsyncCallback& cbAsync)
		{
			return TRUE == QueueUserWorkItem(cbQueueUserWorkItem, &cbAsync, WT_EXECUTEDEFAULT);
		}

	private:
		static DWORD WINAPI cbQueueUserWorkItem(LPVOID lpPara)
		{
			if (NULL != lpPara)
			{
				((IAsyncCallback*)lpPara)->onAsync();
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
		bool try_lock(UINT uRetryTimes=1, UINT uSleepTime = 0)
		{
			return _lock(uRetryTimes, uSleepTime);
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
			__super::wait(lock);
		}

		void notify()
		{
			std::unique_lock<mutex> lock(m_mtx);
			__super::notify_all();
		}

	private:
		mutex m_mtx;
	};
}