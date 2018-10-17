#include "stdafx.h"

#include "Guide.h"

#include "App.h"

// CRedrawLockGuide

CRedrawLockGuide::CRedrawLockGuide(CWnd& wndCtrl)
	: m_wndCtrl(wndCtrl)
{
	m_wndCtrl.SetRedraw(FALSE);
}

CRedrawLockGuide::~CRedrawLockGuide()
{
	this->Unlock();
}

void CRedrawLockGuide::Unlock()
{
	m_wndCtrl.SetRedraw(TRUE);
}


CMenuGuide::CMenuGuide(CPage& Page, UINT nIDMenu)
	: m_Page(Page)
	, m_uIDMenu(nIDMenu)
{
}

void CMenuGuide::EnableItem(UINT nIDItem, BOOL bEnable)
{
	m_mapMenuItemInfos[nIDItem].first = bEnable;
}

void CMenuGuide::SetItemText(UINT nIDItem, const CString& cstrText)
{
	m_mapMenuItemInfos[nIDItem].second = cstrText;
}

BOOL CMenuGuide::Popup()
{
	CMainApp::DoEvents();

	if (!*this)
	{
		CResourceLock ResourceLock(m_Page.m_Module);

		__AssertReturn(__super::LoadMenu(m_uIDMenu), FALSE);
	}

	CMenu *pSubMenu = __super::GetSubMenu(0);
	__AssertReturn(pSubMenu, FALSE);

	UINT nIDItem = 0;
	BOOL bEnable = FALSE;
	CString *pcstrText = NULL;
	for (map<UINT, pair<BOOL, CString>>::iterator itrMenuItemInfo = m_mapMenuItemInfos.begin()
		; itrMenuItemInfo != m_mapMenuItemInfos.end(); ++itrMenuItemInfo)
	{
		nIDItem = itrMenuItemInfo->first;
		bEnable = itrMenuItemInfo->second.first;
		pcstrText = &itrMenuItemInfo->second.second;

		if (!pcstrText->IsEmpty())
		{
			__AssertReturn(pSubMenu->ModifyMenu(nIDItem, MF_BYCOMMAND | MF_STRING, nIDItem, *pcstrText), FALSE);
		}

		(void)pSubMenu->EnableMenuItem(nIDItem, bEnable ? MF_ENABLED : MF_GRAYED);
	}

	m_mapMenuItemInfos.clear();

	CPoint ptCursor(0, 0);
	(void)::GetCursorPos(&ptCursor);

	return pSubMenu->TrackPopupMenu(0, ptCursor.x, ptCursor.y, &m_Page);
}


bool CFontGuide::setFontSize(CWnd& wnd, ULONG uFontSize)
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
