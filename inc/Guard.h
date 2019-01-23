#pragma once

#include <Page.h>

class __CommonExt CResGuard
{
public:
	CResGuard(CResModule& resModule)
	{
		m_hPreInstance = AfxGetResourceHandle();
		resModule.ActivateResource();
	}

	CResGuard(CResModule *pResModule)
	{
		if (NULL != pResModule)
		{
			m_hPreInstance = AfxGetResourceHandle();
			pResModule->ActivateResource();
		}
	}

	~CResGuard()
	{
		if (NULL != m_hPreInstance)
		{
			AfxSetResourceHandle(m_hPreInstance);
		}
	}

private:
	HINSTANCE m_hPreInstance = NULL;
};

class __CommonExt CRedrawLockGuard
{
public:
	CRedrawLockGuard(CWnd& wnd);

	~CRedrawLockGuard();

	void Unlock();

private:
	CWnd& m_wnd;
};

class __CommonExt CMenuGuard : public CMenu
{
public:
	CMenuGuard(CPage& Page, UINT uIDMenu);

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

class __CommonExt CCompatableFont : public CFont
{
public:
	CCompatableFont(int iFontSizeOffset=0)
		: m_iFontSizeOffset(iFontSizeOffset)
	{
	}

private:
	int m_iFontSizeOffset = 0;

public:
	bool create(CWnd& wnd, const CB_CompatableFont& cb = NULL);

	bool create(CWnd& wnd, int iFontSizeOffset, const CB_CompatableFont& cb = NULL);
};

class __CommonExt CFontGuard
{
public:
	CFontGuard() {};

	CCompatableFont m_font;

public:
	bool setFont(CWnd& wnd, ULONG uFontSizeOffset, const CB_CompatableFont& cb=NULL);

	bool setFont(CWnd& wnd, const CB_CompatableFont& cb);
};
