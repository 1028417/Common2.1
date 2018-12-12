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


CMenuGuard::CMenuGuard(CPage& Page, UINT uIDMenu)
	: m_Page(Page)
	, m_uIDMenu(uIDMenu)
{
}

void CMenuGuard::EnableItem(UINT uIDItem, BOOL bEnable)
{
	m_mapMenuItemInfos[uIDItem].first = bEnable;
}

void CMenuGuard::SetItemText(UINT uIDItem, const CString& cstrText)
{
	m_mapMenuItemInfos[uIDItem].second = cstrText;
}

BOOL CMenuGuard::Popup()
{
	if (!*this)
	{
		CResGuard ResGuard (m_Page.m_resModule);

		__AssertReturn(__super::LoadMenu(m_uIDMenu), FALSE);
	}

	CMenu *pSubMenu = __super::GetSubMenu(0);
	__AssertReturn(pSubMenu, FALSE);

	UINT uIDItem = 0;
	BOOL bEnable = FALSE;
	CString *pcstrText = NULL;
	for (map<UINT, pair<BOOL, CString>>::iterator itrMenuItemInfo = m_mapMenuItemInfos.begin()
		; itrMenuItemInfo != m_mapMenuItemInfos.end(); ++itrMenuItemInfo)
	{
		uIDItem = itrMenuItemInfo->first;
		bEnable = itrMenuItemInfo->second.first;
		pcstrText = &itrMenuItemInfo->second.second;

		if (!pcstrText->IsEmpty())
		{
			__AssertReturn(pSubMenu->ModifyMenu(uIDItem, MF_BYCOMMAND | MF_STRING, uIDItem, *pcstrText), FALSE);
		}

		(void)pSubMenu->EnableMenuItem(uIDItem, bEnable ? MF_ENABLED : MF_GRAYED);
	}

	m_mapMenuItemInfos.clear();

	CPoint ptCursor(0, 0);
	(void)::GetCursorPos(&ptCursor);

	return pSubMenu->TrackPopupMenu(0, ptCursor.x, ptCursor.y, &m_Page);
}


bool CFontGuard::setFontSize(CWnd& wnd, ULONG uFontSize)
{
	if (NULL == m_font.m_hObject)
	{
		if (!m_font.create(wnd, [uFontSize](LOGFONT& logFont) {
			if (logFont.lfHeight < 0)
			{
				logFont.lfHeight -= uFontSize;
			}
			else
			{
				logFont.lfHeight += uFontSize;
			}
		}))
		{
			return false;
		}
	}

	wnd.SetFont(&m_font);

	return true;
}
