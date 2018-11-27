// stdafx.cpp : 只包括标准包含文件的源文件
// Control.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

void DoEvents(bool bOnce)
{
	MSG msg;
	while(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if(msg.message==WM_QUIT)
		{
			return;
		}
		(void)::TranslateMessage(&msg);
		(void)::DispatchMessage(&msg);

		if (bOnce)
		{
			break;
		}
	}
}
