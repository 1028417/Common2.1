#pragma once

using CB_Timer = function<bool()>;

class __UtilExt CTimer
{
public:
	static UINT_PTR setTimer(UINT uElapse, const CB_Timer& cb);

	static void resetTimer(UINT_PTR idEvent, const CB_Timer& cb);

	static void killTimer(UINT_PTR idEvent);
};
