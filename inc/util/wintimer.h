#pragma once

class __UtilExt WinTimer
{
public:
	using CB_Timer = function<bool()>;
	static UINT_PTR setTimer(UINT uElapse, const CB_Timer& cb);

	static void resetTimer(UINT_PTR idEvent, const CB_Timer& cb);

	static void killTimer(UINT_PTR idEvent);

public:
	WinTimer()
	{
	}

	~WinTimer()
	{
		kill();
	}

private:
	UINT m_uElapse = 0;

	UINT_PTR m_idTimer = 0;
	
	CB_Timer m_cb;

private:
	bool _onTimer();
		
	void _set(const CB_Timer& cb, UINT uElapse=0);

public:
	void set(UINT uElapse, const CB_Timer& cb);

	void set(const CB_Timer& cb);

	void kill();
};
