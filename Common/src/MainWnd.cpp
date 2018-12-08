
#include "stdafx.h"

#include <MainWnd.h>

BEGIN_MESSAGE_MAP(CMainWnd, CWnd)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

BOOL CMainWnd::Create(tagMainWndInfo& MainWndInfo)
{
	m_WndInfo = MainWndInfo;

	LPCTSTR lpszClassName = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS
		, 0, ::GetSysColorBrush(CTLCOLOR_DLG), m_WndInfo.hIcon);
	__AssertReturn(lpszClassName, FALSE);

	DWORD dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
	//if (0 == m_WndInfo.uWidth || 0 == m_WndInfo.uHeight)
	//{
	//	dwStyle |= WS_MAXIMIZE;
	//}
	__AssertReturn(
		this->CreateEx(WS_EX_OVERLAPPEDWINDOW, lpszClassName
			, m_WndInfo.strText.c_str(), dwStyle, 0, 0
			, m_WndInfo.uWidth, m_WndInfo.uHeight, NULL, m_WndInfo.hMenu)
		, FALSE);

	//if (NULL != m_WndInfo.hIcon)
	//{
	//	this->SetIcon(m_WndInfo.hIcon, TRUE);
	//	this->SetIcon(m_WndInfo.hIcon, FALSE);
	//}

	return TRUE;
}

void CMainWnd::Show()
{
	this->CenterWindow();

	if (0 == m_WndInfo.uWidth || 0 == m_WndInfo.uHeight)
	{
		(void)this->ShowWindow(SW_SHOWMAXIMIZED);
	}
	else
	{
		(void)this->ShowWindow(SW_SHOW);
	}

	if (!m_WndInfo.bSizeable)
	{
		(void)this->ModifyStyle(WS_THICKFRAME, 0);
	}
}

void CMainWnd::OnSize(UINT nType, int cx, int cy)
{
	if (SIZE_MINIMIZED == nType)
	{
		return;
	}
	else if (SIZE_RESTORED == nType)
	{
		if (!m_WndInfo.bSizeable && ::IsWindowVisible(m_hWnd))
		{
			(void)this->ShowWindow(SW_MAXIMIZE);
			return;
		}
	}

	resizeView();
}

void CMainWnd::resizeView(bool bManual)
{
	this->GetClientRect(&m_rcViewArea);
	if (m_ctlStatusBar)
	{
		CRect rcStatusBar;
		m_ctlStatusBar.GetWindowRect(&rcStatusBar);

		m_rcViewArea.bottom -= rcStatusBar.Height();

		m_ctlStatusBar.MoveWindow(0, m_rcViewArea.bottom, m_rcViewArea.Width(), rcStatusBar.Height());
	}

	for (auto& pr : m_mapDockViews)
	{
		pr.second->Resize(m_rcViewArea, bManual);
	}
}

BOOL CMainWnd::CreateStatusBar(UINT uParts, ...)
{
	if (0 == uParts)
	{
		return FALSE;
	}

	m_vctStatusPartWidth.resize(uParts);

	va_list argList;
	va_start( argList, uParts );

	int nWidth = 0;
	for (UINT uIndex = 0; uIndex < uParts; uIndex++)
	{
		nWidth += va_arg(argList, int);

		m_vctStatusPartWidth[uIndex] = nWidth;
	}

	va_end( argList );

	if (!m_ctlStatusBar.Create(WS_VISIBLE| WS_CHILD, CRect(0,0,100,100), this, 0))
	{
		return FALSE;
	}

	m_ctlStatusBar.SetParts(uParts, &m_vctStatusPartWidth[0]);

	return TRUE;
}

BOOL CMainWnd::SetStatusText(UINT uPart, const CString& cstrText)
{
	__AssertReturn(m_ctlStatusBar.m_hWnd, FALSE)
	
	return m_ctlStatusBar.SetText(cstrText, (int)uPart, 0);
}

BOOL CMainWnd::_AddView(CDockView& View, CPage& Page)
{
	__AssertReturn(View.AddPage(Page), FALSE);

	m_mapDockViews.set(View.getViewStyle().eViewType, &View);

	this->resizeView();

	return TRUE;
}

BOOL CMainWnd::AddView(CPage& Page, const tagViewStyle& ViewStyle)
{
	__AssertReturn(!m_mapDockViews.includes(ViewStyle.eViewType), FALSE);

	CDockView *pView = new CDockView(*this, ViewStyle);

	if (!_AddView(*pView, Page))
	{
		delete pView;
		return FALSE;
	}

	return TRUE;
}

BOOL CMainWnd::AddPage(CPage& Page, E_ViewType eViewType)
{
	return m_mapDockViews.get(eViewType, [&](CDockView *pDockView) {
		pDockView->AddPage(Page);
	});
}

BOOL CMainWnd::ActivePage(CPage& Page)
{
	return m_mapDockViews.some([&](auto& pr) {
		return pr.second->ActivePage(Page);
	});
}

BOOL CMainWnd::SetPageTitle(CPage& Page, const CString& cstrTitle, int iImage)
{
	return m_mapDockViews.some([&](auto& pr) {
		return pr.second->SetPageTitle(Page, cstrTitle, iImage);
	});
}

int CMainWnd::MsgBox(const CString& cstrText, const CString& cstrTitle, UINT uType)
{
	LPCTSTR pszTitle = NULL;
	if (!cstrTitle.IsEmpty())
	{
		pszTitle = cstrTitle;
	}
	else
	{
		pszTitle = m_WndInfo.strText.c_str();
	}

	int nResult = ::MessageBox(this->GetSafeHwnd(), cstrText, pszTitle, uType);

	(void)this->EnableWindow(TRUE);

	(void)this->SetFocus();

	(void)::DoEvents();

	return nResult;
}

void CMainWnd::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	if (m_WndInfo.bSizeable)
	{
		if (0 != m_WndInfo.uMinWidth && 0 != m_WndInfo.uMinHeight)
		{
			lpMMI->ptMinTrackSize = { (LONG)m_WndInfo.uMinWidth, (LONG)m_WndInfo.uMinHeight };
		}
	}
	else
	{
		if (0 != m_WndInfo.uWidth && 0 != m_WndInfo.uHeight)
		{
			lpMMI->ptMaxSize = { (LONG)m_WndInfo.uWidth, (LONG)m_WndInfo.uHeight };
		}
	}
}

CDockView* CMainWnd::hittestView(const CPoint& ptPos)
{
	CRect rcPos;
	for (auto& pr : m_mapDockViews)
	{
		CDockView* pView = pr.second;

		E_ViewType eViewType = pView->getViewStyle().eViewType;
		if (E_ViewType::VT_DockCenter == eViewType)
		{
			continue;
		}

		if (!pView->getViewStyle().sizeable())
		{
			continue;
		}

		pView->GetWindowRect(&rcPos);
		this->ScreenToClient(&rcPos);
		
		if ((E_ViewType::VT_DockLeft == eViewType && PtInRect(&CRect(rcPos.right, rcPos.top, rcPos.right+__DXView, rcPos.bottom), ptPos))
			|| (E_ViewType::VT_DockTop == eViewType && PtInRect(&CRect(rcPos.left, rcPos.bottom, rcPos.right, rcPos.bottom+__DXView), ptPos))
			|| (E_ViewType::VT_DockRight == eViewType && PtInRect(&CRect(rcPos.left-__DXView, rcPos.top, rcPos.left, rcPos.bottom), ptPos))
			|| (E_ViewType::VT_DockBottom == eViewType && PtInRect(&CRect(rcPos.left, rcPos.top-__DXView, rcPos.right, rcPos.top), ptPos)))
		{
			return pView;
		}
	}

	return NULL;
}

void CMainWnd::ResizeView(CDockView &wndTargetView, CPoint &ptPos)
{
	CRect rcPos;
	this->GetClientRect(&rcPos);

	if (!::PtInRect(&rcPos, ptPos))
	{
		return;
	}

	switch (wndTargetView.getViewStyle().eViewType)
	{
	case E_ViewType::VT_DockLeft:
		wndTargetView.setDockSize(ptPos.x);

		break;
	case E_ViewType::VT_DockTop:
		wndTargetView.setDockSize(ptPos.y);
		
		break;
	case E_ViewType::VT_DockRight:
		wndTargetView.setDockSize(rcPos.right - ptPos.x);
		
		break;
	case E_ViewType::VT_DockBottom:
		wndTargetView.setDockSize(rcPos.bottom - ptPos.y);
		
		break;
	default:
		break;
	}
	
	(void)::DoEvents();
	resizeView(true);
}

BOOL CMainWnd::HandleResizeViewMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	static CDockView* pTargetView = NULL;

	switch (message)
	{
	case WM_CAPTURECHANGED:
		pTargetView = NULL;
		
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();

		break;
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	{
		CPoint ptPos(lParam);

		if (WM_MOUSEMOVE == message && pTargetView)
		{
			ResizeView(*pTargetView, ptPos);
			break;
		}

		CDockView* pView = hittestView(ptPos);
		__EnsureBreak(pView);

		LPCWSTR pszCursorName = NULL;
		
		E_ViewType eViewType = pView->getViewStyle().eViewType;
		if (E_ViewType::VT_DockLeft == eViewType || E_ViewType::VT_DockRight == eViewType)
		{
			pszCursorName = IDC_SIZEWE;
		}
		else if (E_ViewType::VT_DockTop == eViewType || E_ViewType::VT_DockBottom == eViewType)
		{
			pszCursorName = IDC_SIZENS;
		}
		else
		{
			break;
		}

		::SetCursor(::LoadCursor(NULL, pszCursorName));
		if (WM_LBUTTONDOWN == message)
		{
			pTargetView = pView;

			this->SetCapture();
		}
	}
	
	break;
	default:
		return FALSE;
	}
	
	return TRUE;
}

BOOL CMainWnd::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (WM_NCLBUTTONDBLCLK == message)
	{
		if (!m_WndInfo.bSizeable)
		{
			return true;
		}
	}

	if (HandleResizeViewMessage(message, wParam, lParam))
	{
		return TRUE;
	}

	return __super::OnWndMsg(message, wParam, lParam, pResult);
}
