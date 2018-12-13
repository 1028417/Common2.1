
#include "stdafx.h"

#include <Page.h>

#include "MainWnd.h"

#define WM_PageAsync WM_USER+1

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
		//pDockView->resizePage(*this);
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
	static BOOL bDragable = FALSE;
	static CPoint ptLButtonDown(0, 0);

	if (pMsg->hwnd != m_hWnd && !m_setDragableCtrls.empty() && util::ContainerFind(m_setDragableCtrls, pMsg->hwnd))
	{
		switch (pMsg->message)
		{
		case WM_LBUTTONDOWN:
			bDragable = TRUE;
			ptLButtonDown = CPoint(pMsg->lParam);

			break;
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
			bDragable = FALSE;
			
			break;
		case WM_MOUSEMOVE:
			if (bDragable && (MK_LBUTTON & GET_FLAGS_LPARAM(pMsg->wParam)))
			{
				CPoint point(pMsg->lParam);
				if (abs(point.x - ptLButtonDown.x) > 1 || abs(point.y - ptLButtonDown.y) > 1)
				{
					bDragable = FALSE;
					
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

void CPage::Async(const CB_Async& cb, UINT uDelayTime)
{
	if (!cb)
	{
		return;
	}

	/*CB_Async cbPrev = m_cbAsync;
	m_cbAsync = [=]()
	{
		if (cbPrev)
		{
			cbPrev();
		}

		cb();
	};*/
	
	m_cbAsync = cb;

	if (0 == uDelayTime)
	{
		this->PostMessage(WM_PageAsync);
	}
	else
	{
		thread thr([=]() {
			::Sleep(uDelayTime);
			this->PostMessage(WM_PageAsync);
		});
		thr.detach();
	}
}

void CPage::AsyncLoop(UINT uDelayTime, const CB_AsyncLoop& cb)
{
	if (0 == uDelayTime)
	{
		return;
	}

	if (!cb)
	{
		return;
	}

	m_cbAsyncLoop = cb;

	_AsyncLoop(uDelayTime);
}

void CPage::_AsyncLoop(UINT uDelayTime)
{
	Async([=]() {
		if (!m_cbAsyncLoop)
		{
			return;
		}
;
		if (!m_cbAsyncLoop())
		{
			//m_cbAsyncLoop = NULL;
			return;
		}

		_AsyncLoop(uDelayTime);
	}, uDelayTime);
}

BOOL CPage::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (WM_PageAsync == message)
	{
		if (m_cbAsync)
		{
			CB_Async cb = m_cbAsync;
			m_cbAsync = NULL;
			cb();
		}

		return TRUE;
	}

	return __super::OnWndMsg(message, wParam, lParam, pResult);
}
