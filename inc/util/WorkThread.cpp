
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

	pThreadInfo->bActive = true;

	return 0;
}

CWorkThread::CWorkThread()
{
	m_hEventCancel = ::CreateEvent(NULL, TRUE, FALSE, NULL);
}

CWorkThread::~CWorkThread()
{
	(void)::CloseHandle(m_hEventCancel);
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


	(void)::ResetEvent(m_hEventCancel);


	HANDLE hThread = NULL;

	for (UINT uIndex = 0; uIndex < uThreadCount; ++uIndex)
	{
		tagWorkThreadInfo ThreadInfo;
		::ZeroMemory(&ThreadInfo, sizeof ThreadInfo);

		ThreadInfo.uIndex = uIndex;
		ThreadInfo.pThread = this;

		m_lstThreadInfos.push_back(ThreadInfo);

		hThread = ::CreateThread(NULL, 0, ThreadProc, &m_lstThreadInfos.back(), CREATE_SUSPENDED, NULL);

		m_lstThreadInfos.back().hHandle = hThread;

		::Sleep(50);
	}

	this->Pause(FALSE);

	return TRUE;
}

void CWorkThread::Pause(BOOL bPause)
{
	for (list<tagWorkThreadInfo>::iterator itThreadInfo = m_lstThreadInfos.begin()
		; itThreadInfo != m_lstThreadInfos.end(); ++itThreadInfo)
	{
		if (bPause)
		{
			(void)::SuspendThread(itThreadInfo->hHandle);
		}
		else
		{
			(void)::ResumeThread(itThreadInfo->hHandle);
		}
	}
}

void CWorkThread::Cancel()
{
	(void)::SetEvent(m_hEventCancel);
}

BOOL CWorkThread::CheckCancelSignal()
{
	return WAIT_OBJECT_0 == ::WaitForSingleObject(m_hEventCancel, 0);
}

UINT CWorkThread::GetActiveCount()
{
	UINT uCount = 0;
	for (list<tagWorkThreadInfo>::iterator itThreadInfo = m_lstThreadInfos.begin()
		; itThreadInfo != m_lstThreadInfos.end(); ++itThreadInfo)
	{
		if (!itThreadInfo->bActive)
		{
			uCount++;
		}
	}

	return uCount;
}
