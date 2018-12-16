
#include "stdafx.h"

#include <App.h>

#include "MainWnd.h"


#define WM_AppAsync WM_USER + 1


static map<UINT, LPVOID> g_mapInterfaces;

static vector<tagHotkeyInfo> g_vctHotkeyInfos;

static UINT g_uTimerID = 0;

struct tagTimer
{
	UINT uTimerID;
	CB_Timer cb;
	bool bDynamicallyKill = true;
};

static SMap<HWND, SMap<UINT, tagTimer>> g_mapTimer;

void __stdcall TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	g_mapTimer.del_if(hwnd, [&](auto& pr) {
		auto& mapTimer = pr.second;
		if (0 != mapTimer.del_if([&](const auto& pr) {
			const auto& timer = pr.second;
			if (timer.uTimerID == idEvent)
			{
				if (!timer.cb())
				{
					::KillTimer(hwnd, idEvent);
					return E_DelConfirm::DC_YesAbort;
				}

				return E_DelConfirm::DC_Abort;
			}

			return E_DelConfirm::DC_No;
		}))
		{
			if (!mapTimer)
			{
				return true;
			}
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

bool CMainApp::SetTimer(const CB_Timer& cb, HWND hWnd, UINT uElapse)
{
	__AssertReturn(uElapse, false);
	
	bool bExists = false;
	g_mapTimer.get(hWnd, [&](auto& mapTimer) {
		if (mapTimer.includes(uElapse))
		{
			bExists = true;
		}
	});
	__EnsureReturn(!bExists, false);


	tagTimer timer;
	timer.uTimerID = ++g_uTimerID;
	timer.cb = cb;

	if (!g_mapTimer.get(hWnd, [&](auto& mapTimer) {
		mapTimer.set(uElapse, timer);
	}))
	{
		g_mapTimer.insert(hWnd).set(uElapse, timer);
	}

	UINT_PTR idEvent = ::SetTimer(hWnd, g_uTimerID, uElapse, TimerProc);

	return true;
}

bool CMainApp::KillTimer(HWND hWnd, UINT uElapse)
{
	bool bRet = false;
	g_mapTimer.get(hWnd, [&](auto& mapTimer) {
		mapTimer.del(uElapse, [&](auto& pr) {
			::KillTimer(hWnd, pr.second.uTimerID);
			bRet = true;
		});
	});

	return bRet;
}

void CMainApp::Async(const CB_Async& cb, UINT uDelayTime)
{
	if (!cb)
	{
		return;
	}

	m_cbAsync = cb;

	if (0 == uDelayTime)
	{
		this->PostThreadMessage(WM_AppAsync, 0, 0);
	}
	else
	{
		thread thr([=]() {
			::Sleep(uDelayTime);
			this->PostThreadMessage(WM_AppAsync, 0, 0);
		});
		thr.detach();
	}
}

BOOL CMainApp::PreTranslateMessage(MSG* pMsg)
{
	if (WM_AppAsync == pMsg->message && NULL == pMsg->hwnd)
	{
		if (m_cbAsync)
		{
			CB_Async cb = m_cbAsync;
			m_cbAsync = NULL;
			cb();
		}

		return TRUE;
	}

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
			(void)HandleHotkey(pMsg->lParam, TRUE);
			
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

bool CMainApp::HandleHotkey(LPARAM lParam, bool bGlobal)
{
	static DWORD dwLastTime = 0;
	if (100 > ::GetTickCount() - dwLastTime)
	{
		return FALSE;		
	}
	dwLastTime = ::GetTickCount();

	for (auto& HotkeyInfo : g_vctHotkeyInfos)
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

	for (vector<tagHotkeyInfo>::iterator itrHotkeyInfo = g_vctHotkeyInfos.begin()
		; itrHotkeyInfo != g_vctHotkeyInfos.end(); ++itrHotkeyInfo)
	{
		if (itrHotkeyInfo->bGlobal)
		{
			(void)::UnregisterHotKey(AfxGetMainWnd()->GetSafeHwnd(), itrHotkeyInfo->lParam);
		}
	}

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

BOOL CMainApp::RegHotkey(const tagHotkeyInfo &HotkeyInfo)
{
	if (!util::ContainerFind(g_vctHotkeyInfos, HotkeyInfo))
	{
		if (HotkeyInfo.bGlobal)
		{
			if (!::RegisterHotKey(AfxGetMainWnd()->GetSafeHwnd(), HotkeyInfo.lParam, (UINT)HotkeyInfo.eFlag, HotkeyInfo.uKey))
			{
				return FALSE;
			}
		}

		g_vctHotkeyInfos.push_back(HotkeyInfo);
	}

	return TRUE;
}
