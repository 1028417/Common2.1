
#include <util.h>

#include <WorkThread.h>

// CWorkThread

DWORD WINAPI CWorkThread::ThreadProc(LPVOID lpThreadParam)
{
	tagWorkThreadInfo* pThreadInfo = (tagWorkThreadInfo*)lpThreadParam;
	__EnsureReturn(pThreadInfo, 0);

	pThreadInfo->bActive = true;

	::Sleep(10);

	pThreadInfo->pThread->WorkThreadProc(*pThreadInfo);

	pThreadInfo->bActive = false;

	return 0;
}

BOOL CWorkThread::RunWorkThread(UINT uThreadCount)
{
	for (list<tagWorkThreadInfo>::iterator itThreadInfo = m_lstThreadInfos.begin()
		; itThreadInfo != m_lstThreadInfos.end(); ++itThreadInfo)
	{
		if (itThreadInfo->hHandle)
		{
			return FALSE;
		}
	}

	m_lstThreadInfos.clear();

	for (UINT uIndex = 0; uIndex < uThreadCount; ++uIndex)
	{
		tagWorkThreadInfo ThreadInfo;
		ThreadInfo.uIndex = uIndex;
		ThreadInfo.pThread = this;
		m_lstThreadInfos.push_back(ThreadInfo);
		m_lstThreadInfos.back().hHandle = ::CreateThread(NULL, 0, ThreadProc, &m_lstThreadInfos.back(), CREATE_SUSPENDED, NULL);
	}

	for (tagWorkThreadInfo& WorkThreadInfo : m_lstThreadInfos)
	{
		(void)::ResumeThread(WorkThreadInfo.hHandle);
	}

	return TRUE;
}

void CWorkThread::Pause(BOOL bPause)
{
	m_bPause = bPause;

	//for (list<tagWorkThreadInfo>::iterator itThreadInfo = m_lstThreadInfos.begin()
	//	; itThreadInfo != m_lstThreadInfos.end(); ++itThreadInfo)
	//{
	//	if (bPause)
	//	{
	//		(void)::SuspendThread(itThreadInfo->hHandle);
	//	}
	//	else
	//	{
	//		(void)::ResumeThread(itThreadInfo->hHandle);
	//	}
	//}
}

void CWorkThread::Cancel()
{
	(void)m_CancelEvent.notify();
}

BOOL CWorkThread::CheckCancelSignal()
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
	for (list<tagWorkThreadInfo>::iterator itThreadInfo = m_lstThreadInfos.begin()
		; itThreadInfo != m_lstThreadInfos.end(); ++itThreadInfo)
	{
		if (itThreadInfo->bActive)
		{
			uCount++;
		}
	}

	return uCount;
}
