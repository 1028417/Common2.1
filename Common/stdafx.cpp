// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// Control.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

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
