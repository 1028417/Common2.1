
#include "stdafx.h"

#include <App.h>

#include "MainWnd.h"

static map<UINT, LPVOID> g_mapInterfaces;

static vector<tagHotkeyInfo> g_vctHotkeyInfos;

static UINT g_uTimerID = 0;

static SMap<UINT, CB_Timer> g_mapTimer;

void __stdcall TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	g_mapTimer.del_if(idEvent, [&](auto& pr) {
		if (!pr.second())
		{
			::KillTimer(hwnd, idEvent);
			return true;
		}
		
		return false;
	});
}

BOOL CMainApp::InitInstance()
{
	__AssertReturn(__super::InitInstance(), FALSE);

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);

	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	(void)AfxOleInit();
	//(void)::OleInitialize(NULL);

	AfxEnableControlContainer();

	srand(GetTickCount());

	TCHAR pszPath[MAX_PATH];
	(void)::GetModuleFileName(0, pszPath, MAX_PATH);
	m_strAppPath = fsutil::GetParentDir(pszPath);
	__AssertReturn(::SetCurrentDirectory(m_strAppPath.c_str()), FALSE);

	CMainWnd *pMainWnd = getView().init();
	__EnsureReturn(NULL != pMainWnd->GetSafeHwnd(), FALSE);
	m_pMainWnd = pMainWnd;

	__AssertReturn(getController().init(), FALSE);

	__AssertReturn(getView().show(), FALSE);

	__AssertReturn(getController().start(), FALSE);

	for (ModuleVector::iterator itModule = m_vctModules.begin(); itModule != m_vctModules.end(); ++itModule)
	{
		if (!(*itModule)->OnReady(*pMainWnd))
		{
			return FALSE;
		}
	}

	if (!pMainWnd->IsWindowVisible())
	{
		pMainWnd->ShowWindow(SW_SHOW);
	}

	return TRUE;
}

UINT_PTR CMainApp::setTimer(UINT uElapse, const CB_Timer& cb)
{
	UINT_PTR idEvent = ::SetTimer(NULL, ++g_uTimerID, uElapse, TimerProc);
	g_mapTimer.insert(idEvent, cb);
	return idEvent;
}

void CMainApp::killTimer(UINT_PTR idEvent)
{
	g_mapTimer.del(idEvent);
	::KillTimer(NULL, idEvent);
}

BOOL CMainApp::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->hwnd == AfxGetMainWnd()->GetSafeHwnd())
	{
		switch (pMsg->message)
		{
		case WM_COMMAND:
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
			(void)_HandleHotkey(pMsg->lParam);
			
			return TRUE;
		default:
			;
		}
	}
	
	if (WM_SYSKEYDOWN == pMsg->message)
	{
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
	}

	if (WM_KEYDOWN == pMsg->message)
	{
		UINT uKey = pMsg->wParam;
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

BOOL CMainApp::Quit()
{
	for (ModuleVector::iterator itModule=m_vctModules.begin(); itModule!=m_vctModules.end(); ++itModule)
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
	}

	AfxPostQuitMessage(0);

	return TRUE;
}

E_DoEventsResult CMainApp::DoEvents(bool bOnce)
{
	int iRet = ::DoEvents(bOnce);
	if (-1 == iRet)
	{
		return E_DoEventsResult::DER_Quit;
	}

	return iRet ? E_DoEventsResult::DER_OK : E_DoEventsResult::DER_None;
}

bool CMainApp::peekMsg(UINT uMsg, MSG msg, bool bPeekAll)
{
	bool bRet = false;
	do
	{
		bRet = ::PeekMessage(&msg, NULL, uMsg, uMsg, PM_REMOVE);
	} while (bPeekAll && bRet);

	return bRet;
}

BOOL CMainApp::AddModule(CModuleApp& Module)
{
	CMainApp *pMainApp = GetMainApp();
	__EnsureReturn(pMainApp, FALSE);

	__AssertReturn(!util::ContainerFind(pMainApp->m_vctModules, &Module), FALSE);

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

BOOL CMainApp::RegHotkey(const tagHotkeyInfo &HotkeyInfo, bool bGlobal)
{
	if (!util::ContainerFind(g_vctHotkeyInfos, HotkeyInfo))
	{
		g_vctHotkeyInfos.push_back(HotkeyInfo);

		if (bGlobal)
		{
			if (!::RegisterHotKey(AfxGetMainWnd()->GetSafeHwnd(), HotkeyInfo.lParam, (UINT)HotkeyInfo.eFlag, HotkeyInfo.uKey))
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}
