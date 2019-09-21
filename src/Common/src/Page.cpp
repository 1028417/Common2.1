
#include "stdafx.h"

#include "Common/Page.h"

#include "Common/MainWnd.h"

BEGIN_MESSAGE_MAP(CPage, CPropertyPage)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

HBRUSH CPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_DLG)
	{
		return (HBRUSH)::GetStockObject(WHITE_BRUSH);
	}

	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}

CPage::CPage(CResModule& resModule, UINT uIDDlgRes, const CString& cstrTitle, bool bAutoActive)
	: m_resModule(resModule)
	, m_cstrTitle(cstrTitle)
	, m_bAutoActive(bAutoActive)
{
	m_resModule.ActivateResource();

	CPropertyPage::CommonConstruct(MAKEINTRESOURCE(uIDDlgRes), 0);
}

void CPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BOOL CPage::Active(bool bForceFocus)
{
	if (NULL == this->m_hWnd)
	{
		CMainWnd *pMainWnd = CMainWnd::getMainWnd();
		__EnsureReturn(pMainWnd, FALSE);

		__EnsureReturn(pMainWnd->ActivePage(*this), FALSE);
	}
	else if (!::IsWindowVisible(this->m_hWnd))
	{
		auto pParent = GetParentSheet();
		__EnsureReturn(pParent, FALSE);
		pParent->SetActivePage(this);

		(void)this->SetFocus();
	}
	else if (bForceFocus)
	{
		(void)this->SetFocus();
	}

	return TRUE;
}

BOOL CPage::SetTitle(const CString& cstrTitle, int iImage)
{
	CDockView *pDockView = dynamic_cast<CDockView*>(GetParentSheet());
	if (NULL == pDockView)
	{
		return FALSE;
	}

	return pDockView->SetPageTitle(*this, cstrTitle, iImage);
}

void CPage::RegMenuHotkey(CWnd& wndCtrl, UINT uVkKeyCode, UINT uCmd)
{
	if (0 == uCmd)
	{
		uCmd = uVkKeyCode;
	}
	m_mapMenuHotKeys[wndCtrl.m_hWnd][uVkKeyCode] = uCmd;
}

BOOL CPage::OnSetActive()
{
	BOOL bRet = __super::OnSetActive();

	CDockView *pDockView = dynamic_cast<CDockView*>(GetParentSheet());
	if (NULL != pDockView)
	{
		pDockView->resizePage(*this);
	}

	OnActive(TRUE);

	return bRet;
}

BOOL CPage::OnKillActive()
{
	BOOL bRet = __super::OnKillActive();

	OnActive(FALSE);

	return bRet;
}

static CPoint g_ptLButtonDown;

BOOL CPage::PreTranslateMessage(MSG* pMsg)
{
	static HWND s_hWndKeyDown = NULL;

	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		s_hWndKeyDown = pMsg->hwnd;
		
		break;
	case WM_KEYUP:
	{
		__EnsureBreak(pMsg->hwnd == s_hWndKeyDown);

		map<HWND, map<UINT, UINT>>::iterator itHotKeys = m_mapMenuHotKeys.find(pMsg->hwnd);
		if (itHotKeys != m_mapMenuHotKeys.end())
		{
			WORD uVkKey = GET_KEYSTATE_LPARAM(pMsg->wParam);
			map<UINT, UINT>::iterator itHotKey = itHotKeys->second.find(uVkKey);
			if (itHotKey != itHotKeys->second.end())
			{
				if (::GetFocus() == pMsg->hwnd)
				{
					OnMenuCommand(itHotKey->second, uVkKey);

					return TRUE;
				}
			}
		}
	}

	break;
	case WM_LBUTTONDOWN:
		if (pMsg->hwnd != m_hWnd && m_setDragableCtrls.find(pMsg->hwnd) != m_setDragableCtrls.end())
		{
			m_bDragable = true;
			g_ptLButtonDown = CPoint(pMsg->lParam);
		}
		
		break;
	case WM_MOUSEMOVE:
	{
		__EnsureBreak(m_bDragable);

		m_bDragable = false;

		__EnsureBreak(MK_LBUTTON & GET_FLAGS_LPARAM(pMsg->wParam));

		CPoint point(pMsg->lParam);
		__EnsureBreak(abs(point.x - g_ptLButtonDown.x) > 1 || abs(point.y - g_ptLButtonDown.y) > 1);

		LPVOID pDragData = NULL;
		__EnsureBreak(GetCtrlDragData(pMsg->hwnd, point, pDragData));

		(void)::SetFocus(pMsg->hwnd);

		BOOL bRet = CPropertyPage::PreTranslateMessage(pMsg);

		(void)CDragDropMgr::DoDrag(pDragData);

		return bRet;
	}
	
	break;
	}
	
	return CPropertyPage::PreTranslateMessage(pMsg);
}

BOOL CPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT uCode = HIWORD(wParam);

	if (CN_COMMAND != uCode || NULL != (HWND)lParam)
	{
		return __super::OnCommand(wParam, lParam);
	}

	UINT uID = LOWORD(wParam);
	OnMenuCommand(uID);

	return TRUE;
}

