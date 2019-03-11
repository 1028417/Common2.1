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


CMenuGuard::CMenuGuard(UINT uIDMenu)
	: m_uIDMenu(uIDMenu)
{
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

BOOL CMenuGuard::Popup(CResModule& resModule, CWnd *pWnd)
{
	CResGuard ResGuard(resModule);
	CMenu menu;
	__AssertReturn(menu.LoadMenu(m_uIDMenu), FALSE);

	CMenu popupMenu;
	if (!popupMenu.CreatePopupMenu())
	{
		return FALSE;
	}

	(void)clonePopupMenu(popupMenu.m_hMenu, menu.m_hMenu);

	CPoint ptCursor(0, 0);
	(void)::GetCursorPos(&ptCursor);
	return popupMenu.TrackPopupMenu(0, ptCursor.x, ptCursor.y, pWnd);
}

BOOL CMenuGuard::Popup(CPage& Page, BOOL bShowDisable)
{
	CResGuard ResGuard(Page.m_resModule);
	CMenu menu;
	__AssertReturn(menu.LoadMenu(m_uIDMenu), FALSE);
	
	CMenu *pSubMenu = menu.GetSubMenu(0);
	__AssertReturn(pSubMenu, FALSE);

	for (auto& pr : m_mapMenuItemInfos)
	{
		auto& uIDItem = pr.first;
		auto& MenuItemInfo = pr.second;
		
		if (!MenuItemInfo.strText.IsEmpty())
		{
			__AssertReturn(pSubMenu->ModifyMenu(uIDItem, MF_BYCOMMAND | MF_STRING, uIDItem, MenuItemInfo.strText), FALSE);
		}

		if (!MenuItemInfo.bEnable)
		{
			if (!bShowDisable)
			{
				(void)pSubMenu->RemoveMenu(uIDItem, MF_BYCOMMAND);
			}
			else
			{
				(void)pSubMenu->EnableMenuItem(uIDItem, MF_GRAYED);
			}
		}
	}

	m_mapMenuItemInfos.clear();

	int iCount = pSubMenu->GetMenuItemCount();
	if (iCount > 0)
	{
		if (0 == pSubMenu->GetMenuItemID(0))
		{
			(void)pSubMenu->RemoveMenu(0, MF_BYPOSITION);
			iCount--;
		}

		if (iCount > 0)
		{
			if (0 == pSubMenu->GetMenuItemID(iCount - 1))
			{
				(void)pSubMenu->RemoveMenu(iCount - 1, MF_BYPOSITION);
			}
		}
	}
	
	if (iCount <= 0)
	{
		return TRUE;
	}

	CPoint ptCursor(0, 0);
	(void)::GetCursorPos(&ptCursor);
	return pSubMenu->TrackPopupMenu(0, ptCursor.x, ptCursor.y, &Page);
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
