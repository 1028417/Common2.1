// stdafx.cpp : ֻ������׼�����ļ���Դ�ļ�
// Control.pch ����ΪԤ����ͷ
// stdafx.obj ������Ԥ����������Ϣ

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
