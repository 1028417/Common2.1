#include "stdafx.h"

#include "Common/Guard.h"

#include "Common/App.h"

CRedrawLock::CRedrawLock(CWnd& wnd, bool bLockUpdate)
	: m_wnd(wnd)
	, m_bLockUpdate(bLockUpdate)
{
	if (m_wnd)
	{
		if (m_bLockUpdate)
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

CRedrawLock::CRedrawLock(CRedrawLock& other)
	: m_wnd(other.m_wnd)
	, m_bLockUpdate(other.m_bLockUpdate)
	, m_bLocked(other.m_bLocked)
{
	other.m_bLocked = false;
}

CRedrawLock::~CRedrawLock()
{
	this->Unlock();
}

void CRedrawLock::Unlock()
{
	if (m_bLocked && m_wnd)
	{
		m_bLocked = false;

		if (m_bLockUpdate)
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
		memzero(mii);
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
	for (int nIdx = nItemCount-1; nIdx >= 0; nIdx--)
	{
		int nItemID = (int)this->GetMenuItemID(nIdx);
		if (-1 == nItemID) // 子菜单
		{
			HMENU hSubMenu = this->GetSubMenu(nIdx)->m_hMenu;

			m_lstSubMenu.emplace_back(*this, true);
			auto& SubMenu = m_lstSubMenu.back();
			if (!SubMenu.Attach(hSubMenu))
			{
				m_lstSubMenu.pop_back();
				(void)this->RemoveMenu(nIdx, MF_BYPOSITION);
				continue;
			}

			(void)this->ModifyMenu(nIdx, MF_BYPOSITION | MF_OWNERDRAW | MF_POPUP, (UINT)hSubMenu);
		}
		else if (0 == nItemID) // 分隔条
		{
			if (uAvilbleCount == nPrevSpliterPos + 1)
			{
				(void)this->RemoveMenu(nIdx, MF_BYPOSITION);
				continue;
			}

			(void)this->ModifyMenu(nIdx, MF_BYPOSITION | MF_OWNERDRAW | MF_SEPARATOR);
			nPrevSpliterPos = uAvilbleCount;
		}
		else
		{
			auto itr = m_mapMenuItemInfos.find((UINT)nItemID);
			if (itr != m_mapMenuItemInfos.end())
			{
				cauto MenuItemInfo = itr->second;

				if (MenuItemInfo.bDelete)
				{
					(void)this->RemoveMenu(nIdx, MF_BYPOSITION);
					continue;
				}
				else if(MenuItemInfo.bDisable)
				{
					(void)this->RemoveMenu(nIdx, MF_BYPOSITION); //(void)this->EnableMenuItem(nItemID, MF_GRAYED);
					continue;
				}
			}

			(void)this->ModifyMenu(nIdx, MF_BYPOSITION | MF_OWNERDRAW, (UINT)nItemID);
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
	for (auto uIDItem : ilIDItems)
	{
		m_mapMenuItemInfos[uIDItem].bDisable = !bEnable;
	}
}

void CMenuEx::CheckItem(UINT uIDItem, bool bCheck)
{
	m_mapMenuItemInfos[uIDItem].bChecked = bCheck;
}

void CMenuEx::DeleteItem(UINT uIDItem)
{
	m_mapMenuItemInfos[uIDItem].bDelete = true;
}

void CMenuEx::DeleteItem(const std::initializer_list<UINT>& ilIDItems)
{
	for (auto uIDItem : ilIDItems)
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

	COLORREF crBk = 0;
	if (lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		crBk = __crSelect;
	}
	else
	{
		crBk = m_bTopMenu ? GetSysColor(COLOR_MENU) : RGB(251, 251, 251);
	}
	dc.FillSolidRect(&rcItem, crBk);

	if (0 == nItemID) // 分隔条
	{
		CRect rcLine;
		rcLine.left = rcItem.left + 10;
		rcLine.right = rcItem.right - 10;
		rcLine.top = (rcItem.top + rcItem.bottom) / 2;
		rcLine.bottom = rcLine.top + 1;

		dc.FillSolidRect(&rcLine, RGB(234, 234, 234));
	}
	else
	{
		dc.SetBkMode(TRANSPARENT);

		CDCFontGuard DCFontGuard(dc, m_bTopMenu ? m_fTopFontSize : m_fFontSize
			, m_bTopMenu ? 400 : 0);

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
					dc.DrawText(L"  √", rcItem, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
				}
			}
		}

		if (strText.IsEmpty())
		{
			this->GetMenuString((UINT)nItemID, strText, MF_BYCOMMAND);
		}
		dc.DrawText(strText, rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
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

void CMenuGuard::CheckItem(UINT uIDItem, bool bCheck)
{
	m_menu.CheckItem(uIDItem, bCheck);
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

bool CCompatableFont::create(HFONT hFont, const CB_CompatableFont& cb)
{
	LOGFONT logFont;
	::ZeroMemory(&logFont, sizeof(logFont));
	if (0 == ::GetObject(hFont, sizeof(logFont), &logFont))
	{
		return false;
	}

	logFont.lfQuality = CLEARTYPE_NATURAL_QUALITY; // CLEARTYPE_QUALITY
	logFont.lfOutPrecision = OUT_TT_PRECIS; // OUT_TT_ONLY_PRECIS
	logFont.lfClipPrecision = CLIP_CHARACTER_PRECIS;
	logFont.lfCharSet = DEFAULT_CHARSET;
	//logFont.lfPitchAndFamily = DEFAULT_PITCH;

	int nOffset = (int)round(logFont.lfHeight*abs(m_fFontSizeOffset));
	if (m_fFontSizeOffset > 0)
	{
		logFont.lfHeight += nOffset;
	}
	else if (m_fFontSizeOffset < 0)
	{
		logFont.lfHeight -= nOffset;
	}
		
	if (m_bItalic)
	{
		logFont.lfItalic = 1;
	}

	if (m_bUnderline)
	{
		logFont.lfUnderline = 1;
	}

	if (cb)
	{
		cb(logFont);
	}

	if (m_lfWeight > 0)
	{
		logFont.lfWeight = m_lfWeight;
	}
	else
	{
		logFont.lfWeight = 0;
	}

	wstring	strFaceName;	
	if (logFont.lfWeight < 400)
	{
		strFaceName = L" Light";
	}
	else if (400 == logFont.lfWeight)
	{
		strFaceName = L" Semilight";
	}

	wcscpy_s(logFont.lfFaceName, (L"微软雅黑" + strFaceName).c_str());

	if (m_hObject)
	{
		this->DeleteObject();
	}

	if (!CreateFontIndirect(&logFont))
	{
		return false;
	}

	return true;
}

bool CCompatableFont::create(HFONT hFont, float fFontSizeOffset, LONG lfWeight, bool bItalic, bool bUnderline)
{
	m_fFontSizeOffset = fFontSizeOffset;
	m_lfWeight = lfWeight;
	m_bItalic = bItalic;
	m_bUnderline = bUnderline;

	return create(hFont, nullptr);
}

bool CCompatableFont::create(HDC hDC, float fFontSizeOffset, LONG lfWeight, bool bItalic, bool bUnderline)
{
	m_fFontSizeOffset = fFontSizeOffset;
	m_lfWeight = lfWeight;
	m_bItalic = bItalic;
	m_bUnderline = bUnderline;

	return create(hDC, nullptr);
}

bool CCompatableFont::create(HWND hWnd, float fFontSizeOffset, LONG lfWeight, bool bItalic, bool bUnderline)
{
	m_fFontSizeOffset = fFontSizeOffset;
	m_lfWeight = lfWeight;
	m_bItalic = bItalic;
	m_bUnderline = bUnderline;

	return create(hWnd, nullptr);
}

bool CCompatableFont::setFont(HWND hWnd, float fFontSizeOffset, LONG lfWeight, bool bItalic, bool bUnderline)
{
	if (!create(hWnd, fFontSizeOffset, lfWeight, bItalic, bUnderline))
	{
		return false;
	}
	
	SendMessage(hWnd, WM_SETFONT, (WPARAM)this->GetSafeHandle(), 1);

	return true;
}
