#include "stdafx.h"

#include "Guard.h"

#include "App.h"

CRedrawLockGuard::CRedrawLockGuard(CWnd& wnd, bool bFlag)
	: m_wnd(wnd)
	, m_bFlag(bFlag)
{
	if (m_wnd)
	{
		if (m_bFlag)
		{
			m_bLocked = TRUE == wnd.LockWindowUpdate();
		}
		else
		{
			m_wnd.SetRedraw(FALSE);
			m_bLocked = true;
		}
	}
}

CRedrawLockGuard::CRedrawLockGuard(CRedrawLockGuard& other)
	: m_wnd(other.m_wnd)
	, m_bFlag(other.m_bFlag)
	, m_bLocked(other.m_bLocked)
{
	other.m_bFlag = false;
}

CRedrawLockGuard::CRedrawLockGuard(CRedrawLockGuard&& other)
	: CRedrawLockGuard((CRedrawLockGuard&)other)
{
	other.m_bFlag = false;
}

CRedrawLockGuard::~CRedrawLockGuard()
{
	this->Unlock();
}

void CRedrawLockGuard::Unlock()
{
	if (m_bLocked && m_wnd)
	{
		m_bLocked = false;

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

bool CMenuEx::Popup(HMENU hMenu, CWnd *pWnd)
{
	bool bRet = true;
	if (Attach(hMenu))
	{
		POINT ptCursor{ 0, 0 };
		(void)::GetCursorPos(&ptCursor);
		bRet = TRUE == this->TrackPopupMenu(0, ptCursor.x, ptCursor.y, pWnd);

		Detach();
	}

	m_mapMenuItemInfos.clear();
	
	return bRet;
}

bool CMenuEx::Popup(HMENU hMenu, UINT uItemHeight, float fFontSize, CWnd *pWnd)
{
	m_uItemHeight = uItemHeight;
	m_fFontSize = fFontSize;

	return Popup(hMenu, pWnd);
}

static int cloneMenu(HMENU hDst, HMENU hSrc)
{
	int nCount = 0;

	for (int nSrc = 0, nDst = GetMenuItemCount(hDst); nSrc<GetMenuItemCount(hSrc); nSrc++)
	{
		CString strMenuStr(L'\0', 256);
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
		mii.dwTypeData = (LPTSTR)(LPCTSTR)strMenuStr;
		mii.cch = strMenuStr.GetLength();

		VERIFY(GetMenuItemInfo(hSrc, nSrc, TRUE, &mii));
		strMenuStr.Trim();

		if (mii.hSubMenu)
		{
			HMENU hSub = CreatePopupMenu();
			cloneMenu(hSub, mii.hSubMenu);
			mii.hSubMenu = hSub;
		}

		InsertMenuItem(hDst, nDst++, TRUE, &mii);
		nCount++;
	}

	return nCount;
}

bool CMenuEx::clonePopup(HMENU hMenu, CWnd *pWnd)
{
	HMENU hPopupMenu = ::CreatePopupMenu();
	if (NULL == hPopupMenu)
	{
		return false;
	}
	(void)cloneMenu(hPopupMenu, hMenu);

	return Popup(hPopupMenu, pWnd);
}

bool CMenuEx::clonePopup(HMENU hMenu, UINT uItemHeight, float fFontSize, CWnd *pWnd)
{
	m_uItemHeight = uItemHeight;
	m_fFontSize = fFontSize;
	return clonePopup(hMenu, pWnd);
}

bool CMenuEx::Attach(HMENU hMenu)
{
	if (!__super::Attach(hMenu))
	{
		return false;
	}

	UINT uAvilbleCount = 0;
	int nPrevSpliterPos = -1;
	int nItemCount = this->GetMenuItemCount();
	for (int nItem = nItemCount-1; nItem >= 0; nItem--)
	{
		int nItemID = (int)this->GetMenuItemID(nItem);
		if (-1 == nItemID) // 子菜单
		{
			HMENU hSubMenu = this->GetSubMenu(nItem)->m_hMenu;

			m_lstSubMenu.push_back(CMenuEx(*this, true));
			auto& SubMenu = m_lstSubMenu.back();
			if (!SubMenu.Attach(hSubMenu))
			{
				m_lstSubMenu.pop_back();
				(void)this->RemoveMenu(nItem, MF_BYPOSITION);
				continue;
			}

			(void)this->ModifyMenu(nItem, MF_BYPOSITION | MF_OWNERDRAW | MF_POPUP, (UINT)hSubMenu);
		}
		else if (0 == nItemID) // 分隔条
		{
			if (uAvilbleCount == nPrevSpliterPos + 1)
			{
				(void)this->RemoveMenu(nItem, MF_BYPOSITION);
				continue;
			}

			(void)this->ModifyMenu(nItem, MF_BYPOSITION | MF_OWNERDRAW | MF_SEPARATOR);
			nPrevSpliterPos = uAvilbleCount;
		}
		else
		{
			auto itr = m_mapMenuItemInfos.find((UINT)nItemID);
			if (itr != m_mapMenuItemInfos.end())
			{
				cauto& MenuItemInfo = itr->second;

				if (MenuItemInfo.bDelete || MenuItemInfo.bDisable)
				{
					(void)this->RemoveMenu(nItem, MF_BYPOSITION);
					//(void)this->EnableMenuItem(uIDItem, MF_GRAYED);
					continue;
				}
			}

			(void)this->ModifyMenu(nItem, MF_BYPOSITION | MF_OWNERDRAW, (UINT)nItemID);
		}

		uAvilbleCount++;
	}

	if (uAvilbleCount > 0)
	{
		if (0 == this->GetMenuItemID(0))
		{
			(void)this->RemoveMenu(0, MF_BYPOSITION);
			uAvilbleCount--;
		}
	}

	if (0 == uAvilbleCount)
	{
		Detach();
		return false;
	}

	return true;
}

void CMenuEx::Detach()
{
	for (auto& SubMenu : m_lstSubMenu)
	{
		SubMenu.Detach();
	}
	m_lstSubMenu.clear();

	(void)__super::Detach();
}

void CMenuEx::EnableItem(UINT uIDItem, bool bEnable)
{
	m_mapMenuItemInfos[uIDItem].bDisable = !bEnable;
}

void CMenuEx::EnableItem(const std::initializer_list<UINT>& ilIDItems, bool bEnable)
{
	for (auto& uIDItem : ilIDItems)
	{
		m_mapMenuItemInfos[uIDItem].bDisable = !bEnable;
	}
}

void CMenuEx::CheckItem(UINT uIDItem)
{
	m_mapMenuItemInfos[uIDItem].bChecked = true;
}

void CMenuEx::DeleteItem(UINT uIDItem)
{
	m_mapMenuItemInfos[uIDItem].bDelete = true;
}

void CMenuEx::DeleteItem(const std::initializer_list<UINT>& ilIDItems)
{
	for (auto& uIDItem : ilIDItems)
	{
		m_mapMenuItemInfos[uIDItem].bDelete = true;
	}
}

void CMenuEx::SetItemText(UINT uIDItem, const CString& cstrText)
{
	m_mapMenuItemInfos[uIDItem].strText = cstrText;
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

	onDrawItem(dc, lpDrawItemStruct);

	dc.Detach();
}

void CMenuEx::onDrawItem(CDC& dc, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CRect rcItem = lpDrawItemStruct->rcItem;
	int nItemID = lpDrawItemStruct->itemID;

	auto crBk = m_bTopMenu ? GetSysColor(COLOR_MENU) : RGB(251, 251, 251);
	if (0 != nItemID && lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		crBk = m_bTopMenu ? RGB(204, 232, 255) : RGB(229, 243, 255);
	}
	dc.FillSolidRect(&rcItem, crBk);

	if (0 == nItemID) // 分隔条
	{
		CRect rcLine;
		rcLine.left = rcItem.left + 10;
		rcLine.right = rcItem.right - 10;
		rcLine.top = (rcItem.top + rcItem.bottom) / 2;
		rcLine.bottom = rcLine.top + 1;

		dc.FillSolidRect(&rcLine, RGB(200, 200, 200));
	}
	else
	{
		dc.SetBkMode(TRANSPARENT);

		CFont *pFontPrev = dc.GetCurrentFont();
		CCompatableFont font(m_bTopMenu ? m_fTopFontSize : m_fFontSize);
		if (font.create(*pFontPrev))
		{
			(void)dc.SelectObject(&font);
		}

		if (m_bTopMenu)
		{
			rcItem.top -= 3;
		}

		CString strText;
		if (nItemID > 0)
		{
			auto itr = m_mapMenuItemInfos.find((UINT)nItemID);
			if (itr != m_mapMenuItemInfos.end())
			{
				strText = itr->second.strText;

				if (itr->second.bChecked)
				{
					dc.DrawText(L" √", rcItem, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
				}
			}
		}

		if (strText.IsEmpty())
		{
			this->GetMenuString((UINT)nItemID, strText, MF_BYCOMMAND);
		}
		dc.DrawText(strText, rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		
		(void)dc.SelectObject(pFontPrev);
	}
}

void CMenuGuard::EnableItem(UINT uIDItem, bool bEnable)
{
	m_menu.EnableItem(uIDItem, bEnable);
}

void CMenuGuard::EnableItem(const std::initializer_list<UINT>& ilIDItems, bool bEnable)
{
	m_menu.EnableItem(ilIDItems, bEnable);
}

void CMenuGuard::CheckItem(UINT uIDItem)
{
	m_menu.CheckItem(uIDItem);
}

void CMenuGuard::DeleteItem(UINT uIDItem)
{
	m_menu.DeleteItem(uIDItem);
}

void CMenuGuard::DeleteItem(const std::initializer_list<UINT>& ilIDItems)
{
	m_menu.DeleteItem(ilIDItems);
}

void CMenuGuard::SetItemText(UINT uIDItem, const CString& cstrText)
{
	m_menu.SetItemText(uIDItem, cstrText);
}

bool CMenuGuard::Popup(CWnd *pWnd, UINT uItemHeight, float fFontSize)
{
	HMENU hMenu = m_resModule.loadMenu(m_uIDMenu);
	__AssertReturn(hMenu, false);
	
	bool bRet = m_menu.Popup(::GetSubMenu(hMenu, 0), uItemHeight + 2, fFontSize, pWnd);

	(void)::DestroyMenu(hMenu);

	return bRet;
}

bool CMenuGuard::clonePopup(CWnd *pWnd, UINT uItemHeight, float fFontSize)
{
	HMENU hMenu = m_resModule.loadMenu(m_uIDMenu);
	__AssertReturn(hMenu, false);

	bool bRet = m_menu.clonePopup(hMenu, uItemHeight + 2, fFontSize, pWnd);
	
	(void)::DestroyMenu(hMenu);

	return bRet;
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

	wcscpy_s(logFont.lfFaceName, L"微软雅黑");

	int nOffset = (int)round(logFont.lfHeight*abs(m_fFontSizeOffset));
	if (m_fFontSizeOffset > 0)
	{
		logFont.lfHeight += nOffset;
	}
	else if (m_fFontSizeOffset < 0)
	{
		logFont.lfHeight -= nOffset;
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
