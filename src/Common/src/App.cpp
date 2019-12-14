
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

static map<UINT, LPVOID> g_mapInterfaces;

static vector<tagHotkeyInfo> g_vctHotkeyInfos;

static CB_Sync g_cbAsync;
static CCSLock g_lckAsync;

void CMainApp::_sync(const CB_Sync& cb)
{
	g_lckAsync.lock();

	CB_Sync cbPrev = g_cbAsync;
	if (cbPrev)
	{
		g_cbAsync = [=]() {
			cbPrev();

			cb();
		};

		g_lckAsync.unlock();
	}
	else
	{
		g_cbAsync = cb;

		g_lckAsync.unlock();

		CMainApp::GetMainApp()->PostThreadMessage(WM_NULL, 0, 0);
	}
}

void CMainApp::sync(const CB_Sync& cb)
{
	DWORD dwThreadID = ::GetCurrentThreadId();
	if (dwThreadID == GetMainApp()->m_nThreadID)
	{
		cb();
		return;
	}
	
	_sync([=]() {
		cb();
		
		mtutil::apcWakeup(dwThreadID);
	});
	
	::SleepEx(-1, TRUE);
}

void CMainApp::sync(UINT uDelayTime, const CB_Sync& cb)
{
	if (0 == uDelayTime)
	{
		_sync(cb);
	}
	else
	{
		_sync([=]() {
			__async(uDelayTime, cb);
		});
	}
}

void CMainApp::thread(cfn_void cb)
{
	bool bExit = false;
	std::thread thr([&]() {
		cb();

		bExit = true;

		this->PostThreadMessage(WM_NULL, 0, 0);
	});

	MSG msg;
	while (!bExit && ::GetMessage(&msg, NULL, 0, 0))
	{
		if (!AfxPreTranslateMessage(&msg))
		{
			(void)::TranslateMessage(&msg);
			(void)::DispatchMessage(&msg);
		}
	}

	thr.join();
}

E_DoEventsResult CMainApp::DoEvents(bool bOnce)
{
	bool bFlag = false;

	MSG msg;
	while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		bFlag = true;

		if (!AfxPreTranslateMessage(&msg))
		{
			(void)::TranslateMessage(&msg);
			(void)::DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			return E_DoEventsResult::DER_Quit;
		}

		if (bOnce)
		{
			break;
		}
	}

	return bFlag ? E_DoEventsResult::DER_OK : E_DoEventsResult::DER_None;
}

BOOL CMainApp::InitInstance()
{
	if (g_bRuning)
	{
		if (g_hMainWnd)
		{
			if (IsIconic(g_hMainWnd))
			{
				(void)ShowWindow(g_hMainWnd, SW_RESTORE);
			}
			else
			{
				//::SetWindowPos(g_hMainWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				::SetForegroundWindow(g_hMainWnd);
			}
		}

		return FALSE;
	}
	g_bRuning = true;


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
	__Assert(getController().init());

	CMainWnd *pMainWnd = getView().show();
	g_hMainWnd = pMainWnd->GetSafeHwnd();
	__Ensure(g_hMainWnd);
	m_pMainWnd = pMainWnd;

	for (auto& HotkeyInfo : g_vctHotkeyInfos)
	{
		if (HotkeyInfo.bGlobal)
		{
			(void)_RegGlobalHotkey(g_hMainWnd, HotkeyInfo);
		}
	}

	if (getController().start())
	{
		_run(*pMainWnd);

		getController().stop();
	}

	//for (vector<tagHotkeyInfo>::iterator itrHotkeyInfo = g_vctHotkeyInfos.begin()
	//	; itrHotkeyInfo != g_vctHotkeyInfos.end(); ++itrHotkeyInfo)
	//{
	//	if (itrHotkeyInfo->bGlobal)
	//	{
	//		(void)::UnregisterHotKey(g_hMainWnd, itrHotkeyInfo->lParam);
	//	}
	//}

	getView().close();
}

void CMainApp::_run(CMainWnd& MainWnd)
{
	for (ModuleVector::iterator itModule = m_vctModules.begin(); itModule != m_vctModules.end(); ++itModule)
	{
		if (!(*itModule)->OnReady(MainWnd))
		{
			return;
		}
	}

	(void)__super::Run();

	for (ModuleVector::iterator itModule = m_vctModules.begin(); itModule != m_vctModules.end(); ++itModule)
	{
		(void)(*itModule)->OnQuit();
	}
}

BOOL CMainApp::PreTranslateMessage(MSG* pMsg)
{
	if (g_cbAsync)
	{
		g_lckAsync.lock();
		CB_Sync cb = g_cbAsync;
		g_cbAsync = NULL;
		g_lckAsync.unlock();

		if (WM_QUIT != pMsg->message)
		{
			if (cb)
			{
				cb();
			}
			
			if (WM_NULL == pMsg->message)
			{
				return TRUE;
			}
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
	
	for (ModuleVector::iterator itModule=m_vctModules.begin(); itModule!=m_vctModules.end(); ++itModule)
	{
		if ((*itModule)->HandleCommand(uID))
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

		for (auto pModule : m_vctModules)
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

bool CMainApp::removeMsg(UINT uMsg)
{
	bool bRet = false;
	MSG msg;
	do
	{
		bRet = ::PeekMessage(&msg, NULL, uMsg, uMsg, PM_REMOVE);
	} while (bRet);

	return bRet;
}

BOOL CMainApp::AddModule(CModuleApp& Module)
{
	CMainApp *pMainApp = GetMainApp();
	__EnsureReturn(pMainApp, FALSE);

	pMainApp->m_vctModules.push_back(&Module);

	return TRUE;
}

LRESULT CMainApp::SendModuleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMainApp *pMainApp = GetMainApp();
	__EnsureReturn(pMainApp, 0);

	CWaitCursor WaitCursor;

	LRESULT lResult = pMainApp->getController().handleModuleMessage(uMsg, wParam, lParam);
	if (0 != lResult)
	{
		return lResult;
	}

	for (auto pModule : pMainApp->m_vctModules)
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

	CWaitCursor WaitCursor;

	(void)pMainApp->getController().handleModuleMessage(uMsg, wParam, lParam);

	for (auto pModule : pMainApp->m_vctModules)
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

int CMainApp::showMsg(const wstring& strMsg, const wstring& strTitle, UINT nType, CWnd *pWnd)
{
	if (NULL == pWnd)
	{
		pWnd = CMainApp::GetMainApp()->m_pMainWnd;
	}

	wstring strText(L"  ");
	strText.append(strMsg);

	int nAppend = (50 - (int)strMsg.size()) / 2;
	if (nAppend > 0)
	{
		strText.append(nAppend, ' ');
	}

	return pWnd->MessageBox(strText.c_str(), (L" " + strTitle).c_str(), nType);
}

void CMainApp::showMsg(const wstring& strMsg, class CPage& wndPage)
{
	showMsg(strMsg, (wstring)wndPage.GetTitle(), &wndPage);
}

bool CMainApp::showConfirmMsg(const wstring& strMsg, class CPage& wndPage)
{
	return showConfirmMsg(strMsg, (wstring)wndPage.GetTitle());
}
