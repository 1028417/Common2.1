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
	CRedrawLockGuard(CWnd& wnd, bool bFlag = false);

	~CRedrawLockGuard();

	void Unlock();

private:
	CWnd& m_wnd;

	bool m_bFlag = false;

	BOOL m_bLocked = FALSE;
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
	CMenuEx(UINT uItemHeight, UINT uMenuWidth, float fFontSize, float fTopFontSize, HMENU hMenuAttach)
		: m_uItemHeight(uItemHeight)
		, m_uMenuWidth(uMenuWidth)
		, m_fFontSize(fFontSize)
		, m_fTopFontSize(fTopFontSize)
	{
		if (0 == m_fTopFontSize)
		{
			m_fTopFontSize = m_fFontSize;
		}

		Attach(hMenuAttach, TRUE);
	}

	CMenuEx(UINT uItemHeight, UINT uMenuWidth, float fFontSize=0, HMENU hMenuAttach=NULL)
		: m_uItemHeight(uItemHeight)
		, m_uMenuWidth(uMenuWidth)
		, m_fFontSize(fFontSize)
	{
		if (NULL != hMenuAttach)
		{
			Attach(hMenuAttach, FALSE);
		}
	}

	CMenuEx(CMenuEx& other)
		: m_uItemHeight(other.m_uItemHeight)
		, m_uMenuWidth(other.m_uMenuWidth)
		, m_fFontSize(other.m_fFontSize)
		, m_fTopFontSize(other.m_fTopFontSize)
	{
		(void)Attach(other);
	}

	CMenuEx(CMenuEx&& other)
		: m_uItemHeight(other.m_uItemHeight)
		, m_uMenuWidth(other.m_uMenuWidth)
		, m_fFontSize(other.m_fFontSize)
		, m_fTopFontSize(other.m_fTopFontSize)
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

	float m_fFontSize = 0;
	float m_fTopFontSize = 0;

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
	BOOL _popup(HMENU hMenu, CWnd *pWnd, UINT uItemHeight, float fFontSize);
	
public:
	void EnableItem(UINT uIDItem, BOOL bEnable);
	void EnableItem(const std::initializer_list<UINT>& ilIDItems, BOOL bEnable);

	void DisableItem(UINT uIDItem);
	void DisableItem(const std::initializer_list<UINT>& ilIDItems, BOOL bEnable);

	void DeleteItem(UINT uIDItem);
	void DeleteItem(const std::initializer_list<UINT>& ilIDItems);

	void SetItemText(UINT uIDItem, const CString& cstrText);
	
	BOOL Popup(CWnd *pWnd, UINT uItemHeight, float fFontSize=0);
	BOOL PopupEx(CWnd *pWnd, UINT uItemHeight, float fFontSize=0);
};

using CB_CompatableFont = function<void(LOGFONT&)>;

class __CommonExt CCompatableFont : public CFont
{
public:
	CCompatableFont(float fFontSizeOffset=0)
		: m_fFontSizeOffset(fFontSizeOffset)
	{
	}

private:
	float m_fFontSizeOffset = 0;

public:
	bool create(CFont& font, const CB_CompatableFont& cb = NULL);

	bool create(CDC& dc, const CB_CompatableFont& cb = NULL);

	bool create(CWnd& wnd, const CB_CompatableFont& cb = NULL);

	bool create(CWnd& wnd, float fFontSizeOffset, const CB_CompatableFont& cb = NULL);

	bool setFont(CWnd& wnd, float fFontSizeOffset);
};
