
#include "util.h"

void CThreadGroup::start(UINT uThreadCount, const CB_WorkThread& cb, bool bBlock)
{
	m_bCancelEvent = false; // m_CancelEvent.reset();

    m_vecThreadStatus.assign(uThreadCount, 0);

	if (bBlock)
	{
		list<thread> lstThread;
		for (UINT uIndex = 0; uIndex < uThreadCount; uIndex++)
		{
			lstThread.push_back(thread([&, uIndex]() {
				cb(uIndex);
			}));
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
			mtutil::start([=]() {
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
	m_bCancelEvent = true; // (void)m_CancelEvent.notify();
}

bool CThreadGroup::checkCancel()
{
	while (m_bPause)
	{
		mtutil::usleep(10);
	}

	return m_bCancelEvent; // m_CancelEvent.wait(0);
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
