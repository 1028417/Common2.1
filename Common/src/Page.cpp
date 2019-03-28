
#include "stdafx.h"

#include <Page.h>

#include "MainWnd.h"

//CPage

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
	if (NULL != pDockView)
	{
		return pDockView->SetPageTitle(*this, cstrTitle, iImage);
	}

	CMainWnd *pMainWnd = CMainWnd::getMainWnd();
	__EnsureReturn(pMainWnd, FALSE);

	return pMainWnd->SetPageTitle(*this, cstrTitle, iImage);
}

int CPage::MsgBox(const CString& cstrText, UINT uType)
{
	CMainWnd *pMainWnd = CMainWnd::getMainWnd();
	__EnsureReturn(pMainWnd, FALSE);

	return pMainWnd->MsgBox(cstrText, m_cstrTitle, uType);
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

void CPage::AsyncLoop(UINT uDelayTime, const CB_AsyncLoop& cb)
{
	m_cbAsyncLoop = cb;

	if (0 == m_idTimer)
	{
		m_idTimer = CMainApp::setTimer(uDelayTime, [=]() {
			if (!onAsyncLoop())
			{
				m_idTimer = 0;
				return false;
			}

			return true;
		});
	}
}

bool CPage::onAsyncLoop()
{
	if (m_cbAsyncLoop)
	{
		return m_cbAsyncLoop();
	}

	return false;
}
