// stdafx.cpp : 只包括标准包含文件的源文件
// Control.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

int DoEvents(bool bOnce)
{
	bool bFlag = false;

	MSG msg;
	while(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		bFlag = true;

		(void)::TranslateMessage(&msg);
		(void)::DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
		{
			return -1;
		}

		if (bOnce)
		{
			break;
		}
	}

	return bFlag?1:0;
}
