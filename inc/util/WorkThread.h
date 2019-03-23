
#pragma once

// CWorkThread

class CWorkThread;

struct tagWorkThreadInfo
{
	UINT uIndex = 0;
	HANDLE hHandle = INVALID_HANDLE_VALUE;
	bool bActive = false;
	CWorkThread *pThread = NULL;
};

class __UtilExt CWorkThread
{
public:
	CWorkThread()
		: m_CancelEvent(TRUE)
	{
	}

private:
	list<tagWorkThreadInfo> m_lstThreadInfos;

	bool m_bPause = false;

	NS_mtutil::CWinEvent m_CancelEvent;

public:
	BOOL Run(UINT uThreadCount=1);

	BOOL CheckCancel();

protected:
	void Pause(BOOL bPause=TRUE);

	void Cancel();

	UINT GetActiveCount();

private:
	virtual void WorkThreadProc(tagWorkThreadInfo& ThreadInfo) = 0;

	static DWORD WINAPI ThreadProc(LPVOID lpThreadParam);
};
