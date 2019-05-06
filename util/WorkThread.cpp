
#include <util.h>

#include <WorkThread.h>

// CWorkThread

BOOL CWorkThread::Run(UINT uThreadCount)
{
	m_CancelEvent.reset();

	m_vecThreadStatus.assign(uThreadCount, FALSE);

	for (UINT uIndex = 0; uIndex < uThreadCount; ++uIndex)
	{
		NS_mtutil::startThread([=]() {
			m_vecThreadStatus[uIndex] = TRUE;

			::Sleep(10);

			this->WorkThreadProc(uIndex);

			m_vecThreadStatus[uIndex] = FALSE;
		});
	}

	return TRUE;
}

void CWorkThread::Pause(BOOL bPause)
{
	m_bPause = bPause;
}

void CWorkThread::Cancel()
{
	(void)m_CancelEvent.notify();
}

BOOL CWorkThread::CheckCancel()
{
	while (m_bPause)
	{
		Sleep(10);
	}

	return m_CancelEvent.wait(0);
}

UINT CWorkThread::GetActiveCount()
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
