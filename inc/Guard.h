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
		
		logFont.lfQuality = PROOF_QUALITY;
		wcscpy_s(logFont.lfFaceName, L"Î¢ÈíÑÅºÚ");

		cb(logFont);

		if (!CreateFontIndirect(&logFont))
		{
			return false;
		}

		return true;
	}
};

class __CommonExt CFontGuard
{
public:
	CFontGuard() {};

	CCompatableFont m_font;

public:
	bool setFontSize(CWnd& wnd, ULONG uFontSize);
};
