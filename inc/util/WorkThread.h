
#pragma once

#include <thread>

class CWorkThread;

class __UtilExt CWorkThread
{
public:
	CWorkThread()
		: m_CancelEvent(TRUE)
	{
	}

private:
	vector<BOOL> m_vecThreadStatus;

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
	virtual void WorkThreadProc(UINT uWorkThreadIndex) = 0;

	static DWORD WINAPI ThreadProc(LPVOID lpThreadParam);
};
