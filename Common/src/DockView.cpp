
#include "stdafx.h"

#include <DockView.h>

BOOL CTabCtrlEx::init(const tagViewTabStyle& TabStyle)
{
	SetTabStyle(TabStyle.eTabStyle);

	if (0 != TabStyle.uTabFontSize)
	{
		__EnsureReturn(SetFontSize(TabStyle.uTabFontSize), FALSE);
	}

	if (NULL != TabStyle.pImglst)
	{
		SetImageList(TabStyle.pImglst);
	}
	else if (0 != TabStyle.uTabHeight)
	{
		if (0 != TabStyle.uTabWidth)
		{
			SetItemSize(CSize(TabStyle.uTabWidth, TabStyle.uTabHeight));
		}
		else
		{
			__EnsureReturn(SetTabHeight(TabStyle.uTabHeight), FALSE);
		}
	}

	return TRUE;
}

void CTabCtrlEx::SetTabStyle(E_ViewTabStyle eTabStyle)
{
	DWORD dwTabStyle = TCS_FOCUSNEVER;

	switch (m_eTabStyle = eTabStyle)
	{
	case E_ViewTabStyle::VTS_HideTab:
		ShowWindow(SW_HIDE);
		break;
	case E_ViewTabStyle::VTS_TabBottom:
		dwTabStyle |= TCS_BOTTOM;
		break;
	case E_ViewTabStyle::VTS_TabLeft:
		dwTabStyle |= TCS_VERTICAL | TCS_MULTILINE | TCS_FIXEDWIDTH | TCS_FORCELABELLEFT;
		break;
	case E_ViewTabStyle::VTS_TabRight:
		dwTabStyle |= TCS_RIGHT | TCS_VERTICAL | TCS_MULTILINE | TCS_FIXEDWIDTH | TCS_FORCELABELLEFT;
		break;
	}
	
	ModifyStyle(0, dwTabStyle);
}

BOOL CTabCtrlEx::SetFontSize(UINT uFontSize)
{
	__EnsureReturn(0 != uFontSize, FALSE);
	
	return m_fontGuide.setFontSize(*this, uFontSize);
}

BOOL CTabCtrlEx::SetTabHeight(UINT uTabHeight)
{
	__EnsureReturn(0 != uTabHeight, FALSE);

	auto pImglst = GetImageList();
	if (NULL != pImglst)
	{
		if (pImglst != &m_Imglst)
		{
			return FALSE;
		}
	}

	if (m_Imglst)
	{
		__EnsureReturn(m_Imglst.DeleteImageList(), FALSE);
	}

	UINT cx = 1;
	UINT cy = 1;
	if (E_ViewTabStyle::VTS_TabTop == m_eTabStyle || E_ViewTabStyle::VTS_TabBottom == m_eTabStyle)
	{
		cy = uTabHeight;
	}
	else if (E_ViewTabStyle::VTS_TabLeft == m_eTabStyle || E_ViewTabStyle::VTS_TabRight == m_eTabStyle)
	{
		cx = uTabHeight;
	}
	else
	{
		return FALSE;
	}

	__EnsureReturn(m_Imglst.Create(cx, cy, ILC_COLOR, 0, 0), FALSE);
	SetImageList(&m_Imglst);
	
	return TRUE;
}

void CTabCtrlEx::OnTrackMouseEvent(E_TrackMouseEvent eMouseEvent, const CPoint& point)
{
	m_iTrackMouseFlag = 0;

	if (m_cbMouseEvent)
	{
		m_cbMouseEvent(eMouseEvent, point);
	}
}

void CTabCtrlEx::SetTrackMouse(const CB_TrackMouseEvent& cbMouseEvent)
{
	m_cbMouseEvent = cbMouseEvent;

	m_iTrackMouseFlag = 0;
}

BOOL CTabCtrlEx::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	switch (message)
	{
	case WM_MOUSEMOVE:
		if (0 == m_iTrackMouseFlag)
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.hwndTrack = m_hWnd;
			tme.dwFlags = TME_LEAVE | TME_HOVER;
			tme.dwHoverTime = HOVER_DEFAULT;
			m_iTrackMouseFlag = ::TrackMouseEvent(&tme);
		}

		OnTrackMouseEvent(E_TrackMouseEvent::LME_MouseMove, CPoint(lParam));

		break;
	case WM_MOUSELEAVE:
		m_iTrackMouseFlag = 0;

		OnTrackMouseEvent(E_TrackMouseEvent::LME_MouseLeave, CPoint(lParam));

		break;
	case WM_MOUSEHOVER:
	{
		//m_iTrackMouseFlag = 0;

		OnTrackMouseEvent(E_TrackMouseEvent::LME_MouseHover, CPoint(lParam));
	}

	break;
	}

	return __super::OnWndMsg(message, wParam, lParam, pResult);
}


//CDockView

CDockView::CDockView(CWnd& wndParent)
	: CPropertySheet(L"", &wndParent)
{
}

CDockView::CDockView(CWnd& wndParent, const tagViewStyle& ViewStyle)
	: CPropertySheet(L"", &wndParent)
{
	setViewStyle(ViewStyle);
}

BEGIN_MESSAGE_MAP(CDockView, CPropertySheet)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CDockView::setViewStyle(const tagViewStyle& ViewStyle)
{
	m_ViewStyle = ViewStyle;

	m_ViewStyle.uMaxDockSize = max(m_ViewStyle.uMaxDockSize, m_ViewStyle.uDockSize);

	if (0 == m_ViewStyle.uMinDockSize)
	{
		m_ViewStyle.uMinDockSize = m_ViewStyle.uDockSize;
	}
	else
	{
		m_ViewStyle.uMinDockSize = min(m_ViewStyle.uMinDockSize, m_ViewStyle.uDockSize);
	}
}

BOOL CDockView::Create()
{
	DWORD dwStyle = WS_CHILD;

	if (E_ViewTabStyle::VTS_HideTab == m_ViewStyle.TabStyle.eTabStyle)
	{
		dwStyle |= WS_BORDER;
	}

	__AssertReturn(__super::Create(m_pParentWnd, dwStyle, WS_EX_CONTROLPARENT), FALSE);

	__AssertReturn(m_wndTabCtrl.SubclassWindow(this->GetTabControl()->GetSafeHwnd()), FALSE);
	
	__AssertReturn(m_wndTabCtrl.init(m_ViewStyle.TabStyle), FALSE);

	return TRUE;
}

BOOL CDockView::AddPage(CPage& Page)
{
	__AssertReturn(!util::ContainerFind(m_vctPages, &Page), FALSE);

	m_vctPages.push_back(&Page);

	__super::AddPage(&Page);

	if (NULL == m_hWnd)
	{
		__AssertReturn(this->Create(), FALSE);
		this->ShowWindow(SW_SHOWNOACTIVATE);
	}
	
	if (E_ViewTabStyle::VTS_HideTab != m_ViewStyle.TabStyle.eTabStyle)
	{
		TCITEM tci = {0};
		tci.mask = TCIF_TEXT;
		tci.pszText = (LPTSTR)(LPCTSTR)Page.m_cstrTitle;
		(void)m_wndTabCtrl.SetItem(m_wndTabCtrl.GetItemCount()-1, &tci);
	}

	if (Page.m_bAutoActive)
	{
		m_wndTabCtrl.SetTrackMouse([&](E_TrackMouseEvent eMouseEvent, const CPoint& point) {
			__Ensure(E_TrackMouseEvent::LME_MouseHover == eMouseEvent);

			tagTCHITTESTINFO htInfo;
			htInfo.pt = point;
			htInfo.flags = TCHT_ONITEM;
			int iItem = m_wndTabCtrl.HitTest(&htInfo);

			__Ensure(iItem >= 0);
			__Ensure(iItem < (int)m_vctPages.size());
			__Ensure(m_vctPages[iItem]->m_bAutoActive);

			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		});
	}

	return TRUE;
}

BOOL CDockView::ActivePage(CPage& Page)
{
	int nActivePage = __super::GetActiveIndex();

	PageVector::iterator itPage = std::find(m_vctPages.begin(), m_vctPages.end(), &Page);
	__EnsureReturn(itPage != m_vctPages.end(), FALSE);

	if (itPage - m_vctPages.begin() != nActivePage)
	{
		(void)__super::SetActivePage(*itPage);
	}

	this->resizePage();

	return TRUE;
}

BOOL CDockView::SetPageTitle(CPage& Page, const CString& cstrTitle, int iImage)
{
	PageVector::iterator itPage = std::find(m_vctPages.begin(), m_vctPages.end(),& Page);
	__EnsureReturn(itPage != m_vctPages.end(), FALSE);

	int nPage = (int)(itPage - m_vctPages.begin());
	
	__AssertReturn(m_wndTabCtrl.GetItemCount() > nPage, TRUE);

	Page.m_cstrTitle = cstrTitle;

	TCITEM tci;
	tci.mask = TCIF_TEXT;
	//if (-1 != iImage)
	{
		tci.mask |= TCIF_IMAGE;
		tci.iImage = iImage;
	}
	tci.pszText = (LPTSTR)(LPCTSTR)Page.m_cstrTitle;
	(void)m_wndTabCtrl.SetItem(nPage, &tci);

	return TRUE;
}


static bool g_bManualResize = false;

void CDockView::Resize(CRect& rcViewArea, bool bManual)
{
	UINT uOffset = 0;
	if (0 != m_ViewStyle.uMaxDockSize)
	{
		uOffset = __DXView;
	}
	else
	{
		uOffset = 2;
	}
	
	CRect rtPos(rcViewArea);
	switch (m_ViewStyle.eViewType)
	{
	case E_ViewType::VT_DockLeft:
		rtPos.SetRect(0, rcViewArea.top + m_ViewStyle.uStartPos, m_ViewStyle.uDockSize
			, m_ViewStyle.uEndPos ? rcViewArea.top + m_ViewStyle.uEndPos : rcViewArea.bottom);
		rcViewArea.left += m_ViewStyle.uDockSize + uOffset;
		
		break;
	case E_ViewType::VT_DockTop:
		rtPos.SetRect(rcViewArea.left + m_ViewStyle.uStartPos, 0
			, m_ViewStyle.uEndPos ? rcViewArea.left + m_ViewStyle.uEndPos : rcViewArea.right, m_ViewStyle.uDockSize);
		rcViewArea.top += m_ViewStyle.uDockSize + uOffset;

		break;
	case E_ViewType::VT_DockRight:
		rtPos.SetRect(rcViewArea.right - m_ViewStyle.uDockSize, rcViewArea.top + m_ViewStyle.uStartPos
			, rcViewArea.right, m_ViewStyle.uStartPos ? rcViewArea.top + m_ViewStyle.uStartPos : rcViewArea.bottom);
		rcViewArea.right -= m_ViewStyle.uDockSize + uOffset;

		break;
	case E_ViewType::VT_DockBottom:
		rtPos.SetRect(rcViewArea.left + m_ViewStyle.uStartPos, rcViewArea.bottom - m_ViewStyle.uDockSize
			, m_ViewStyle.uEndPos ? rcViewArea.left + m_ViewStyle.uStartPos : rcViewArea.right, rcViewArea.bottom);
		rcViewArea.bottom -= m_ViewStyle.uDockSize + uOffset;

		break;
	}

	g_bManualResize = bManual;
	(void)this->MoveWindow(&rtPos);
	g_bManualResize = false;
}

void CDockView::OnSize(UINT nType, int, int)
{
	__Ensure(SIZE_MINIMIZED != nType);

	__Ensure(m_wndTabCtrl);

	this->resizePage();
}

void CDockView::resizePage()
{
	CRect rcClient;
	this->GetClientRect(&rcClient);

	int nPage = __super::GetActiveIndex();
	__Assert(nPage < (int)m_vctPages.size());

	CPage* pPage = m_vctPages[nPage];
	__Ensure(pPage->m_hWnd);

	if (E_ViewTabStyle::VTS_HideTab == m_ViewStyle.TabStyle.eTabStyle)
	{
		pPage->resize(rcClient, g_bManualResize);
	}
	else
	{
		CRect rcTabItem;
		(void)m_wndTabCtrl.GetItemRect(0, &rcTabItem);

		CRect rcPage(1, 1, rcClient.Width()-2, rcClient.Height()-2);
		switch (m_ViewStyle.TabStyle.eTabStyle)
		{
		case E_ViewTabStyle::VTS_TabTop:
			rcPage.top += rcTabItem.Height() + 2;
			break;
		case E_ViewTabStyle::VTS_TabBottom:
			rcPage.bottom -= rcTabItem.Height() + 2;
			break;
		case E_ViewTabStyle::VTS_TabLeft:
			rcPage.left += rcTabItem.Width() + 2;
			break;
		case E_ViewTabStyle::VTS_TabRight:
			rcPage.right -= rcTabItem.Width() + 2;
			break;
		}

		m_wndTabCtrl.MoveWindow(0, 0, rcClient.Width(), rcClient.Height(), g_bManualResize?FALSE:TRUE);

		pPage->resize(rcPage, g_bManualResize);
	}
}
