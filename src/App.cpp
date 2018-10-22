
#include "stdafx.h"

#include <App.h>

#include "MainWnd.h"

// CMainApp

BOOL CModuleApp::InitInstance()
{
	CMainApp::GetMainApp()->AddModule(*this);

	return __super::InitInstance();
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
	m_strAppPath = fsutil::GetParentPath(pszPath);
	__AssertReturn(::SetCurrentDirectory(m_strAppPath.c_str()), FALSE);

	__AssertReturn(getView().init(), FALSE);

	__AssertReturn(getController().init(), FALSE);

	CMainWnd *pMainWnd = getView().show();
	__EnsureReturn(NULL != pMainWnd->GetSafeHwnd(), FALSE);

	__AssertReturn(getController().start(), FALSE);

	m_pMainWnd = pMainWnd;

	for (ModuleVector::iterator itModule = m_vctModules.begin(); itModule != m_vctModules.end(); ++itModule)
	{
		if (!(*itModule)->OnReady(*pMainWnd))
		{
			AfxPostQuitMessage(0);
			return FALSE;
		}
	}

	if (!pMainWnd->IsWindowVisible())
	{
		pMainWnd->Show();
	}

	return TRUE;
}

BOOL CMainApp::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->hwnd == GetMainWnd()->GetSafeHwnd())
	{
		switch (pMsg->message)
		{
		case WM_CLOSE:
			Quit();

			break;
		case WM_COMMAND:
			{				
				UINT nCode = HIWORD(pMsg->wParam);
				UINT nID = LOWORD(pMsg->wParam);

				HWND hWndCtrl = (HWND)pMsg->lParam;

				if (CN_COMMAND == nCode && !hWndCtrl)
				{
					if (OnCommand(nID))
					{
						return TRUE;
					}
				}
			}

			break;
		case WM_HOTKEY:
			(void)HandleHotkey(pMsg->lParam, TRUE);
			
			return TRUE;
		default:
			;
		}
	}
	
	if (WM_SYSKEYDOWN == pMsg->message)
	{
		for (vector<tagHotkeyInfo>::iterator itrHotkeyInfo = m_vctHotkeyInfos.begin()
			; itrHotkeyInfo != m_vctHotkeyInfos.end(); ++itrHotkeyInfo)
		{
			if (MOD_ALT == (UINT)itrHotkeyInfo->eFlag && ::GetKeyState(itrHotkeyInfo->nKey)&0x8000)
			{
				UINT uFlag = MOD_ALT;

				if (0 == (::GetKeyState(VK_CONTROL) & 0x800))
				{
					uFlag |= MOD_CONTROL;
				}

				if (0 == (::GetKeyState(VK_SHIFT) & 0x800))
				{
					uFlag |= MOD_SHIFT;
				}

				if (HandleHotkey(MAKELPARAM(uFlag, itrHotkeyInfo->nKey), FALSE))
				{
					return TRUE;
				}
			}
		}
	}

	if (WM_KEYDOWN == pMsg->message)
	{
		UINT nKey = pMsg->wParam;
				
		if (VK_CONTROL != nKey && VK_SHIFT != nKey && VK_MENU != nKey)
		{
			UINT nFlag = 0;

			if(::GetKeyState(VK_CONTROL)&0x8000)
			{
				nFlag = MOD_CONTROL;
			}

			if (::GetKeyState(VK_SHIFT) & 0x8000)
			{
				nFlag |= MOD_SHIFT;
			}

			if (::GetKeyState(VK_MENU) & 0x8000)
			{
				nFlag |= MOD_ALT;
			}
			
			if (HandleHotkey(MAKELPARAM(nFlag, nKey), FALSE))
			{
				return TRUE;
			}
		}		
	}

	return __super::PreTranslateMessage(pMsg);
}

BOOL CMainApp::OnCommand(UINT nID)
{
	if (!getView().handleCommand(nID))
	{
		if (getController().handleCommand(nID))
		{
			return TRUE;
		}
	}

	for (ModuleVector::iterator itModule=m_vctModules.begin(); itModule!=m_vctModules.end(); ++itModule)
	{
		if ((*itModule)->HandleCommand(nID))
		{
			return TRUE;
		}
	}
	
	return FALSE;
}

bool CMainApp::HandleHotkey(LPARAM lParam, bool bGlobal)
{
	static DWORD dwLastTime = 0;
	if (100 > ::GetTickCount() - dwLastTime)
	{
		DoEvents();
		return FALSE;		
	}
	dwLastTime = ::GetTickCount();

	for (auto& HotkeyInfo : m_vctHotkeyInfos)
	{
		if (HotkeyInfo.bGlobal != bGlobal)
		{
			continue;
		}

		if (HotkeyInfo.lParam == lParam)
		{
			if (HandleHotkey(HotkeyInfo))
			{
				return true;
			}
		}
	}

	return false;
}

bool CMainApp::HandleHotkey(tagHotkeyInfo &HotkeyInfo)
{
	if (HotkeyInfo.bHandling)
	{
		return false;
	}
	HotkeyInfo.bHandling = TRUE;

	bool bResult = false;

	if (0 != HotkeyInfo.nIDMenuItem)
	{
		OnCommand(HotkeyInfo.nIDMenuItem);
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

	for (vector<tagHotkeyInfo>::iterator itrHotkeyInfo = m_vctHotkeyInfos.begin()
		; itrHotkeyInfo != m_vctHotkeyInfos.end(); ++itrHotkeyInfo)
	{
		if (itrHotkeyInfo->bGlobal)
		{
			(void)::UnregisterHotKey(CMainApp::GetMainWnd()->GetSafeHwnd(), itrHotkeyInfo->lParam);
		}
	}

	getController().stop();

	CMainWnd* pMainWnd = GetMainWnd();
	if (NULL != pMainWnd)
	{
		(void)pMainWnd->DestroyWindow();
	}

	AfxPostQuitMessage(0);

	return TRUE;
}

void CMainApp::DoEvents(bool bOnce)
{
	::DoEvents();
}

BOOL CMainApp::AddModule(CModuleApp& Module)
{
	CMainApp *pMainApp = GetMainApp();
	__EnsureReturn(pMainApp, FALSE);

	__AssertReturn(!util::ContainerFind(pMainApp->m_vctModules, &Module), FALSE);

	pMainApp->m_vctModules.push_back(&Module);

	return TRUE;
}

LRESULT CMainApp::SendMessage(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	CMainApp *pMainApp = GetMainApp();
	__EnsureReturn(pMainApp, 0);

	CWaitCursor WaitCursor;

	LRESULT lResult = pMainApp->getController().handleMessage(nMsg, wParam, lParam);
	if (0 != lResult)
	{
		return lResult;
	}

	for (auto pModule : pMainApp->m_vctModules)
	{
		lResult = pModule->HandleMessage(nMsg, wParam, lParam);
		if (0 != lResult)
		{
			return lResult;
		}
	}

	return 0;
}

void CMainApp::SendMessageEx(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	CMainApp *pMainApp = GetMainApp();
	__Ensure(pMainApp);

	CWaitCursor WaitCursor;

	(void)pMainApp->getController().handleMessage(nMsg, wParam, lParam);

	for (auto pModule : pMainApp->m_vctModules)
	{
		(void)pModule->HandleMessage(nMsg, wParam, lParam);
	}
}

BOOL CMainApp::RegisterInterface(UINT nIndex, LPVOID lpInterface)
{
	if (m_mapInterfaces.find(nIndex) != m_mapInterfaces.end())
	{
		return FALSE;
	}

	m_mapInterfaces[nIndex] = lpInterface;

	return TRUE;
}

LPVOID CMainApp::GetInterface(UINT nIndex)
{
	map<UINT, LPVOID>::iterator itInterface = m_mapInterfaces.find(nIndex);

	if (itInterface == m_mapInterfaces.end())
	{
		return itInterface->second;
	}

	return NULL;
}

BOOL CMainApp::RegHotkey(const tagHotkeyInfo &HotkeyInfo)
{
	CMainApp *pMainApp = GetMainApp();
	__EnsureReturn(pMainApp, FALSE);

	if (!util::ContainerFind(pMainApp->m_vctHotkeyInfos, HotkeyInfo))
	{
		if (HotkeyInfo.bGlobal)
		{
			if (!::RegisterHotKey(CMainApp::GetMainWnd()->GetSafeHwnd(), HotkeyInfo.lParam, (UINT)HotkeyInfo.eFlag, HotkeyInfo.nKey))
			{
				return FALSE;
			}
		}

		pMainApp->m_vctHotkeyInfos.push_back(HotkeyInfo);
	}

	return TRUE;
}
