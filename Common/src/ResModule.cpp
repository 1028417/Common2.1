
#include "stdafx.h"

#include <ModuleApp.h>

#include <App.h>

void CResModule::ActivateResource()
{
	HINSTANCE hInstance = GetHInstance();
	__Assert(hInstance);

	AfxSetResourceHandle(hInstance);
}

HICON CResModule::loadIcon(UINT uID)
{
	HINSTANCE hInstance = GetHInstance();
	__AssertReturn(hInstance, NULL);

	HICON hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(uID));
	__AssertReturn(hIcon, NULL);

	return hIcon;
}

HBITMAP CResModule::loadBitmap(UINT uID)
{
	HINSTANCE hInstance = GetHInstance();
	__AssertReturn(hInstance, NULL);

	HBITMAP hBitmap = ::LoadBitmap(hInstance, MAKEINTRESOURCE(uID));
	__AssertReturn(hBitmap, NULL);

	return hBitmap;
}

HMENU CResModule::loadMenu(UINT uID)
{
	HINSTANCE hInstance = GetHInstance();
	__AssertReturn(hInstance, NULL);

	HMENU hMenu = ::LoadMenu(hInstance, MAKEINTRESOURCE(uID));
	__AssertReturn(hMenu, NULL);

	return hMenu;
}

LPCDLGTEMPLATE CResModule::loadDialog(UINT uID)
{
	HINSTANCE hInstance = GetHInstance();
	__AssertReturn(hInstance, NULL);

	HRSRC hRes = ::FindResource(hInstance, MAKEINTRESOURCE(uID), RT_DIALOG);
	__AssertReturn(hRes, NULL);

	HGLOBAL hGlobal = ::LoadResource(hInstance, hRes);
	__AssertReturn(hGlobal, NULL);

	LPCDLGTEMPLATE lpRes = (LPCDLGTEMPLATE)LockResource(hGlobal);
	__AssertReturn(lpRes, NULL);

	return lpRes;
}

BOOL CModuleApp::InitInstance()
{
	CMainApp::AddModule(*this);

	return __super::InitInstance();
}
