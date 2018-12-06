#pragma once

#include <Page.h>

class __CommonExt CResGuide
{
public:
	CResGuide(CResModule& resModule)
	{
		m_hPreInstance = AfxGetResourceHandle();
		resModule.ActivateResource();
	}

	CResGuide(CResModule *pResModule)
	{
		if (NULL != pResModule)
		{
			m_hPreInstance = AfxGetResourceHandle();
			pResModule->ActivateResource();
		}
	}

	~CResGuide()
	{
		if (NULL != m_hPreInstance)
		{
			AfxSetResourceHandle(m_hPreInstance);
		}
	}

private:
	HINSTANCE m_hPreInstance = NULL;
};

class __CommonExt CRedrawLockGuide
{
public:
	CRedrawLockGuide(CWnd& wnd);

	~CRedrawLockGuide();

	void Unlock();

private:
	CWnd& m_wnd;
};

class __CommonExt CMenuGuide : public CMenu
{
public:
	CMenuGuide(CPage& Page, UINT uIDMenu);

private:
	CPage& m_Page;
	UINT m_uIDMenu;

	map<UINT, pair<BOOL, CString>> m_mapMenuItemInfos;

public:
	void EnableItem(UINT uIDItem, BOOL bEnable);

	void SetItemText(UINT uIDItem, const CString& cstrText);

	BOOL Popup();
};

using CB_CompatableFont = function<void(LOGFONT&)>;

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

class __CommonExt CFontGuide
{
public:
	CFontGuide() {};

	CCompatableFont m_font;

public:
	bool setFontSize(CWnd& wnd, ULONG uFontSize);
};
