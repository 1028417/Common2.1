
#include "stdafx.h"

#include <App.h>

#include "MainWnd.h"


// IModuleApp
IModuleApp::IModuleApp()
{
	CMainApp::AddModule(*this);
}

IModuleApp::~IModuleApp()
{
}

HINSTANCE IModuleApp::GetHInstance()
{
	return m_hInstance;
}

// CMainApp

CMainApp *CMainApp::_pMainApp = NULL;

CMainWnd *CMainApp::_pMainWnd = NULL;

ModuleVector CMainApp::m_vctModules;

map<UINT, LPVOID> CMainApp::m_mapInterfaces;

map<char, LPVOID> CMainApp::m_mapHotkeyInfos;

vector<tagHotkeyInfo> CMainApp::m_vctHotkeyInfos;

wstring CMainApp::GetAppPath()
{
	static wstring strPath;
	if (!strPath.empty())
	{
		return strPath;
	}

	TCHAR pszPath[MAX_PATH];
	(void)::GetModuleFileName(0, pszPath, MAX_PATH);

	return fsutil::GetParentPath(pszPath);
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

	__AssertReturn(::SetCurrentDirectory(this->GetAppPath().c_str()), FALSE);

	tagMainWndInfo MainWndInfo;
	_pMainWnd = OnInitMainWnd(MainWndInfo);
	__EnsureReturn(NULL != _pMainWnd, FALSE);
	__EnsureReturn(_pMainWnd->Create(MainWndInfo), FALSE);
	m_pMainWnd = _pMainWnd;

	for (ModuleVector::iterator itModule = m_vctModules.begin(); itModule != m_vctModules.end(); ++itModule)
	{
		if (!(*itModule)->OnReady(*_pMainWnd))
		{
			AfxPostQuitMessage(0);
			return FALSE;
		}
	}

	_pMainWnd->Show();

	return TRUE;
}

BOOL CMainApp::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->hwnd == _pMainWnd->GetSafeHwnd())
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
					if (HandleCommand(nID))
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
		for (ModuleVector::iterator itModule=m_vctModules.begin(); itModule!=m_vctModules.end(); ++itModule)
		{
			if ((*itModule)->HandleHotkey(HotkeyInfo))
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

	(void)_pMainWnd->DestroyWindow();

	AfxPostQuitMessage(0);

	return TRUE;
}

void CMainApp::DoEvents(bool bOnce)
{
	::DoEvents();
}

BOOL CMainApp::AddModule(IModuleApp& Module)
{
	__AssertReturn(!util::ContainerFind(m_vctModules, &Module), FALSE);

	m_vctModules.push_back(&Module);

	return TRUE;
}

LRESULT CMainApp::SendMessage(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	CWaitCursor WaitCursor;

	for (ModuleVector::iterator itModule=m_vctModules.begin(); itModule!=m_vctModules.end(); ++itModule)
	{
		LRESULT lResult = (*itModule)->HandleMessage(nMsg, wParam, lParam);
		if (lResult)
		{
			return lResult;
		}
	}

	return 0;
}

void CMainApp::SendMessageEx(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	CWaitCursor WaitCursor;

	for (ModuleVector::iterator itModule=m_vctModules.begin(); itModule!=m_vctModules.end(); ++itModule)
	{
		(void)(*itModule)->HandleMessage(nMsg, wParam, lParam);
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
	if (!util::ContainerFind(m_vctHotkeyInfos, HotkeyInfo))
	{
		if (HotkeyInfo.bGlobal)
		{
			if (!::RegisterHotKey(CMainApp::GetMainWnd()->GetSafeHwnd(), HotkeyInfo.lParam, (UINT)HotkeyInfo.eFlag, HotkeyInfo.nKey))
			{
				return FALSE;
			}
		}

		m_vctHotkeyInfos.push_back(HotkeyInfo);
	}

	return TRUE;
}
