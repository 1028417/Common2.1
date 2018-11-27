
#pragma once

// CWorkThread

class CWorkThread;

struct tagWorkThreadInfo
{
	UINT uIndex;
	HANDLE hHandle;
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
	HANDLE m_hExitEvent;

public:
	BOOL RunWorkThread(UINT uThreadCount=1);

	BOOL GetExitSignal();

protected:
	void Pause(BOOL bPause=TRUE);

	void SetExitSignal();

	void WaitForExit();

	int GetWorkThreadCount();

private:
	virtual void WorkThreadProc(tagWorkThreadInfo& ThreadInfo) = 0;

	static DWORD WINAPI ThreadProc(LPVOID lpThreadParam);
};
