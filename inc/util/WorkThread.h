
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
	using CB_WorkThread = function<void(UINT uWorkThreadIndex)>;

	BOOL Run(const CB_WorkThread& cb, UINT uThreadCount=1);

	BOOL CheckCancel();

protected:
	void Pause(BOOL bPause=TRUE);

	void Cancel();

	UINT GetActiveCount();
};
