
#pragma once

// CWorkThread

class CWorkThread;

struct tagWorkThreadInfo
{
	UINT uIndex = 0;
	HANDLE hHandle = INVALID_HANDLE_VALUE;
	bool bActive = false;
	CWorkThread *pThread;
};

class __UtilExt CWorkThread
{
public:
	CWorkThread();
	virtual ~CWorkThread();

public:
	list<tagWorkThreadInfo> m_lstThreadInfos;

private:
	HANDLE m_hEventCancel;

public:
	BOOL RunWorkThread(UINT uThreadCount=1);

	BOOL CheckCancelSignal();

protected:
	void Pause(BOOL bPause=TRUE);

	void Cancel();

	UINT GetActiveCount();

private:
	virtual void WorkThreadProc(tagWorkThreadInfo& ThreadInfo) = 0;

	static DWORD WINAPI ThreadProc(LPVOID lpThreadParam);
};
