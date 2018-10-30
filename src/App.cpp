
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

	CMainWnd *pMainWnd = m_view.init();
	__AssertReturn(NULL != pMainWnd->GetSafeHwnd(), FALSE);
	m_pMainWnd = pMainWnd;

	__AssertReturn(m_Controller.init(), FALSE);

	__AssertReturn(m_view.show(), FALSE);

	__AssertReturn(m_Controller.start(), FALSE);

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
			if (MOD_ALT == (UINT)itrHotkeyInfo->eFlag && ::GetKeyState(itrHotkeyInfo->uKey)&0x8000)
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

				if (HandleHotkey(MAKELPARAM(uFlag, itrHotkeyInfo->uKey), FALSE))
				{
					return TRUE;
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

			if(::GetKeyState(VK_CONTROL)&0x8000)
			{
				uFlag = MOD_CONTROL;
			}

			if (::GetKeyState(VK_SHIFT) & 0x8000)
			{
				uFlag |= MOD_SHIFT;
			}

			if (::GetKeyState(VK_MENU) & 0x8000)
			{
				uFlag |= MOD_ALT;
			}
			
			if (HandleHotkey(MAKELPARAM(uFlag, uKey), FALSE))
			{
				return TRUE;
			}
		}		
	}

	return __super::PreTranslateMessage(pMsg);
}

BOOL CMainApp::OnCommand(UINT uID)
{
	if (m_view.handleCommand(uID))
	{
		return TRUE;
	}

	if (m_Controller.handleCommand(uID))
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

	if (0 != HotkeyInfo.uIDMenuItem)
	{
		OnCommand(HotkeyInfo.uIDMenuItem);
		bResult = true;
	}
	else
	{
		if (m_view.handleHotkey(HotkeyInfo))
		{
			bResult = true;
		}
		else
		{
			if (m_Controller.handleHotkey(HotkeyInfo))
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

	m_view.close();

	m_Controller.stop();

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

LRESULT CMainApp::SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMainApp *pMainApp = GetMainApp();
	__EnsureReturn(pMainApp, 0);

	CWaitCursor WaitCursor;

	LRESULT lResult = pMainApp->m_Controller.handleMessage(uMsg, wParam, lParam);
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

void CMainApp::SendMessageEx(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMainApp *pMainApp = GetMainApp();
	__Ensure(pMainApp);

	CWaitCursor WaitCursor;

	(void)pMainApp->m_Controller.handleMessage(uMsg, wParam, lParam);

	for (auto pModule : pMainApp->m_vctModules)
	{
		(void)pModule->HandleMessage(uMsg, wParam, lParam);
	}
}

BOOL CMainApp::RegisterInterface(UINT uIndex, LPVOID lpInterface)
{
	if (m_mapInterfaces.find(uIndex) != m_mapInterfaces.end())
	{
		return FALSE;
	}

	m_mapInterfaces[uIndex] = lpInterface;

	return TRUE;
}

LPVOID CMainApp::GetInterface(UINT uIndex)
{
	map<UINT, LPVOID>::iterator itInterface = m_mapInterfaces.find(uIndex);

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
			if (!::RegisterHotKey(CMainApp::GetMainWnd()->GetSafeHwnd(), HotkeyInfo.lParam, (UINT)HotkeyInfo.eFlag, HotkeyInfo.uKey))
			{
				return FALSE;
			}
		}

		pMainApp->m_vctHotkeyInfos.push_back(HotkeyInfo);
	}

	return TRUE;
}
