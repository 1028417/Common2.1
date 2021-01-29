
#include "stdafx.h"

#include <gdiplus.h>

#include "Common/App.h"

#include "Common/MainWnd.h"

#pragma comment(lib, "gdiplus.lib")

#pragma data_seg("Shared")
static bool volatile g_bRuning = false;
static HWND volatile g_hMainWnd = NULL;
#pragma data_seg()
#pragma comment(linker,"/section:Shared,RWS")

bool CMainApp::checkRuning()
{
	if (g_bRuning)
	{
		if (g_hMainWnd)
		{
			CMainApp::foregroundWnd(g_hMainWnd);
		}

		return true;
	}
	g_bRuning = true;

	return false;
}

static vector<CModuleApp*> g_vctModules;

static map<UINT, LPVOID> g_mapInterfaces;

static vector<tagHotkeyInfo> g_vctHotkeyInfos;

static fn_void g_fnSync;
static CCSLock g_lckAsync;

void CMainApp::_sync(const fn_void& fn)
{
	g_lckAsync.lock();

	if (g_fnSync)
	{
		auto fnPrev = g_fnSync;
		g_fnSync = [=]{
			fnPrev();

			fn();
		};
		g_lckAsync.unlock();
	}
	else
	{
		g_fnSync = fn;
		g_lckAsync.unlock();

		PostThreadMessage(WM_NULL, 0, 0);
	}
}

BOOL CMainApp::DoEvent(bool bBlock, UINT& uMsg)
{
	MSG msg;
	if (bBlock)
	{
		BOOL bRet = ::GetMessage(&msg, NULL, 0, 0);
		if (-1 == bRet)
		{
			return -1;
		}

		if (0 == bRet)
		{
			uMsg = WM_QUIT;
			(void)::PostThreadMessage(GetCurrentThreadId(), WM_QUIT, 0, 0);
			return FALSE;
		}
	}
	else
	{
		if (!::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			return FALSE;
		}

		if (WM_QUIT == msg.message)
		{
			uMsg = WM_QUIT;
			(void)::PostThreadMessage(GetCurrentThreadId(), WM_QUIT, 0, 0);
			return FALSE;
		}
	}

	uMsg = msg.message;

	if (!AfxPreTranslateMessage(&msg))
	{
		(void)::TranslateMessage(&msg);
		(void)::DispatchMessage(&msg);
	}

	return TRUE;
}

E_DoEventsResult CMainApp::DoEvents(bool bOnce)
{
	bool bFlag = false;

	UINT uMsg = 0;
	do
	{
		if (!DoEvent(false, uMsg))
		{
			if (WM_QUIT == uMsg)
			{
				return E_DoEventsResult::DER_Quit;
			}
			break;
		}

		bFlag = true;
	} while (!bOnce);

	return bFlag ? E_DoEventsResult::DER_OK : E_DoEventsResult::DER_None;
}

void CMainApp::foregroundWnd(HWND hWnd)
{
	if (IsIconic(hWnd))
	{
		::ShowWindow(hWnd, SW_RESTORE);
	}
	else
	{
		//::SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		//::SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		::SetForegroundWindow(hWnd);
	}
}

BOOL CMainApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	
	(void)AfxOleInit(); //(void)::OleInitialize(NULL);
	
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken = 0;
	(void)GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	__AssertReturn(__super::InitInstance(), FALSE);

	_run();

	(void)Gdiplus::GdiplusShutdown(gdiplusToken);

	//AfxOleTerm(); // OleUninitialize();

	return FALSE;
}

void CMainApp::_run()
{
	CMainWnd *pMainWnd = getView().show();
	g_hMainWnd = pMainWnd->GetSafeHwnd();
	if (g_hMainWnd)
	{
		m_pMainWnd = pMainWnd;

		__async([=]{
			for (auto pModule : g_vctModules)
			{
				pModule->OnReady(*pMainWnd);
			}

			getController().start();

			for (cauto HotkeyInfo : g_vctHotkeyInfos)
			{
				if (HotkeyInfo.bGlobal)
				{
					(void)_RegGlobalHotkey(g_hMainWnd, HotkeyInfo);
				}
			}
		});

		(void)__super::Run();

		for (cauto HotkeyInfo : g_vctHotkeyInfos)
		{
			if (HotkeyInfo.bGlobal)
			{
				(void)::UnregisterHotKey(g_hMainWnd, HotkeyInfo.lParam);
			}
		}
	}

	for (auto pModule : g_vctModules)
	{
		pModule->OnQuit();
	}
	
	getView().close();

	getController().stop();
}

BOOL CMainApp::PreTranslateMessage(MSG* pMsg)
{
	if (g_fnSync)
	{
		g_lckAsync.lock();
		auto fn = g_fnSync;
		g_fnSync = NULL;
		g_lckAsync.unlock();

		if (fn)
		{
			fn();
		}
			
		if (WM_NULL == pMsg->message)
		{
			return TRUE;
		}
	}

	switch (pMsg->message)
	{
	case WM_COMMAND:
		if (pMsg->hwnd == g_hMainWnd)
		{
			UINT uCode = HIWORD(pMsg->wParam);
			UINT uID = LOWORD(pMsg->wParam);

			HWND hWndCtrl = (HWND)pMsg->lParam;

			if (CN_COMMAND == uCode && !hWndCtrl)
			{
				if (OnCommand(uID))
				{
					return TRUE;
				}
			}
		}

		break;
	case WM_HOTKEY:
		if (pMsg->hwnd == g_hMainWnd)
		{
			(void)_HandleHotkey(pMsg->lParam);

			return TRUE;
		}

		break;
	case WM_SYSKEYDOWN:
		for (vector<tagHotkeyInfo>::iterator itrHotkeyInfo = g_vctHotkeyInfos.begin()
			; itrHotkeyInfo != g_vctHotkeyInfos.end(); ++itrHotkeyInfo)
		{
			if ((UINT)itrHotkeyInfo->eFlag & MOD_ALT)
			{
				if (getKeyState(itrHotkeyInfo->uKey))
				{
					UINT uFlag = MOD_ALT;
					if (getKeyState(VK_SHIFT))
					{
						uFlag |= MOD_SHIFT;
					}

					if (_HandleHotkey(MAKELPARAM(uFlag, itrHotkeyInfo->uKey)))
					{
						return TRUE;
					}
				}
			}
		}
	
		break;
	case WM_KEYUP:
	{
		WORD uVkKey = GET_KEYSTATE_LPARAM(pMsg->wParam);
		if (VK_CONTROL != uVkKey && VK_SHIFT != uVkKey && VK_MENU != uVkKey)
		{
			UINT uFlag = 0;
			if(getKeyState(VK_CONTROL))
			{
				uFlag = MOD_CONTROL;
			}

			if (getKeyState(VK_SHIFT))
			{
				uFlag |= MOD_SHIFT;
			}

			if (getKeyState(VK_MENU))
			{
				uFlag |= MOD_ALT;
			}
			
			if (_HandleHotkey(MAKELPARAM(uFlag, uVkKey)))
			{
				return TRUE;
			}
		}
	}
	break;
	}

	return __super::PreTranslateMessage(pMsg);
}

BOOL CMainApp::OnCommand(UINT uID)
{
	if (getView().handleCommand(uID))
	{
		return TRUE;
	}

	if (getController().handleCommand(uID))
	{
		return TRUE;
	}
	
	for (auto pModule : g_vctModules)
	{
		if (pModule->HandleCommand(uID))
		{
			return TRUE;
		}
	}
	
	return FALSE;
}

bool CMainApp::_HandleHotkey(LPARAM lParam)
{
	static DWORD s_dwPrevTickCount = 0;
	auto dwTickCount = ::GetTickCount();
	if (dwTickCount - s_dwPrevTickCount < 1000)
	{
		return false;	
	}
	s_dwPrevTickCount = dwTickCount;
	
	for (auto& HotkeyInfo : g_vctHotkeyInfos)
	{
		if (HotkeyInfo.lParam == lParam)
		{
			if (_HandleHotkey(HotkeyInfo))
			{
				return true;
			}
		}
	}

	return false;
}

bool CMainApp::_HandleHotkey(tagHotkeyInfo &HotkeyInfo)
{
	if (HotkeyInfo.bHandling)
	{
		return false;
	}
	HotkeyInfo.bHandling = TRUE;

	bool bResult = false;

	if (0 != HotkeyInfo.uIDMenuItem)
	{
		OnCommand(HotkeyInfo.uIDMenuItem);
		bResult = true;
	}
	else
	{
		if (getView().handleHotkey(HotkeyInfo))
		{
			bResult = true;
		}
		else
		{
			if (getController().handleHotkey(HotkeyInfo))
			{
				bResult = true;
			}
		}

		for (auto pModule : g_vctModules)
		{
			if (pModule->HandleHotkey(HotkeyInfo))
			{
				bResult = true;
			}
		}
	}

	HotkeyInfo.bHandling = FALSE;

	return bResult;
}

void CMainApp::Quit()
{
	AfxPostQuitMessage(0);
}

UINT CMainApp::removeMsg(UINT uMsg)
{
	UINT uRet = 0;

	MSG msg;
	while (::PeekMessage(&msg, NULL, uMsg, uMsg, PM_REMOVE))
	{
		uRet++;
	}

	return uRet;
}

LRESULT CMainApp::SendModuleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMainApp *pMainApp = GetMainApp();
	__EnsureReturn(pMainApp, 0);

	__waitCursor;

	LRESULT lResult = pMainApp->getController().handleModuleMessage(uMsg, wParam, lParam);
	if (0 != lResult)
	{
		return lResult;
	}

	for (auto pModule : g_vctModules)
	{
		lResult = pModule->HandleMessage(uMsg, wParam, lParam);
		if (0 != lResult)
		{
			return lResult;
		}
	}

	return 0;
}

void CMainApp::BroadcastModuleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMainApp *pMainApp = GetMainApp();
	__Ensure(pMainApp);

	__waitCursor;

	(void)pMainApp->getController().handleModuleMessage(uMsg, wParam, lParam);

	for (auto pModule : g_vctModules)
	{
		(void)pModule->HandleMessage(uMsg, wParam, lParam);
	}
}

BOOL CMainApp::RegisterInterface(UINT uIndex, LPVOID lpInterface)
{
	if (g_mapInterfaces.find(uIndex) != g_mapInterfaces.end())
	{
		return FALSE;
	}

	g_mapInterfaces[uIndex] = lpInterface;

	return TRUE;
}

LPVOID CMainApp::GetInterface(UINT uIndex)
{
	map<UINT, LPVOID>::iterator itInterface = g_mapInterfaces.find(uIndex);

	if (itInterface == g_mapInterfaces.end())
	{
		return itInterface->second;
	}

	return NULL;
}

BOOL CMainApp::RegHotkey(const tagHotkeyInfo &HotkeyInfo)
{
	g_vctHotkeyInfos.push_back(HotkeyInfo);

	if (HotkeyInfo.bGlobal)
	{
		if (NULL != g_hMainWnd)
		{
			if (!_RegGlobalHotkey(g_hMainWnd, HotkeyInfo))
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

BOOL CMainApp::_RegGlobalHotkey(HWND hWnd, const tagHotkeyInfo &HotkeyInfo)
{
	return ::RegisterHotKey(hWnd, HotkeyInfo.lParam, (UINT)HotkeyInfo.eFlag, HotkeyInfo.uKey);
}

int CMainApp::msgBox(cwstr strMsg, cwstr strTitle, UINT nType, CWnd *pWnd)
{
	wstring strText(L"    ");
	strText.append(strMsg).append(L"    ");

	/*if (strText.find('\n') == __wnpos)
	{
		int nAppend = (80 - (int)strText.size()) / 2;
		if (nAppend > 0)
		{
			strText.append(nAppend, ' ');
		}
	}*/
	
	strutil::replace(strText, L"\n", L"    \n    ");
	
	strText.append(L"\n ");

	if (NULL == pWnd)
	{
		pWnd = __app->m_pMainWnd;
	}
	return pWnd->MessageBoxW(strText.c_str(), (L" " + strTitle).c_str(), nType);
}

BOOL CModuleApp::InitInstance()
{
	g_vctModules.push_back(this);

	return __super::InitInstance();
}
