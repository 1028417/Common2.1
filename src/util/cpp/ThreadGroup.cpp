
#include "util.h"

void CThreadGroup::start(UINT uThreadCount, CB_WorkThread cb, bool bBlock)
{
	m_bRunSignal = true; // m_CancelEvent.reset();

    m_vecThreadStatus.assign(uThreadCount, 0);

	if (bBlock)
	{
		list<thread> lstThread;
		for (UINT uIndex = 0; uIndex < uThreadCount; uIndex++)
		{
			lstThread.emplace_back([&, uIndex]{
				cb(uIndex);
			});
		}

		for (auto& thread : lstThread)
		{
			thread.join();
		}
	}
	else
	{
		for (UINT uIndex = 0; uIndex < uThreadCount; ++uIndex)
		{
			mtutil::thread([=]{
				m_vecThreadStatus[uIndex] = 1;

				cb(uIndex);

				m_vecThreadStatus[uIndex] = 0;
			});
		}
	}
}

void CThreadGroup::pause(bool bPause)
{
	m_bPause = bPause;
}

void CThreadGroup::cancel()
{
	m_bRunSignal = false; // (void)m_CancelEvent.notify();
}

bool CThreadGroup::checkStatus()
{
	while (m_bPause)
	{
        __usleep(50);
	}

	return m_bRunSignal; // !m_CancelEvent.wait(0);
}

UINT CThreadGroup::getActiveCount()
{
	UINT uCount = 0;
	for (BOOL& bThreadStatus : m_vecThreadStatus)
	{
		if (bThreadStatus)
		{
			uCount++;
		}
	}

	return uCount;
}
