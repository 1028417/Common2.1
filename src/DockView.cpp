
#include "stdafx.h"

#include <DockView.h>

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
CDockView::CDockView(CWnd& wndParent, ST_ViewStyle nStyle, UINT nDockSize, UINT uOffset, UINT uTabFontSize, UINT uTabHeight)
	: CPropertySheet(L"", &wndParent)
	, m_nStyle(nStyle)
	, m_nDockSize(nDockSize)
	, m_uOffset(uOffset)
	, m_uTabFontSize(uTabFontSize)
	, m_uTabHeight(uTabHeight)
{
}

CDockView::CDockView(CWnd& wndParent, ST_ViewStyle nStyle, UINT nDockSize, UINT uOffset, UINT uTabFontSize, CImageList *pImglst)
	: CPropertySheet(L"", &wndParent)
	, m_nStyle(nStyle)
	, m_nDockSize(nDockSize)
	, m_uOffset(uOffset)
	, m_uTabFontSize(uTabFontSize)
	, m_pImglst(pImglst)
{
}

CDockView::CDockView(CWnd& wndParent, ST_ViewStyle nStyle, const CRect& rtPos)
	: CPropertySheet(L"", &wndParent)
	, m_nStyle(nStyle)
	, m_rtPos(rtPos)
{
}

BEGIN_MESSAGE_MAP(CDockView, CPropertySheet)
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CDockView::Create()
{
	DWORD dwStyle = WS_CHILD;
	if (!__TabStyle(m_nStyle))
	{
		dwStyle |= WS_BORDER;
	}

	__AssertReturn(__super::Create(m_pParentWnd, dwStyle, WS_EX_CONTROLPARENT), FALSE);
	
	__AssertReturn(m_wndTabCtrl.SubclassWindow(this->GetTabControl()->GetSafeHwnd()), FALSE);

	if (__TabStyle(m_nStyle))
	{
		m_wndTabCtrl.ModifyStyle(TCS_MULTILINE, TCS_FOCUSNEVER| (VS_BottomTab == __TabStyle(m_nStyle)? TCS_BOTTOM:0));

		if (m_uTabFontSize > 0)
		{
			(void)m_fontGuide.setFontSize(m_wndTabCtrl, m_uTabFontSize);
		}

		if (NULL != m_pImglst)
		{
			m_wndTabCtrl.SetImageList(m_pImglst);
		}
		else if (m_uTabHeight > 0)
		{
			if (m_InnerImglst.Create(m_uTabHeight, m_uTabHeight, ILC_COLOR8, 1, 0))
			{
				m_wndTabCtrl.SetImageList(&m_InnerImglst);
			}
		}
	}
	else
	{
		m_wndTabCtrl.ShowWindow(SW_HIDE);
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

	if (__TabStyle(m_nStyle) && !Page.m_cstrTitle.IsEmpty())
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
	if (!__DockStyle(m_nStyle))
	{
		m_nDockSize = 0;
	}
	//else
	//{
	//	if (0 == (m_nStyle & VS_FixSize))
	//	{
	//		m_nDockSize = max(m_nDockSize, m_nMinDockSize);
	//	}
	//}

	int nOffSize = m_nDockSize;
	if (0 == (VS_FixSize & m_nStyle))
	{
		nOffSize += __DXView;
	}

	switch (__DockStyle(m_nStyle))
	{
	case VS_DockLeft:
		m_rtPos.SetRect(0, rcRestrict.top + m_uOffset, m_nDockSize, rcRestrict.bottom);
		rcRestrict.left += nOffSize;

		break;
	case VS_DockTop:
		m_rtPos.SetRect(rcRestrict.left, 0, rcRestrict.Width(), m_nDockSize);
		rcRestrict.top += nOffSize;

		break;
	case VS_DockRight:
		m_rtPos.SetRect(rcRestrict.right - m_nDockSize, rcRestrict.top, rcRestrict.right, rcRestrict.bottom);
		rcRestrict.right -= nOffSize;

		break;
	case VS_DockBottom:
		m_rtPos.SetRect(rcRestrict.left, rcRestrict.bottom - m_nDockSize, rcRestrict.right, rcRestrict.bottom);
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

	this->OnSize();
}

void CDockView::OnSize()
{
	CRect rcClient;
	this->GetClientRect(&rcClient);

	CTabCtrl* pTabCtrl = this->GetTabControl();
	__Ensure(pTabCtrl);
	
	int nPage = __super::GetActiveIndex();
	__Assert(nPage < (int)m_vctPages.size());

	CPage* pPage = m_vctPages[nPage];
	__Ensure(pPage->m_hWnd);

	if (!__TabStyle(m_nStyle))
	{
		pPage->MoveWindow(&rcClient);
	}
	else
	{
		pTabCtrl->MoveWindow(0, 0, rcClient.Width(), rcClient.Height());

		CRect rcItem(0, 0, 0, 0);
		(void)pTabCtrl->GetItemRect(0, &rcItem);
		int nItemHeight = rcItem.Height();
		nItemHeight += 2;

		CRect rcPage(1, 1, rcClient.Width()-3, rcClient.Height()-2);
		if (VS_BottomTab == __TabStyle(m_nStyle))
		{
			rcPage.bottom -= nItemHeight;
		}
		else
		{
			rcPage.top += nItemHeight;
		}

		pPage->MoveWindow(&rcPage);
	}
}
