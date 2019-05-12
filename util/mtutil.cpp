
#include <util.h>

void CWorkThread::Run(const CB_WorkThread& cb, UINT uThreadCount)
{
	m_bCancelEvent = false; // m_CancelEvent.reset();

    m_vecThreadStatus.assign(uThreadCount, 0);

	for (UINT uIndex = 0; uIndex < uThreadCount; ++uIndex)
	{
		mtutil::start([=]() {
            m_vecThreadStatus[uIndex] = 1;

			cb(uIndex);

            m_vecThreadStatus[uIndex] = 0;
		});
	}
}

void CWorkThread::Pause(bool bPause)
{
	m_bPause = bPause;
}

void CWorkThread::Cancel()
{
	m_bCancelEvent = true; // (void)m_CancelEvent.notify();
}

bool CWorkThread::CheckCancel()
{
	while (m_bPause)
	{
		mtutil::usleep(10);
	}

	return m_bCancelEvent; // m_CancelEvent.wait(0);
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
