
#include "stdafx.h"

#include <WorkThread.h>

#include <App.h>


// CWorkThread

DWORD WINAPI CWorkThread::ThreadProc(LPVOID lpThreadParam)
{
	::Sleep(10);

	tagWorkThreadInfo* pThreadInfo = (tagWorkThreadInfo*)lpThreadParam;
	__AssertReturn(pThreadInfo, 0);

	pThreadInfo->pThread->WorkThreadProc(*pThreadInfo);

	pThreadInfo->hHandle = NULL;

	return 0;
}

CWorkThread::CWorkThread()
{
	m_hExitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	ASSERT(m_hExitEvent);
}

CWorkThread::~CWorkThread()
{
	(void)::CloseHandle(m_hExitEvent);
}

BOOL CWorkThread::RunWorkThread(UINT nThreadCount)
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


	(void)::ResetEvent(m_hExitEvent);


	HANDLE hThread = NULL;

	for (UINT nIndex = 0; nIndex < nThreadCount; ++nIndex)
	{
		tagWorkThreadInfo ThreadInfo;
		::ZeroMemory(&ThreadInfo, sizeof ThreadInfo);

		ThreadInfo.nIndex = nIndex;
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

void CWorkThread::SetExitSignal()
{
	(void)::SetEvent(m_hExitEvent);
}

BOOL CWorkThread::GetExitSignal()
{
	return WAIT_OBJECT_0 == ::WaitForSingleObject(m_hExitEvent, 0);
}

void CWorkThread::WaitForExit()
{
	while (TRUE)
	{
		//∑¿÷πÀ¿À¯
		CMainApp::DoEvents();

		::Sleep(100);

		BOOL bExit = TRUE;

		for (list<tagWorkThreadInfo>::iterator itThreadInfo = m_lstThreadInfos.begin()
			; itThreadInfo != m_lstThreadInfos.end(); ++itThreadInfo)
		{
			if (itThreadInfo->hHandle)
			{
				bExit = FALSE;

				break;

				//DWORD nResult = ::WaitForSingleObject(itThreadInfo->hHandle, 100);
				//if (WAIT_OBJECT_0 == nResult || WAIT_FAILED == nResult)
			}
		}

		if (bExit)
		{
			break;
		}
	}
}

int CWorkThread::GetWorkThreadCount()
{
	int nResult = 0;

	for (list<tagWorkThreadInfo>::iterator itThreadInfo = m_lstThreadInfos.begin()
		; itThreadInfo != m_lstThreadInfos.end(); ++itThreadInfo)
	{
		if (itThreadInfo->hHandle)
		{
			nResult++;
		}
	}

	return nResult;
}
