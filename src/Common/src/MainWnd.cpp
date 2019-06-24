
#include "stdafx.h"

#include <MainWnd.h>

#include <app.h>

BEGIN_MESSAGE_MAP(CMainWnd, CWnd)
	ON_WM_SIZE()
	ON_WM_MOVE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL CMainWnd::Create(tagMainWndInfo& MainWndInfo)
{
	m_WndInfo = MainWndInfo;

	LPCTSTR lpszClassName = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS
		, 0, ::GetSysColorBrush(CTLCOLOR_DLG), m_WndInfo.hIcon);
	__AssertReturn(lpszClassName, FALSE);

	DWORD dwStyle = WS_CLIPSIBLINGS | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
	if (m_WndInfo.bSizeable)
	{
		dwStyle  = dwStyle | WS_MAXIMIZEBOX | WS_THICKFRAME;
	}

	__AssertReturn(this->CreateEx(WS_EX_OVERLAPPEDWINDOW, lpszClassName, m_WndInfo.strText.c_str()
		, dwStyle, 0, 0, m_WndInfo.uWidth, m_WndInfo.uHeight, NULL, m_WndInfo.hMenu), FALSE);
	
	return TRUE;
}

void CMainWnd::show()
{
	if (0 == m_WndInfo.uWidth || 0 == m_WndInfo.uHeight)
	{
		if (!m_bFullScreen)
		{
			fixWorkArea();
		}
	}

	ShowWindow(SW_SHOW);
}

#define __XOffset 6
#define __YOffset 3

void CMainWnd::fixWorkArea()
{
	m_bFullScreen = false;
	
	CRect rtWorkArea;
	CMainApp::getWorkArea(rtWorkArea);
	
	::SetWindowPos(m_hWnd, 0, rtWorkArea.left - __XOffset, rtWorkArea.top - __YOffset
		, rtWorkArea.Width() + __XOffset * 2 + 1, rtWorkArea.Height() + __YOffset * 2, SWP_NOZORDER | SWP_NOACTIVATE);
}

void CMainWnd::fullScreen()
{
	m_bFullScreen = true;
	
	::SetWindowPos(m_hWnd, 0, -__XOffset, -__YOffset, GetSystemMetrics(SM_CXSCREEN)+ __XOffset*2+1
		, GetSystemMetrics(SM_CYSCREEN)+ __YOffset*2, SWP_NOZORDER | SWP_NOACTIVATE);
}

void CMainWnd::OnMove(int x, int y)
{
	if (0 == m_WndInfo.uWidth || 0 == m_WndInfo.uHeight)
	{
		if (!m_bFullScreen)
		{
			fixWorkArea();
		}
		//else
		//{
		//	this->fullScreen();
		//}
	}
}

void CMainWnd::OnSize(UINT nType, int cx, int cy)
{
	if (SIZE_MINIMIZED == nType)
	{
		return;
	}

	if (0 == m_WndInfo.uWidth || 0 == m_WndInfo.uHeight)
	{
		if (!m_bFullScreen)
		{
			fixWorkArea();
		}
		//else
		//{
		//	this->fullScreen();
		//}
	}

	CRect rcClient;
	this->GetClientRect(rcClient);
	if (m_cx == rcClient.right && m_cy == rcClient.bottom)
	{
		return;
	}

	m_cx = rcClient.right;
	m_cy = rcClient.bottom;

	if (m_ctlStatusBar)
	{
		m_cy -= m_uStatusBarHeight;

		m_ctlStatusBar.MoveWindow(0, m_cy, m_cx, m_uStatusBarHeight);
	}

	resizeView();
}

static bool g_bResizing = false;

void CMainWnd::resizeView(bool bManual)
{
	if (g_bResizing)
	{
		return;
	}

	g_bResizing = true;

	(void)CMainApp::GetMainApp()->DoEvents();

	g_bResizing = false;

	CRect rcViewArea(0, 0, m_cx, m_cy);	
	for (auto& pr : m_mapDockViews)
	{
		pr.second->Resize(rcViewArea, bManual);

		onViewResize(*pr.second);
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
	
	CRect rcStatusBar;
	m_ctlStatusBar.GetWindowRect(rcStatusBar);
	m_uStatusBarHeight = rcStatusBar.Height();

	return TRUE;
}

BOOL CMainWnd::SetStatusText(UINT uPart, const CString& cstrText)
{
	__AssertReturn(m_ctlStatusBar.m_hWnd, FALSE);
	
	return m_ctlStatusBar.SetText(cstrText, (int)uPart, 0);
}

CDockView* CMainWnd::CreateView(CPage& Page, const tagViewStyle& ViewStyle)
{
	__AssertReturn(!m_mapDockViews.includes(ViewStyle.eViewType), NULL);

	CDockView *pView = new CDockView(*this, ViewStyle);

	if (!pView->AddPage(Page))
	{
		delete pView;
		return NULL;
	}

	m_mapDockViews.set(ViewStyle.eViewType, pView);

	this->resizeView();

	return pView;
}

BOOL CMainWnd::AddPage(CPage& Page, E_DockViewType eViewType)
{
	return m_mapDockViews.get(eViewType, [&](CDockView *pDockView) {
		pDockView->AddPage(Page);
	});
}

BOOL CMainWnd::ActivePage(CPage& Page)
{
	return m_mapDockViews.any([&](auto& pr) {
		return pr.second->SetActivePage(Page);
	});
}

/*BOOL CMainWnd::SetPageTitle(CPage& Page, const CString& cstrTitle, int iImage)
{
	return m_mapDockViews.any([&](auto& pr) {
		return pr.second->SetPageTitle(Page, cstrTitle, iImage);
	});
}*/

/*int CMainWnd::MessageBox(const CString& cstrText, const CString& cstrTitle, UINT uType)
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

	int nResult = __super::MessageBox(cstrText, pszTitle, uType);

	return nResult;
}*/

void CMainWnd::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	if (0 != m_WndInfo.uMinWidth && 0 != m_WndInfo.uMinHeight)
	{
		lpMMI->ptMinTrackSize = { (LONG)m_WndInfo.uMinWidth, (LONG)m_WndInfo.uMinHeight };
	}

	if (0 == m_WndInfo.uWidth || 0 == m_WndInfo.uHeight)
	{
		lpMMI->ptMaxTrackSize = lpMMI->ptMaxSize = {10000,10000}; 
		//{ ::GetSystemMetrics(SM_CXFULLSCREEN) + ::GetSystemMetrics(SM_CXDLGFRAME)
		//	, ::GetSystemMetrics(SM_CYFULLSCREEN) + ::GetSystemMetrics(SM_CYCAPTION) + ::GetSystemMetrics(SM_CYDLGFRAME) };
	}
}

CDockView* CMainWnd::hittestView(const CPoint& ptPos)
{
	CRect rcPos;
	for (auto& pr : m_mapDockViews)
	{
		CDockView* pView = pr.second;

		E_DockViewType eViewType = pView->getViewStyle().eViewType;
		if (E_DockViewType::DVT_DockCenter == eViewType)
		{
			continue;
		}

		if (!pView->getViewStyle().sizeable())
		{
			continue;
		}

		pView->GetWindowRect(&rcPos);
		this->ScreenToClient(&rcPos);
		
		if ((E_DockViewType::DVT_DockLeft == eViewType && PtInRect(&CRect(rcPos.right, rcPos.top, rcPos.right+__DXView, rcPos.bottom), ptPos))
			|| (E_DockViewType::DVT_DockTop == eViewType && PtInRect(&CRect(rcPos.left, rcPos.bottom, rcPos.right, rcPos.bottom+__DXView), ptPos))
			|| (E_DockViewType::DVT_DockRight == eViewType && PtInRect(&CRect(rcPos.left-__DXView, rcPos.top, rcPos.left, rcPos.bottom), ptPos))
			|| (E_DockViewType::DVT_DockBottom == eViewType && PtInRect(&CRect(rcPos.left, rcPos.top-__DXView, rcPos.right, rcPos.top), ptPos)))
		{
			return pView;
		}
	}

	return NULL;
}

void CMainWnd::setDockSize(CDockView &wndTargetView, UINT x, UINT y)
{
	switch (wndTargetView.getViewStyle().eViewType)
	{
	case E_DockViewType::DVT_DockLeft:
		wndTargetView.setDockSize(x);

		break;
	case E_DockViewType::DVT_DockTop:
		wndTargetView.setDockSize(y);
		
		break;
	case E_DockViewType::DVT_DockRight:
		wndTargetView.setDockSize(m_cx - x);
	
		break;
	case E_DockViewType::DVT_DockBottom:
		wndTargetView.setDockSize(m_cy - y);
	
		break;
	default:
		return;
		break;
	}

	resizeView(true);
}

BOOL CMainWnd::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	static CDockView* pTargetView = NULL;

	switch (message)
	{
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	{
		CPoint ptPos(lParam);
		if (WM_MOUSEMOVE == message && pTargetView)
		{
			if (ptPos.x > 0 && ptPos.y > 0 && ptPos.x < m_cx && ptPos.y < m_cy)
			{
				setDockSize(*pTargetView, ptPos.x, ptPos.y);
			}

			break;
		}

		CDockView* pView = hittestView(ptPos);
		__EnsureBreak(pView);

		LPCWSTR pszCursorName = NULL;

		E_DockViewType eViewType = pView->getViewStyle().eViewType;
		if (E_DockViewType::DVT_DockLeft == eViewType || E_DockViewType::DVT_DockRight == eViewType)
		{
			pszCursorName = IDC_SIZEWE;
		}
		else if (E_DockViewType::DVT_DockTop == eViewType || E_DockViewType::DVT_DockBottom == eViewType)
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
	case WM_LBUTTONUP:
		ReleaseCapture();

		break;
	case WM_CAPTURECHANGED:
		pTargetView = NULL;

		break;
	case WM_NCLBUTTONDBLCLK:
		if (!m_WndInfo.bSizeable)
		{
			return TRUE;
		}

		break;
	}

	return __super::OnWndMsg(message, wParam, lParam, pResult);
}

void CMainWnd::OnDestroy()
{
	for (auto& pr : m_mapDockViews)
	{
		if (*pr.second)
		{
			pr.second->DestroyWindow();
		}
		delete pr.second;
	}
	m_mapDockViews.clear();

	__super::OnDestroy();
}
