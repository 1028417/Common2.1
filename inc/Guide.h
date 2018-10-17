#pragma once

#include <Page.h>

// CWndRedrawGuide

class __CommonPrjExt CRedrawLockGuide
{
public:
	CRedrawLockGuide(CWnd& wndCtrl);

	~CRedrawLockGuide();

	void Unlock();

private:
	CWnd& m_wndCtrl;
};

class __CommonPrjExt CMenuGuide : public CMenu
{
public:
	CMenuGuide(CPage& Page, UINT nIDMenu);

private:
	CPage& m_Page;
	UINT m_uIDMenu;

	map<UINT, pair<BOOL, CString>> m_mapMenuItemInfos;

public:
	void EnableItem(UINT nIDItem, BOOL bEnable);

	void SetItemText(UINT nIDItem, const CString& cstrText);

	BOOL Popup();
};

using CB_CompatableFont = function<void(LOGFONT& logFont)>;

class CCompatableFont : public CFont
{
public:
	CCompatableFont()
	{
	}

	bool create(CWnd& wnd, const CB_CompatableFont& cb)
	{
		CFont *pFont = wnd.GetFont();
		if (NULL == pFont)
		{
			return false;
		}

		LOGFONT logFont;
		::ZeroMemory(&logFont, sizeof(logFont));
		(void)pFont->GetLogFont(&logFont);

		cb(logFont);

		if (!CreateFontIndirect(&logFont))
		{
			return false;
		}

		return true;
	}
};

class __CommonPrjExt CFontGuide
{
public:
	CFontGuide() {};

	CCompatableFont m_font;

public:
	bool setFontSize(CWnd& wnd, ULONG uFontSize);
};
