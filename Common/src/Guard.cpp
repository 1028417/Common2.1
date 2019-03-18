#include "stdafx.h"

#include "Guard.h"

#include "App.h"

#include <afxglobals.h>

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

void CMenuEx::_setOwerDraw()
{
	int iItemCount = this->GetMenuItemCount();
	for (int iItem = 0; iItem < iItemCount; iItem++)
	{
		UINT uItemID = this->GetMenuItemID(iItem);
		
		if (-1 == uItemID)//�Ӳ˵�
		{
			m_lstSubMenu.push_back(CMenuEx(m_uItemHeight, m_uMenuWidth, m_uFontSize));
			m_lstSubMenu.back().Attach(this->GetSubMenu(iItem)->m_hMenu, FALSE); //�ݹ����
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

	_setOwerDraw();

	return TRUE;
}

void CMenuEx::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	__super::MeasureItem(lpMeasureItemStruct);

	if (0 != lpMeasureItemStruct->itemID)
	{
		lpMeasureItemStruct->itemHeight = m_uItemHeight;

		lpMeasureItemStruct->itemWidth = m_uMenuWidth;
		if (m_bTopMenu)
		{
			lpMeasureItemStruct->itemWidth += 6;
		}
	}
	else
	{
		lpMeasureItemStruct->itemHeight = 5;
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
		CCompatableFont font(m_uFontSize);
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

BOOL CMenuGuard::Popup(CWnd *pWnd, UINT uItemHeight, UINT uFontSize)
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
		if (0 == ::GetMenuItemID(hSubMenu, 0))
		{
			(void)::RemoveMenu(hSubMenu, 0, MF_BYPOSITION);
			iCount--;
		}

		if (iCount > 0)
		{
			if (0 == ::GetMenuItemID(hSubMenu, iCount - 1))
			{
				(void)::RemoveMenu(hSubMenu, iCount - 1, MF_BYPOSITION);
			}
		}
	}
	
	BOOL bRet = FALSE;
	if (iCount > 0)
	{
		CMenuEx SubMenu(uItemHeight + 2, m_uMenuWidth, uFontSize, hSubMenu);

		CPoint ptCursor(0, 0);
		(void)::GetCursorPos(&ptCursor);
		bRet = SubMenu.TrackPopupMenu(0, ptCursor.x, ptCursor.y, pWnd);
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

BOOL CMenuGuard::PopupEx(CWnd *pWnd)
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
	return popupMenu.TrackPopupMenu(0, ptCursor.x, ptCursor.y, pWnd);
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

	logFont.lfQuality = PROOF_QUALITY;
	wcscpy_s(logFont.lfFaceName, L"΢���ź�");

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

	return create(*pFont, cb);
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
