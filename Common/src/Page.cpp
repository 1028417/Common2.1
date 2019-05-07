
#include "stdafx.h"

#include <Page.h>

#include "MainWnd.h"

BEGIN_MESSAGE_MAP(CPage, CPropertyPage)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CPage::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClip;
	pDC->GetClipBox(rcClip);

	pDC->FillSolidRect(rcClip, __Color_White);

	return TRUE;
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
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
	{
		UINT uVkKey = GET_KEYSTATE_LPARAM(pMsg->wParam);

		map<HWND, map<UINT, UINT>>::iterator itHotKeys = m_mapMenuHotKeys.find(pMsg->hwnd);
		if (itHotKeys != m_mapMenuHotKeys.end())
		{
			map<UINT, UINT>::iterator itHotKey = itHotKeys->second.find(uVkKey);
			if (itHotKey != itHotKeys->second.end())
			{
				OnMenuCommand(itHotKey->second, uVkKey);

				return TRUE;
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

void CPage::AsyncLoop(UINT uDelayTime, const CB_AsyncLoop& cb)
{
	m_cbAsyncLoop = cb;

	auto fn = [=]() {
		if (m_cbAsyncLoop && m_cbAsyncLoop())
		{
			return true;
		}
		
		m_idTimer = 0;
		return false;
	};
	if (0 != m_idTimer)
	{
		CTimer::resetTimer(m_idTimer, fn);
	}
	else
	{
		m_idTimer = CTimer::setTimer(uDelayTime, fn);
	}
}
