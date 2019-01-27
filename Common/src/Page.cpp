
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

BOOL CPage::Active()
{
	if (NULL == this->m_hWnd)
	{
		CMainWnd *pMainWnd = (CMainWnd*)AfxGetMainWnd();
		__EnsureReturn(pMainWnd, FALSE);

		__EnsureReturn(pMainWnd->ActivePage(*this), FALSE);
	}
	else if (!::IsWindowVisible(this->m_hWnd))
	{
		auto pParent = GetParentSheet();
		__EnsureReturn(pParent, FALSE);
		pParent->SetActivePage(this);

		//CDockView *pDockView = dynamic_cast<CDockView*>(GetParentSheet());
		//if (NULL != pDockView)
		//{
		//	pDockView->SetActivePage(*this);
		//}
	}

	(void)this->SetFocus();

	return TRUE;
}

BOOL CPage::SetTitle(const CString& cstrTitle, int iImage)
{
	CDockView *pDockView = dynamic_cast<CDockView*>(GetParentSheet());
	if (NULL != pDockView)
	{
		return pDockView->SetPageTitle(*this, cstrTitle, iImage);
	}

	CMainWnd *pMainWnd = (CMainWnd*)AfxGetMainWnd();
	__EnsureReturn(pMainWnd, FALSE);

	return pMainWnd->SetPageTitle(*this, cstrTitle, iImage);
}

int CPage::MsgBox(const CString& cstrText, UINT uType)
{
	CMainWnd *pMainWnd = (CMainWnd*)AfxGetMainWnd();
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

BOOL CPage::PreTranslateMessage(MSG* pMsg)
{
	static CPoint ptLButtonDown(0, 0);

	if (pMsg->hwnd != m_hWnd && !m_setDragableCtrls.empty() && util::ContainerFind(m_setDragableCtrls, pMsg->hwnd))
	{
		switch (pMsg->message)
		{
		case WM_LBUTTONDOWN:
			m_bDragable = true;
			ptLButtonDown = CPoint(pMsg->lParam);

			break;
		case WM_LBUTTONUP:
			m_bDragable = false;
			
			break;
		case WM_MOUSEMOVE:
			if (m_bDragable && (MK_LBUTTON & GET_FLAGS_LPARAM(pMsg->wParam)))
			{
				m_bDragable = false;

				CPoint point(pMsg->lParam);
				if (abs(point.x - ptLButtonDown.x) > 1 || abs(point.y - ptLButtonDown.y) > 1)
				{
					LPVOID pDragData = NULL;
					if (GetCtrlDragData(CWnd::FromHandle(pMsg->hwnd), point, pDragData))
					{
						(void)::SetFocus(pMsg->hwnd);

						(void)CDragDropMgr::DoDrag(pDragData);

						return TRUE;
					}
				}
			}
		
			break;
		}
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
