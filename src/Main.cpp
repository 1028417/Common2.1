// Control.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"
#include <afxdllx.h>
#ifdef _MANAGED
#error ���Ķ� Control.cpp �е�˵����ʹ�� /clr ���б���
// ���Ҫ��������Ŀ����� /clr������ִ�����в���:
//	1. �Ƴ������� afxdllx.h �İ���
//	2. ��û��ʹ�� /clr ���ѽ���Ԥ����ͷ��
//	   ��Ŀ���һ�� .cpp �ļ������к��������ı�:
//			#include <afxwin.h>
//			#include <afxdllx.h>
#endif

static AFX_EXTENSION_MODULE ControlDLL = { NULL, NULL };

#ifdef _MANAGED
#pragma managed(push, off)
#endif

HINSTANCE g_hInstance = NULL;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	g_hInstance = hInstance;

	// ���ʹ�� lpReserved���뽫���Ƴ�
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("Control.DLL ���ڳ�ʼ��!\n");
		
		// ��չ DLL һ���Գ�ʼ��
		if (!AfxInitExtensionModule(ControlDLL, hInstance))
			return 0;

		// ���� DLL ���뵽��Դ����
		// ע��: �������չ DLL ��
		//  MFC ���� DLL (�� ActiveX �ؼ�)��ʽ���ӵ���
		//  �������� MFC Ӧ�ó������ӵ�������Ҫ
		//  �����д� DllMain ���Ƴ������������һ��
		//  �Ӵ���չ DLL �����ĵ����ĺ����С�ʹ�ô���չ DLL ��
		//  ���� DLL Ȼ��Ӧ��ʽ
		//  ���øú����Գ�ʼ������չ DLL������
		//  CDynLinkLibrary ���󲻻ḽ�ӵ�
		//  ���� DLL ����Դ���������������ص�
		//  ���⡣

		new CDynLinkLibrary(ControlDLL);

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("Control.DLL ������ֹ!\n");

		// �ڵ�����������֮ǰ��ֹ�ÿ�
		AfxTermExtensionModule(ControlDLL);
	}
	return 1;   // ȷ��
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

