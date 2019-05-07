#pragma once

using CB_Timer = function<bool()>;

class __UtilExt timerutil
{
public:
	static UINT_PTR setTimer(UINT uElapse, const CB_Timer& cb);

	static void resetTimer(UINT_PTR idEvent, const CB_Timer& cb);

	static void killTimer(UINT_PTR idEvent);
};

class __UtilExt CTimer
{
public:
	CTimer()
	{
	}

private:
	UINT_PTR m_idTimer = 0;

private:
	void _set(const CB_Timer& cb, UINT uElapse=0);

public:
	void set(UINT uElapse, const CB_Timer& cb);

	void set(const CB_Timer& cb);

	void kill();
};
