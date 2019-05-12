
#include "stdafx.h"

#include <App.h>

#include "MainWnd.h"

#pragma comment(lib, "gdiplus.lib")

static map<UINT, LPVOID> g_mapInterfaces;

static vector<tagHotkeyInfo> g_vctHotkeyInfos;

static CB_Sync g_cbAsync;
static NS_mtutil::CCSLock g_lckAsync;

static void _async(const CB_Sync& cb)
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

void CMainApp::async(const CB_Sync& cb, UINT uDelayTime)
{
	_async([=]() {
		if (0 == uDelayTime)
		{
			cb();
		}
		else
		{
			(void)WinTimer::setTimer(uDelayTime, [=]() {
				cb();
				return false;
			});
		}
	});
}

void CMainApp::sync(const CB_Sync& cb, bool bBlock)
{
	DWORD dwThreadID = ::GetCurrentThreadId();
	if (dwThreadID == GetMainApp()->m_nThreadID)
	{
		cb();
		return;
	}

	async([=]() {
		cb();

		if (bBlock)
		{
			NS_mtutil::CThread::apcWakeup(dwThreadID);
		}
	});

	if (bBlock)
	{
		::SleepEx(-1, TRUE);
	}
}

void CMainApp::thread(const fn_voidvoid& cb)
{
	bool bExit = false;
	NS_mtutil::CThread::Start([&]() {
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

static void _init()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);

	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	(void)AfxOleInit();
	//(void)::OleInitialize(NULL);

	AfxEnableControlContainer();

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken = 0;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	srand(GetTickCount());
}

BOOL CMainApp::InitInstance()
{
	extern void InitMinDump();
	InitMinDump();
	
	__AssertReturn(__super::InitInstance(), FALSE);

	_init();

	TCHAR pszPath[MAX_PATH];
	(void)::GetModuleFileName(0, pszPath, MAX_PATH);
	m_strAppPath = fsutil::GetParentDir(pszPath);
	__AssertReturn(::SetCurrentDirectory(m_strAppPath.c_str()), FALSE);
	
	__AssertReturn(getController().init(), FALSE);

	CMainWnd *pMainWnd = getView().init();
	HWND hwndMain = pMainWnd->GetSafeHwnd();
	__EnsureReturn(NULL != hwndMain, FALSE);
	m_pMainWnd = pMainWnd;

	for (auto& HotkeyInfo : g_vctHotkeyInfos)
	{
		if (HotkeyInfo.bGlobal)
		{
			(void)_RegGlobalHotkey(hwndMain, HotkeyInfo);
		}
	}

	__AssertReturn(getController().start(), FALSE);

	for (ModuleVector::iterator itModule = m_vctModules.begin(); itModule != m_vctModules.end(); ++itModule)
	{
		if (!(*itModule)->OnReady(*pMainWnd))
		{
			return FALSE;
		}
	}

	__super::Run();

	for (ModuleVector::iterator itModule = m_vctModules.begin(); itModule != m_vctModules.end(); ++itModule)
	{
		__EnsureReturn((*itModule)->OnQuit(), FALSE);
	}

	//for (vector<tagHotkeyInfo>::iterator itrHotkeyInfo = g_vctHotkeyInfos.begin()
	//	; itrHotkeyInfo != g_vctHotkeyInfos.end(); ++itrHotkeyInfo)
	//{
	//	if (itrHotkeyInfo->bGlobal)
	//	{
	//		(void)::UnregisterHotKey(AfxGetMainWnd()->GetSafeHwnd(), itrHotkeyInfo->lParam);
	//	}
	//}

	getView().close();

	getController().stop();

	if (NULL != m_pMainWnd)
	{
		(void)m_pMainWnd->DestroyWindow();
		delete m_pMainWnd;
		m_pMainWnd = NULL;
	}

	return FALSE;
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
		if (pMsg->hwnd == AfxGetMainWnd()->GetSafeHwnd())
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
		if (pMsg->hwnd == AfxGetMainWnd()->GetSafeHwnd())
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
	case WM_KEYDOWN:
	{
		auto uKey = GET_KEYSTATE_LPARAM(pMsg->wParam);
		if (VK_CONTROL != uKey && VK_SHIFT != uKey && VK_MENU != uKey)
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
			
			if (_HandleHotkey(MAKELPARAM(uFlag, uKey)))
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
	static DWORD dwLastTime = 0;
	if (100 > ::GetTickCount() - dwLastTime)
	{
		return FALSE;		
	}
	dwLastTime = ::GetTickCount();

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

	LRESULT lResult = pMainApp->getController().handleMessage(uMsg, wParam, lParam);
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

	(void)pMainApp->getController().handleMessage(uMsg, wParam, lParam);

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
		auto hwndMain = AfxGetMainWnd()->GetSafeHwnd();
		if (NULL != hwndMain)
		{
			if (!_RegGlobalHotkey(hwndMain, HotkeyInfo))
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
