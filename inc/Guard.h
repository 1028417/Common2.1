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

class __CommonExt CMenuGuard
{
public:
	CMenuGuard(CWnd& wndTarget, CResModule& resModule, UINT uIDMenu, BOOL bShowDisable=FALSE)
		: m_wndTarget(wndTarget)
		, m_uIDMenu(uIDMenu)
		, m_resModule(resModule)
		, m_bShowDisable(bShowDisable)
	{
	}

private:
	CWnd& m_wndTarget;

	CResModule& m_resModule;

	UINT m_uIDMenu = 0;
	
	BOOL m_bShowDisable = FALSE;

	map<UINT, tagMenuItemInfo> m_mapMenuItemInfos;

public:
	void EnableItem(UINT uIDItem, BOOL bEnable);
	void EnableItem(const std::initializer_list<UINT>& ilIDItems, BOOL bEnable);

	void DisableItem(UINT uIDItem);
	void DisableItem(const std::initializer_list<UINT>& ilIDItems, BOOL bEnable);

	void DeleteItem(UINT uIDItem);
	void DeleteItem(const std::initializer_list<UINT>& ilIDItems);

	void SetItemText(UINT uIDItem, const CString& cstrText);
	
	BOOL Popup(UINT uItemHeight, UINT uItemWidth=200);

	BOOL PopupEx();
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
