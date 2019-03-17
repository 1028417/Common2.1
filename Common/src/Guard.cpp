#include "stdafx.h"

#include "Guard.h"

#include "App.h"

// CRedrawLockGuard

CRedrawLockGuard::CRedrawLockGuard(CWnd& wnd)
	: m_wnd(wnd)
{
	m_wnd.SetRedraw(FALSE);
}

CRedrawLockGuard::~CRedrawLockGuard()
{
	this->Unlock();
}

void CRedrawLockGuard::Unlock()
{
	m_wnd.SetRedraw(TRUE);
	m_wnd.RedrawWindow();
}


class CMenuEx : public CMenu
{
public:
	CMenuEx(UINT uItemHeight, UINT uItemWidth)
		: m_uItemHeight(uItemHeight)
		, m_uItemWidth(uItemWidth)
	{
	}

private:
	UINT m_uItemHeight = 0;
	UINT m_uItemWidth = 0;

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override
	{
		CDC dc;
		dc.Attach(lpDrawItemStruct->hDC);

		CRect rcItem = lpDrawItemStruct->rcItem;
		
		auto crBk = RGB(251, 251, 251);
		if (0 != lpDrawItemStruct->itemID && lpDrawItemStruct->itemState & ODS_SELECTED)
		{
			crBk = RGB(229, 243, 255);
		}

		dc.FillSolidRect(&rcItem, crBk);

		if (0 != lpDrawItemStruct->itemID)
		{
			dc.SetBkMode(TRANSPARENT);

			CString str;
			this->GetMenuString(lpDrawItemStruct->itemID, str, MF_BYCOMMAND);
			dc.DrawText(str, &rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
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

		dc.Detach();
	}

	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) override
	{
		__super::MeasureItem(lpMeasureItemStruct);

		if (0 != lpMeasureItemStruct->itemID)
		{			
			lpMeasureItemStruct->itemHeight = m_uItemHeight;

			lpMeasureItemStruct->itemWidth = m_uItemWidth;
		}
	}
};

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

static void MenuToOwerDraw(CMenu& Menu)
{
	int iItemCount = Menu.GetMenuItemCount();
	for (int iItem = 0; iItem < iItemCount; iItem++)
	{
		UINT id = Menu.GetMenuItemID(iItem);

		CString str;
		Menu.GetMenuString(iItem, str, MF_BYPOSITION);
		(void)Menu.ModifyMenu(iItem, MF_BYPOSITION | MF_OWNERDRAW, id, (LPCTSTR)str);
		
		if (id == -1)//×Ó²Ëµ¥
		{
			MenuToOwerDraw(*Menu.GetSubMenu(iItem)); //µÝ¹éµ÷ÓÃ
		}
	}
}

BOOL CMenuGuard::Popup(UINT uItemHeight, UINT uItemWidth)
{
	HMENU hMenu = m_resModule.loadMenu(m_uIDMenu);
	__AssertReturn(hMenu, FALSE);
	
	CMenuEx SubMenu(uItemHeight, uItemWidth);
	if (!SubMenu.Attach(::GetSubMenu(hMenu, 0)))
	{
		(void)::DestroyMenu(hMenu);
		return FALSE;
	}

	for (auto& pr : m_mapMenuItemInfos)
	{
		auto& uIDItem = pr.first;
		auto& MenuItemInfo = pr.second;
		
		if (!MenuItemInfo.strText.IsEmpty())
		{
			(void)SubMenu.ModifyMenu(uIDItem, MF_BYCOMMAND | MF_STRING, uIDItem, MenuItemInfo.strText);
		}

		if (!MenuItemInfo.bEnable)
		{
			if (!m_bShowDisable)
			{
				(void)SubMenu.RemoveMenu(uIDItem, MF_BYCOMMAND);
			}
			else
			{
				(void)SubMenu.EnableMenuItem(uIDItem, MF_GRAYED);
			}
		}
	}

	m_mapMenuItemInfos.clear();

	int iCount = SubMenu.GetMenuItemCount();
	if (iCount > 0)
	{
		if (0 == SubMenu.GetMenuItemID(0))
		{
			(void)SubMenu.RemoveMenu(0, MF_BYPOSITION);
			iCount--;
		}

		if (iCount > 0)
		{
			if (0 == SubMenu.GetMenuItemID(iCount - 1))
			{
				(void)SubMenu.RemoveMenu(iCount - 1, MF_BYPOSITION);
			}
		}
	}
	
	BOOL bRet = FALSE;
	if (iCount > 0)
	{
		MenuToOwerDraw(SubMenu);

		CPoint ptCursor(0, 0);
		(void)::GetCursorPos(&ptCursor);
		bRet = SubMenu.TrackPopupMenu(0, ptCursor.x, ptCursor.y, &m_wndTarget);
	}

	SubMenu.Detach();

	(void)::DestroyMenu(hMenu);

	return bRet;
}

static int clonePopupMenu(HMENU hDst, HMENU hSrc)
{
	int iCnt = 0;

	for (int iSrc = 0, iDst = GetMenuItemCount(hDst); iSrc<GetMenuItemCount(hSrc); iSrc++)
	{
		CString szMenuStr(L'\0', 256);
		MENUITEMINFO mInfo = { 0 };
		mInfo.cbSize = sizeof(mInfo);
		mInfo.fMask = 0
			| MIIM_CHECKMARKS //Retrieves or sets the hbmpChecked and hbmpUnchecked members. 
			| MIIM_DATA //Retrieves or sets the dwItemData member. 
			| MIIM_ID //Retrieves or sets the wID member. 
			| MIIM_STATE //Retrieves or sets the fState member. 
			| MIIM_SUBMENU //Retrieves or sets the hSubMenu member. 
			| MIIM_TYPE //Retrieves or sets the fType and dwTypeData members. 
			| 0;
		mInfo.dwTypeData = (LPTSTR)(LPCTSTR)szMenuStr;
		mInfo.cch = szMenuStr.GetLength();

		VERIFY(GetMenuItemInfo(hSrc, iSrc, TRUE, &mInfo));

		szMenuStr.Trim();

		if (mInfo.hSubMenu)
		{
			HMENU hSub = CreatePopupMenu();
			clonePopupMenu(hSub, mInfo.hSubMenu);
			mInfo.hSubMenu = hSub;
		}

		InsertMenuItem(hDst, iDst++, TRUE, &mInfo);
		iCnt++;
	}

	return iCnt;
}

BOOL CMenuGuard::PopupEx()
{
	CMenu popupMenu;
	if (!popupMenu.CreatePopupMenu())
	{
		return FALSE;
	}

	HMENU hMenu = m_resModule.loadMenu(m_uIDMenu);
	__AssertReturn(hMenu, FALSE);

	(void)clonePopupMenu(popupMenu.m_hMenu, hMenu);

	(void)::DestroyMenu(hMenu);

	CPoint ptCursor(0, 0);
	(void)::GetCursorPos(&ptCursor);
	return popupMenu.TrackPopupMenu(0, ptCursor.x, ptCursor.y, &m_wndTarget);
}

bool CCompatableFont::create(CWnd& wnd, const CB_CompatableFont& cb)
{
	if (NULL != m_hObject)
	{
		return false;
	}

	CFont *pFont = wnd.GetFont();
	if (NULL == pFont)
	{
		return false;
	}

	LOGFONT logFont;
	::ZeroMemory(&logFont, sizeof(logFont));
	(void)pFont->GetLogFont(&logFont);

	logFont.lfQuality = PROOF_QUALITY;
	wcscpy_s(logFont.lfFaceName, L"Î¢ÈíÑÅºÚ");

	if (logFont.lfHeight > 0)
	{
		logFont.lfHeight += m_iFontSizeOffset;
	}
	else
	{
		logFont.lfHeight -= m_iFontSizeOffset;
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

bool CCompatableFont::create(CWnd& wnd, int iFontSizeOffset, const CB_CompatableFont& cb)
{
	m_iFontSizeOffset = iFontSizeOffset;

	return create(wnd, cb);
}

bool CFontGuard::setFont(CWnd& wnd, ULONG uFontSizeOffset, const CB_CompatableFont& cb)
{
	if (NULL == m_font.m_hObject)
	{
		if (!m_font.create(wnd, uFontSizeOffset, cb))
		{
			return false;
		}
	}

	(void)wnd.SetFont(&m_font);

	return true;
}

bool CFontGuard::setFont(CWnd& wnd, const CB_CompatableFont& cb)
{
	return setFont(wnd, 0, cb);
}
