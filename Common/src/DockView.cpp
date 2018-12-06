
#include "stdafx.h"

#include <DockView.h>

void CTabCtrlEx::SetTabStyle(E_ViewTabStyle eTabStyle, UINT cx, UINT cy)
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
	
	if (0 != cx && 0 != cy)
	{
		SetItemSize(CSize(cx, cy));
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
CDockView::CDockView(CWnd& wndParent, ST_ViewStyle eStyle, UINT uDockSize, UINT uOffset, UINT uTabFontSize, UINT uTabHeight)
	: CPropertySheet(L"", &wndParent)
	, m_eStyle(eStyle)
	, m_uDockSize(uDockSize)
	, m_uOffset(uOffset)
	, m_uTabFontSize(uTabFontSize)
	, m_uTabHeight(uTabHeight)
{
}

CDockView::CDockView(CWnd& wndParent, ST_ViewStyle eStyle, UINT uDockSize, UINT uOffset, UINT uTabFontSize, CImageList *pImglst)
	: CPropertySheet(L"", &wndParent)
	, m_eStyle(eStyle)
	, m_uDockSize(uDockSize)
	, m_uOffset(uOffset)
	, m_uTabFontSize(uTabFontSize)
	, m_pImglst(pImglst)
{
}

CDockView::CDockView(CWnd& wndParent, ST_ViewStyle eStyle, const CRect& rtPos)
	: CPropertySheet(L"", &wndParent)
	, m_eStyle(eStyle)
	, m_rtPos(rtPos)
{
}

BEGIN_MESSAGE_MAP(CDockView, CPropertySheet)
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CDockView::Create()
{
	DWORD dwStyle = WS_CHILD;

	E_ViewTabStyle eTabStyle = __TabStyle(m_eStyle);
	if (E_ViewTabStyle::VTS_HideTab == eTabStyle)
	{
		dwStyle |= WS_BORDER;
	}

	__AssertReturn(__super::Create(m_pParentWnd, dwStyle, WS_EX_CONTROLPARENT), FALSE);

	__AssertReturn(m_wndTabCtrl.SubclassWindow(this->GetTabControl()->GetSafeHwnd()), FALSE);
	
	m_wndTabCtrl.SetTabStyle(eTabStyle);

	if (0 != m_uTabFontSize)
	{
		__EnsureReturn(m_wndTabCtrl.SetFontSize(m_uTabFontSize), FALSE);
	}

	if (NULL != m_pImglst)
	{
		m_wndTabCtrl.SetImageList(m_pImglst);
	}
	else if (0 != m_uTabHeight)
	{
		__EnsureReturn(m_wndTabCtrl.SetTabHeight(m_uTabHeight), FALSE);
	}
	
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
	}

	//pPage->MoveWindow(m_rtPos);

	if (E_ViewTabStyle::VTS_HideTab != __TabStyle(m_eStyle) && !Page.m_cstrTitle.IsEmpty())
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

	this->OnSize();

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

void CDockView::Resize(CRect& rcRestrict)
{
	if (E_ViewDockStyle::VDS_NoDock ==__DockStyle(m_eStyle))
	{
		m_uDockSize = 0;
	}
	//else
	//{
	//	if (0 == (m_eStyle & VS_FixSize))
	//	{
	//		m_uDockSize = max(m_uDockSize, m_nMinDockSize);
	//	}
	//}

	int nOffSize = m_uDockSize;
	if (0 == (VS_FixSize & m_eStyle))
	{
		nOffSize += __DXView;
	}

	switch (__DockStyle(m_eStyle))
	{
	case VS_DockLeft:
		m_rtPos.SetRect(0, rcRestrict.top + m_uOffset, m_uDockSize, rcRestrict.bottom);
		rcRestrict.left += nOffSize;

		break;
	case VS_DockTop:
		m_rtPos.SetRect(rcRestrict.left, 0, rcRestrict.Width(), m_uDockSize);
		rcRestrict.top += nOffSize;

		break;
	case VS_DockRight:
		m_rtPos.SetRect(rcRestrict.right - m_uDockSize, rcRestrict.top, rcRestrict.right, rcRestrict.bottom);
		rcRestrict.right -= nOffSize;

		break;
	case VS_DockBottom:
		m_rtPos.SetRect(rcRestrict.left, rcRestrict.bottom - m_uDockSize, rcRestrict.right, rcRestrict.bottom);
		rcRestrict.bottom -= nOffSize;

		break;
	case VS_DockCenter:
		m_rtPos.CopyRect(rcRestrict);

		break;
	default:
		break;
	}

	if (this->GetSafeHwnd())
	{
		(void)this->MoveWindow(&m_rtPos);

		(void)this->ShowWindow(SW_SHOW);
	}
}

void CDockView::OnSize(UINT nType, int, int)
{
	__Ensure(SIZE_MINIMIZED != nType);

	__Ensure(m_wndTabCtrl);

	this->OnSize();
}

void CDockView::OnSize()
{
	CRect rcClient;
	this->GetClientRect(&rcClient);

	int nPage = __super::GetActiveIndex();
	__Assert(nPage < (int)m_vctPages.size());

	CPage* pPage = m_vctPages[nPage];
	__Ensure(pPage->m_hWnd);

	E_ViewTabStyle eTabStyle = __TabStyle(m_eStyle);
	if (E_ViewTabStyle::VTS_HideTab == eTabStyle)
	{
		pPage->MoveWindow(&rcClient);
	}
	else
	{
		m_wndTabCtrl.MoveWindow(0, 0, rcClient.Width(), rcClient.Height());

		CRect rcTabItem;
		(void)m_wndTabCtrl.GetItemRect(0, &rcTabItem);

		CRect rcPage(1, 1, rcClient.Width()-2, rcClient.Height()-2);
		switch (eTabStyle)
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

		pPage->MoveWindow(&rcPage);
	}
}
