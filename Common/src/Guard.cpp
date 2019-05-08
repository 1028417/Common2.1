#include "stdafx.h"

#include "Guard.h"

#include "App.h"

#include <afxglobals.h>


CRedrawLockGuard::CRedrawLockGuard(CWnd& wnd, bool bFlag)
	: m_wnd(wnd)
	, m_bFlag(bFlag)
{
	if (m_wnd)
	{
		if (m_bFlag)
		{
			m_bLocked = wnd.LockWindowUpdate();
		}
		else
		{
			m_wnd.SetRedraw(FALSE);
			m_bLocked = TRUE;
		}
	}
}

CRedrawLockGuard::~CRedrawLockGuard()
{
	this->Unlock();
}

void CRedrawLockGuard::Unlock()
{
	if (m_bLocked && m_wnd)
	{
		m_bLocked = FALSE;

		if (m_bFlag)
		{
			m_wnd.UnlockWindowUpdate();
		}
		else
		{
			m_wnd.SetRedraw(TRUE);
			m_wnd.Invalidate(FALSE); //m_wnd.RedrawWindow(NULL, NULL, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
		}
	}
}

void CMenuEx::_setOwnerDraw()
{
	int iItemCount = this->GetMenuItemCount();
	for (int iItem = 0; iItem < iItemCount; iItem++)
	{
		UINT uItemID = this->GetMenuItemID(iItem);
		
		if (-1 == uItemID)//×Ó²Ëµ¥
		{
			m_lstSubMenu.push_back(CMenuEx(m_uItemHeight, m_uMenuWidth, m_fFontSize));
			m_lstSubMenu.back().Attach(this->GetSubMenu(iItem)->m_hMenu, FALSE); //µÝ¹éµ÷ÓÃ
		}

		CString strText;
		this->GetMenuString(iItem, strText, MF_BYPOSITION);

		tagMENUITEMINFOW mmi;
		memset(&mmi, 0, sizeof(mmi));
		mmi.cbSize = sizeof(mmi);
		mmi.fMask = MIIM_SUBMENU;
		if (CMenu::GetMenuItemInfo(iItem, &mmi, TRUE) && NULL != mmi.hSubMenu)
		{
			(void)this->ModifyMenu(iItem, MF_BYPOSITION | MF_OWNERDRAW | MF_POPUP
				, (UINT)mmi.hSubMenu, (LPCTSTR)strText);
		}
		else
		{
			(void)this->ModifyMenu(iItem, MF_BYPOSITION | MF_OWNERDRAW, uItemID, (LPCTSTR)strText);
		}
	}
}

HMENU CMenuEx::Detach()
{
	if (NULL == m_hMenuAttach)
	{
		return NULL;
	}
	
	m_hMenuAttach = NULL;
	
	return __super::Detach();
}

BOOL CMenuEx::Attach(HMENU hMenu, BOOL bTopMenu)
{
	BOOL bRet = __super::Attach(hMenu);
	if (!bRet)
	{
		return FALSE;
	}

	m_hMenuAttach = hMenu;
	m_bTopMenu = bTopMenu;

	_setOwnerDraw();

	return TRUE;
}

#define __CXOffsetTopMenu 6
#define __CYSplitor 5

void CMenuEx::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	__super::MeasureItem(lpMeasureItemStruct);

	if (0 != lpMeasureItemStruct->itemID)
	{
		lpMeasureItemStruct->itemHeight = m_uItemHeight;

		lpMeasureItemStruct->itemWidth = m_uMenuWidth;
		if (m_bTopMenu)
		{
			lpMeasureItemStruct->itemWidth += __CXOffsetTopMenu;
		}
	}
	else
	{
		lpMeasureItemStruct->itemHeight = __CYSplitor;
	}
}

void CMenuEx::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);

	CRect rcItem = lpDrawItemStruct->rcItem;
	
	auto crBk = m_bTopMenu? GetSysColor(COLOR_MENU) : RGB(251, 251, 251);
	if (0 != lpDrawItemStruct->itemID && lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		crBk = m_bTopMenu ? RGB(204, 232, 255) : RGB(229, 243, 255);
	}
	dc.FillSolidRect(&rcItem, crBk);

	if (0 != lpDrawItemStruct->itemID)
	{
		dc.SetBkMode(TRANSPARENT);

		CFont *pFontPrev = dc.GetCurrentFont();
		CCompatableFont font(m_bTopMenu ? m_fTopFontSize : m_fFontSize);
		if (font.create(*pFontPrev))
		{
			(void)dc.SelectObject(&font);
		}

		CString strText;
		this->GetMenuString(lpDrawItemStruct->itemID, strText, MF_BYCOMMAND);
		dc.DrawText(strText, &rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		
		(void)dc.SelectObject(pFontPrev);
	}
	else
	{
		CRect rcLine;
		rcLine.left = rcItem.left+10;
		rcLine.right = rcItem.right-10;
		rcLine.top = (rcItem.top + rcItem.bottom) / 2;
		rcLine.bottom = rcLine.top + 1;

		dc.FillSolidRect(&rcLine, RGB(200, 200, 200));
	}

	onDrawItem(dc, lpDrawItemStruct);

	dc.Detach();
}

void CMenuGuard::EnableItem(UINT uIDItem, BOOL bEnable)
{
	m_mapMenuItemInfos[uIDItem].bEnable = TRUE==bEnable;
}

void CMenuGuard::EnableItem(const std::initializer_list<UINT>& ilIDItems, BOOL bEnable)
{
	for (auto& uIDItem : ilIDItems)
	{
		EnableItem(uIDItem, bEnable);
	}
}

void CMenuGuard::DisableItem(UINT uIDItem)
{
	EnableItem(uIDItem, FALSE);
}

void CMenuGuard::DisableItem(const std::initializer_list<UINT>& ilIDItems, BOOL bEnable)
{
	EnableItem(ilIDItems, FALSE);
}

void CMenuGuard::DeleteItem(UINT uIDItem)
{
	m_mapMenuItemInfos[uIDItem].bDelete = true;
}

void CMenuGuard::DeleteItem(const std::initializer_list<UINT>& ilIDItems)
{
	for (auto& uIDItem : ilIDItems)
	{
		DeleteItem(uIDItem);
	}
}

void CMenuGuard::SetItemText(UINT uIDItem, const CString& cstrText)
{
	m_mapMenuItemInfos[uIDItem].strText = cstrText;
}

BOOL CMenuGuard::_popup(HMENU hMenu, CWnd *pWnd, UINT uItemHeight, float fFontSize)
{
	CPoint ptCursor(0, 0);
	(void)::GetCursorPos(&ptCursor);

	CMenuEx menu(uItemHeight + 2, m_uMenuWidth, fFontSize, hMenu);
	return menu.TrackPopupMenu(0, ptCursor.x, ptCursor.y, pWnd);
}

BOOL CMenuGuard::Popup(CWnd *pWnd, UINT uItemHeight, float fFontSize)
{
	HMENU hMenu = m_resModule.loadMenu(m_uIDMenu);
	__AssertReturn(hMenu, FALSE);
	
	HMENU hSubMenu = ::GetSubMenu(hMenu, 0);

	for (auto& pr : m_mapMenuItemInfos)
	{
		auto& uIDItem = pr.first;
		auto& MenuItemInfo = pr.second;
		
		if (!MenuItemInfo.strText.IsEmpty())
		{
			(void)::ModifyMenu(hSubMenu, uIDItem, MF_BYCOMMAND | MF_STRING, uIDItem, MenuItemInfo.strText);
		}

		if (!MenuItemInfo.bEnable)
		{
			if (!m_bShowDisable)
			{
				(void)::RemoveMenu(hSubMenu, uIDItem, MF_BYCOMMAND);
			}
			else
			{
				(void)::EnableMenuItem(hSubMenu, uIDItem, MF_GRAYED);
			}
		}
	}

	m_mapMenuItemInfos.clear();

	int iCount = ::GetMenuItemCount(hSubMenu);
	if (iCount > 0)
	{
		while (iCount > 0 && 0 == ::GetMenuItemID(hSubMenu, 0))
		{
			(void)::RemoveMenu(hSubMenu, 0, MF_BYPOSITION);
			iCount--;
		}

		while (iCount > 0 && 0 == ::GetMenuItemID(hSubMenu, iCount - 1))
		{
			(void)::RemoveMenu(hSubMenu, iCount - 1, MF_BYPOSITION);
			iCount--;
		}
	}
	
	BOOL bRet = FALSE;
	if (iCount > 0)
	{
		bRet = _popup(hSubMenu, pWnd, uItemHeight, fFontSize);
	}

	(void)::DestroyMenu(hMenu);

	return bRet;
}

static int clonePopupMenu(HMENU hDst, HMENU hSrc)
{
	int iCnt = 0;

	for (int iSrc = 0, iDst = GetMenuItemCount(hDst); iSrc<GetMenuItemCount(hSrc); iSrc++)
	{
		CString szMenuStr(L'\0', 256);
		MENUITEMINFO mii;
		memset(&mii, 0, sizeof mii);
		mii.cbSize = sizeof(mii);
		mii.fMask = 0
			| MIIM_CHECKMARKS //Retrieves or sets the hbmpChecked and hbmpUnchecked members. 
			| MIIM_DATA //Retrieves or sets the dwItemData member. 
			| MIIM_ID //Retrieves or sets the wID member. 
			| MIIM_STATE //Retrieves or sets the fState member. 
			| MIIM_SUBMENU //Retrieves or sets the hSubMenu member. 
			| MIIM_TYPE //Retrieves or sets the fType and dwTypeData members. 
			| 0;
		mii.dwTypeData = (LPTSTR)(LPCTSTR)szMenuStr;
		mii.cch = szMenuStr.GetLength();

		VERIFY(GetMenuItemInfo(hSrc, iSrc, TRUE, &mii));

		szMenuStr.Trim();

		if (mii.hSubMenu)
		{
			HMENU hSub = CreatePopupMenu();
			clonePopupMenu(hSub, mii.hSubMenu);
			mii.hSubMenu = hSub;
		}

		InsertMenuItem(hDst, iDst++, TRUE, &mii);
		iCnt++;
	}

	return iCnt;
}

BOOL CMenuGuard::PopupEx(CWnd *pWnd, UINT uItemHeight, float fFontSize)
{
	HMENU hPopupMenu = ::CreatePopupMenu();
	if (NULL == hPopupMenu)
	{
		return FALSE;
	}

	HMENU hMenu = m_resModule.loadMenu(m_uIDMenu);
	if (NULL == hMenu)
	{
		return FALSE;
	}

	(void)clonePopupMenu(hPopupMenu, hMenu);

	(void)::DestroyMenu(hMenu);

	return _popup(hPopupMenu, pWnd, uItemHeight, fFontSize);
}

bool CCompatableFont::create(CFont& font, const CB_CompatableFont& cb)
{
	if (NULL != m_hObject)
	{
		return false;
	}

	LOGFONT logFont;
	::ZeroMemory(&logFont, sizeof(logFont));
	if (0 == font.GetLogFont(&logFont))
	{
		return false;
	}

	logFont.lfQuality = ANTIALIASED_QUALITY;

	logFont.lfCharSet = DEFAULT_CHARSET;

	wcscpy_s(logFont.lfFaceName, L"Î¢ÈíÑÅºÚ");

	int iOffset = (int)round(logFont.lfHeight*abs(m_fFontSizeOffset));
	if (m_fFontSizeOffset > 0)
	{
		logFont.lfHeight += iOffset;
	}
	else if (m_fFontSizeOffset < 0)
	{
		logFont.lfHeight -= iOffset;
	}

	if (cb)
	{
		cb(logFont);
	}
		
	if (!CreateFontIndirect(&logFont))
	{
		return false;
	}

	return true;
}

bool CCompatableFont::create(CDC& dc, const CB_CompatableFont& cb)
{
	auto pFont = dc.GetCurrentFont();
	if (NULL == pFont)
	{
		return false;
	}

	return create(*pFont, cb);
}

bool CCompatableFont::create(CWnd& wnd, const CB_CompatableFont& cb)
{
	CFont *pFont = wnd.GetFont();
	if (NULL == pFont)
	{
		return false;
	}

	if (!create(*pFont, cb))
	{
		return false;
	}
	
	return true;
}

bool CCompatableFont::create(CWnd& wnd, float fFontSizeOffset, const CB_CompatableFont& cb)
{
	m_fFontSizeOffset = fFontSizeOffset;

	return create(wnd, cb);
}

bool CCompatableFont::setFont(CWnd& wnd, float fFontSizeOffset)
{
	if (NULL == m_hObject)
	{
		if (!create(wnd, fFontSizeOffset))
		{
			return false;
		}
	}

	(void)wnd.SetFont(this);

	return true;
}
