#pragma once

#include <Page.h>

template <class T>
class CTouchWnd : public T
{
public:
	CTouchWnd() {}

protected:
	virtual ULONG GetGestureStatus(CPoint ptTouch) override { return 0; }
};

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

struct tagMenuItemInfo
{
	BOOL bEnable = TRUE;

	BOOL bDelete = FALSE;

	CString strText;
};

class __CommonExt CMenuEx : public CMenu
{
public:
	CMenuEx(UINT uItemHeight, UINT uMenuWidth, UINT uFontSize, UINT uTopFontSize, HMENU hMenuAttach)
		: m_uItemHeight(uItemHeight)
		, m_uMenuWidth(uMenuWidth)
		, m_uFontSize(uFontSize)
		, m_uTopFontSize(uTopFontSize)
	{
		if (0 == m_uTopFontSize)
		{
			m_uTopFontSize = m_uFontSize;
		}

		Attach(hMenuAttach, TRUE);
	}

	CMenuEx(UINT uItemHeight, UINT uMenuWidth, UINT uFontSize=0, HMENU hMenuAttach=NULL)
		: m_uItemHeight(uItemHeight)
		, m_uMenuWidth(uMenuWidth)
		, m_uFontSize(uFontSize)
	{
		if (NULL != hMenuAttach)
		{
			Attach(hMenuAttach, FALSE);
		}
	}

	CMenuEx(CMenuEx& other)
		: m_uItemHeight(other.m_uItemHeight)
		, m_uMenuWidth(other.m_uMenuWidth)
		, m_uFontSize(other.m_uFontSize)
	{
		(void)Attach(other);
	}

	CMenuEx(CMenuEx&& other)
		: m_uItemHeight(other.m_uItemHeight)
		, m_uMenuWidth(other.m_uMenuWidth)
		, m_uFontSize(other.m_uFontSize)
	{
		(void)Attach(other);
	}

	~CMenuEx()
	{
		Detach();
	}

	operator bool () const
	{
		return NULL != m_hMenuAttach;
	}

private:
	UINT m_uItemHeight = 0;
	UINT m_uMenuWidth = 0;

	UINT m_uFontSize = 0;
	UINT m_uTopFontSize = 0;

	BOOL m_bTopMenu = FALSE;

	HMENU m_hMenuAttach = NULL;
	
	list<CMenuEx> m_lstSubMenu;

private:
	void _setOwerDraw();

public:
	HMENU Detach();

	BOOL Attach(HMENU hMenu, BOOL bTopMenu);

	BOOL Attach(CMenuEx& other)
	{
		(void)Detach();

		return Attach(other.Detach(), m_bTopMenu);
	}

	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) override;

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;
};

class __CommonExt CMenuGuard
{
public:
	CMenuGuard(CResModule& resModule, UINT uIDMenu, UINT uMenuWidth, BOOL bShowDisable=FALSE)
		: m_uIDMenu(uIDMenu)
		, m_resModule(resModule)
		, m_uMenuWidth(uMenuWidth)
		, m_bShowDisable(bShowDisable)
	{
	}

private:
	CResModule& m_resModule;

	UINT m_uIDMenu = 0;
	
	BOOL m_bShowDisable = FALSE;

	UINT m_uMenuWidth = 0;

	map<UINT, tagMenuItemInfo> m_mapMenuItemInfos;

private:
	BOOL _popup(HMENU hMenu, CWnd *pWnd, UINT uItemHeight, UINT uFontSize);
	
public:
	void EnableItem(UINT uIDItem, BOOL bEnable);
	void EnableItem(const std::initializer_list<UINT>& ilIDItems, BOOL bEnable);

	void DisableItem(UINT uIDItem);
	void DisableItem(const std::initializer_list<UINT>& ilIDItems, BOOL bEnable);

	void DeleteItem(UINT uIDItem);
	void DeleteItem(const std::initializer_list<UINT>& ilIDItems);

	void SetItemText(UINT uIDItem, const CString& cstrText);
	
	BOOL Popup(CWnd *pWnd, UINT uItemHeight, UINT uFontSize=0);
	BOOL PopupEx(CWnd *pWnd, UINT uItemHeight, UINT uFontSize=0);
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
	bool create(CFont& font, const CB_CompatableFont& cb = NULL);

	bool create(CDC& dc, const CB_CompatableFont& cb = NULL);

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
