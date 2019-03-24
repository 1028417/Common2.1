
#include "stdafx.h"

#include <DockView.h>

#define __Offset 8

UINT CViewTab::getItemHeight() const
{
	switch (m_eTabStyle)
	{
	case E_TabStyle::TS_Top:
	case E_TabStyle::TS_Bottom:
		return m_rcTabItem.Height();
		break;
	case E_TabStyle::TS_Left:
	case E_TabStyle::TS_Right:
		return m_rcTabItem.Width();
		break;
	default:
		return 0;
	}
}

BOOL CViewTab::init(const tagViewTabStyle& TabStyle)
{
	SetTabStyle(TabStyle.eTabStyle);

	if (0 != TabStyle.fTabFontSize)
	{
		__EnsureReturn(SetFontSize(TabStyle.fTabFontSize), FALSE);
	}

	if (NULL != TabStyle.pImglst)
	{
		SetImageList(TabStyle.pImglst);
	
		(void)GetItemRect(0, m_rcTabItem);
	}
	else if (0 != TabStyle.uTabHeight)
	{
		if (0 != TabStyle.uTabWidth)
		{
			SetItemSize(CSize(TabStyle.uTabWidth, TabStyle.uTabHeight));

			(void)GetItemRect(0, m_rcTabItem);
		}
		else
		{
			UINT cx = 1;
			UINT cy = 1;
			if (E_TabStyle::TS_Top == m_eTabStyle || E_TabStyle::TS_Bottom == m_eTabStyle)
			{
				cy = TabStyle.uTabHeight;
			}
			else if (E_TabStyle::TS_Left == m_eTabStyle || E_TabStyle::TS_Right == m_eTabStyle)
			{
				cx = TabStyle.uTabHeight;
			}
			else
			{
				return TRUE;
			}

			__EnsureReturn(m_Imglst.Create(cx, cy, ILC_COLOR, 0, 0), FALSE);
			SetImageList(&m_Imglst);

			(void)GetItemRect(0, m_rcTabItem);
		}
	}

	return TRUE;
}

void CViewTab::SetTabStyle(E_TabStyle eTabStyle)
{
	DWORD dwTabStyle = TCS_FOCUSNEVER;
	//dwTabStyle |= TCS_FLATBUTTONS | TCS_OWNERDRAWFIXED;

	switch (m_eTabStyle = eTabStyle)
	{
	case E_TabStyle::TS_HideTab:
		ShowWindow(SW_HIDE);
		break;
	case E_TabStyle::TS_Bottom:
		dwTabStyle |= TCS_BOTTOM;
		break;
	case E_TabStyle::TS_Left:
		dwTabStyle |= TCS_VERTICAL | TCS_MULTILINE | TCS_FIXEDWIDTH | TCS_FORCELABELLEFT;
		break;
	case E_TabStyle::TS_Right:
		dwTabStyle |= TCS_RIGHT | TCS_VERTICAL | TCS_MULTILINE | TCS_FIXEDWIDTH | TCS_FORCELABELLEFT;
		break;
	}
	
	ModifyStyle(0, dwTabStyle);
}

BOOL CViewTab::SetFontSize(float fFontSizeOffset)
{	
	return m_CompatableFont.setFont(*this, fFontSizeOffset);
}

void CViewTab::SetTrackMouse(const CB_TrackMouseEvent& cbMouseEvent)
{
	m_cbMouseEvent = cbMouseEvent;

	m_iTrackMouseFlag = 0;
}

void CViewTab::OnTrackMouseEvent(E_TrackMouseEvent eMouseEvent, const CPoint& point)
{
	m_iTrackMouseFlag = 0;

	if (m_cbMouseEvent)
	{
		m_cbMouseEvent(eMouseEvent, point);
	}
}

BOOL CViewTab::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	switch (message)
	{
	case WM_ERASEBKGND:
		return TRUE;

		break;
	case WM_MOUSEMOVE:
		if (0 == m_iTrackMouseFlag)
		{
			TRACKMOUSEEVENT tme;
			memset(&tme, 0, sizeof tme);
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
		//m_iTrackMouseFlag = 0;

		OnTrackMouseEvent(E_TrackMouseEvent::LME_MouseHover, CPoint(lParam));

		break;
	}

	return __super::OnWndMsg(message, wParam, lParam, pResult);
}

BEGIN_MESSAGE_MAP(CViewTab, CTabCtrl)
	ON_WM_PAINT()
END_MESSAGE_MAP()

void CViewTab::OnPaint()
{
	if (E_TabStyle::TS_Top != m_eTabStyle && E_TabStyle::TS_Bottom != m_eTabStyle)
	{
		__super::OnPaint();
		return;
	}

	CRect rcClient;
	GetClientRect(&rcClient);

	CPaintDC dc(this);
	CCompDC CompDC;
	CompDC.create(&dc, rcClient.Width(), rcClient.Height());
	CDC& MemDC = CompDC.getDC();

	DefWindowProc(WM_PAINT, (WPARAM)MemDC.m_hDC, 0);
	
	MemDC.SetBkMode(TRANSPARENT);

	Graphics graphics(MemDC.m_hDC);

	CRect rcItem;
	auto nItemCount = GetItemCount();
	int nHeight = 0;
	for (int nItem = 0; nItem < nItemCount; nItem++)
	{
		GetItemRect(nItem, rcItem);

		if (0 == nItem)
		{
			rcItem.left = 0;// rcClient.left;
		}

		if (E_TabStyle::TS_Top == m_eTabStyle)
		{
			rcItem.top = 0;// rcClient.top;
		}
		else
		{
			rcItem.bottom = rcClient.bottom;
		}

		if (0 == nItem)
		{
			MemDC.FillSolidRect(CRect(0, rcItem.top-2, rcClient.right, rcItem.bottom+2), __Color_White);
			nHeight = rcItem.Height() + 2;
		}

		_drawItem(MemDC, graphics, nItem, rcItem);
	}
	
	int y = 0;
	if (E_TabStyle::TS_Bottom == m_eTabStyle)
	{
		y = rcClient.bottom - nHeight;
	}
	dc.BitBlt(0, y, rcClient.right, nHeight, &MemDC, 0, y, SRCCOPY);
}

static const Color g_ckSel(255, 255, 255);
static const Color g_crUnsel(242, 242, 242);

void CViewTab::_drawItem(CDC& dc, Graphics& graphics, int nItem, CRect& rcItem)
{
	bool bSel = GetCurSel() == nItem;

	Point pt[]{ { rcItem.left, rcItem.bottom }
		, { rcItem.left, rcItem.top }
		, { rcItem.right - __Offset, rcItem.top }
		, { rcItem.right, rcItem.bottom }
	};

	if (nItem > 0)
	{
		pt[0].X += +__Offset;
	}

	if (E_TabStyle::TS_Bottom == m_eTabStyle)
	{
		std::swap(pt[0].X, pt[1].X);
		std::swap(pt[2].X, pt[3].X);
	}

	SolidBrush whiteBrush(bSel ? g_ckSel : g_crUnsel);
	graphics.FillPolygon(&whiteBrush, pt, sizeof(pt) / sizeof(pt[0]));

	static const Pen s_pen(Color(225, 225, 225), 1);
	graphics.SetSmoothingMode(SmoothingMode::SmoothingModeHighQuality);
	if (!bSel)
	{
		if (nItem > 0)
		{
			graphics.DrawLine(&s_pen, pt[0].X, pt[0].Y + 1, pt[1].X, pt[1].Y + 1);
		}

		graphics.DrawLine(&s_pen, pt[1].X, pt[1].Y + 1, pt[2].X, pt[2].Y + 1);

		graphics.DrawLine(&s_pen, pt[2].X, pt[2].Y, pt[3].X, pt[3].Y);
	}

	TC_ITEMW tci;
	memset(&tci, 0, sizeof tci);
	tci.mask = TCIF_TEXT | TCIF_IMAGE;
	wchar_t szTabText[256]{ 0 };
	tci.pszText = szTabText;
	tci.cchTextMax = sizeof(szTabText) - 1;
	GetItem(nItem, &tci);

	CImageList *pImgLst = GetImageList();
	if (NULL != pImgLst)
	{
		IMAGEINFO ImageInfo;
		memset(&ImageInfo, 0, sizeof ImageInfo);
		if (pImgLst->GetImageInfo(tci.iImage, &ImageInfo))
		{
			rcItem.left += __Offset;

			int nTop = 2+rcItem.top + (rcItem.Height() - (ImageInfo.rcImage.bottom - ImageInfo.rcImage.top)) / 2;

			pImgLst->Draw(&dc, tci.iImage, { rcItem.left, nTop }, ILD_TRANSPARENT);

			rcItem.left += (ImageInfo.rcImage.right - ImageInfo.rcImage.left);

			rcItem.right -= __Offset / 2;
		}
	}
	
	CString strText(tci.pszText);
	strText.Trim();
	dc.DrawText(strText, rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
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
	ON_WM_DESTROY()
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
	DWORD dwStyle = WS_VISIBLE | WS_CHILD;
	if (m_ViewStyle.bBorder)
	{
		dwStyle |= WS_BORDER;
	}

	__AssertReturn(__super::Create(m_pParentWnd, dwStyle, WS_EX_CONTROLPARENT), FALSE);

	__AssertReturn(m_wndTabCtrl.SubclassWindow(this->GetTabControl()->GetSafeHwnd()), FALSE);
	
	__AssertReturn(m_wndTabCtrl.init(m_ViewStyle.TabStyle), FALSE);

	return TRUE;
}

void CDockView::OnDestroy()
{
	(void)m_wndTabCtrl.UnsubclassWindow();

	__super::OnDestroy();
}

BOOL CDockView::AddPage(CPage& Page)
{
	__AssertReturn(!util::ContainerFind(m_vctPages, &Page), FALSE);

	m_vctPages.push_back(&Page);

	__super::AddPage(&Page);

	if (NULL == m_hWnd)
	{
		__AssertReturn(Create(), FALSE);
	}
	
	if (E_TabStyle::TS_HideTab != m_ViewStyle.TabStyle.eTabStyle)
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

BOOL CDockView::SetActivePage(CPage& Page)
{
	int nActivePage = __super::GetActiveIndex();

	PageVector::iterator itPage = std::find(m_vctPages.begin(), m_vctPages.end(), &Page);
	__EnsureReturn(itPage != m_vctPages.end(), FALSE);

	if (itPage - m_vctPages.begin() != nActivePage)
	{
		(void)__super::SetActivePage(*itPage);
	}

	this->resizePage(Page);

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
	UINT uOffset = __DXView;
	if (!m_ViewStyle.sizeable())
	{
		uOffset /= 2;
	}
	
	CRect rtPos(rcViewArea);
	switch (m_ViewStyle.eViewType)
	{
	case E_DockViewType::DVT_DockLeft:
		rtPos.SetRect(0, rcViewArea.top + m_ViewStyle.uStartPos, m_ViewStyle.uDockSize
			, m_ViewStyle.uEndPos ? rcViewArea.top + m_ViewStyle.uEndPos : rcViewArea.bottom);
		rcViewArea.left += m_ViewStyle.uDockSize + uOffset;
		
		break;
	case E_DockViewType::DVT_DockTop:
		rtPos.SetRect(rcViewArea.left + m_ViewStyle.uStartPos, 0
			, m_ViewStyle.uEndPos ? rcViewArea.left + m_ViewStyle.uEndPos : rcViewArea.right, m_ViewStyle.uDockSize);
		rcViewArea.top += m_ViewStyle.uDockSize + uOffset;

		break;
	case E_DockViewType::DVT_DockRight:
		rtPos.SetRect(rcViewArea.right - m_ViewStyle.uDockSize, rcViewArea.top + m_ViewStyle.uStartPos
			, rcViewArea.right, m_ViewStyle.uStartPos ? rcViewArea.top + m_ViewStyle.uStartPos : rcViewArea.bottom);
		rcViewArea.right -= m_ViewStyle.uDockSize + uOffset;

		break;
	case E_DockViewType::DVT_DockBottom:
		rtPos.SetRect(rcViewArea.left + m_ViewStyle.uStartPos, rcViewArea.bottom - m_ViewStyle.uDockSize
			, m_ViewStyle.uEndPos ? rcViewArea.left + m_ViewStyle.uStartPos : rcViewArea.right, rcViewArea.bottom);
		rcViewArea.bottom -= m_ViewStyle.uDockSize + uOffset;

		break;
	}

	g_bManualResize = bManual;
	(void)this->MoveWindow(&rtPos);
	g_bManualResize = false;
}

void CDockView::OnSize(UINT nType, int cx, int cy)
{
	if (m_wndTabCtrl)
	{
		m_wndTabCtrl.MoveWindow(0, 0, cx, cy, FALSE);
		m_wndTabCtrl.Invalidate();

		CPropertyPage *pPage = __super::GetActivePage();
		if (NULL != pPage)
		{
			this->resizePage(*pPage);
		}
	}
}

void CDockView::resizePage(CPropertyPage& Page)
{
	CRect rcClient;
	this->GetClientRect(&rcClient);

	if (E_TabStyle::TS_HideTab == m_ViewStyle.TabStyle.eTabStyle)
	{
		Page.MoveWindow(rcClient);
	}
	else
	{
		CRect rcPage(0, 0, rcClient.right, rcClient.bottom);
		UINT uTabHeight = m_wndTabCtrl.getItemHeight() + 2;
		switch (m_ViewStyle.TabStyle.eTabStyle)
		{
		case E_TabStyle::TS_Top:
			rcPage.top += uTabHeight;
			break;
		case E_TabStyle::TS_Bottom:
			rcPage.bottom -= uTabHeight;
			break;
		case E_TabStyle::TS_Left:
			rcPage.left += uTabHeight;
			break;
		case E_TabStyle::TS_Right:
			rcPage.right -= uTabHeight;
			break;
		}

		Page.MoveWindow(rcPage);
	}
}
