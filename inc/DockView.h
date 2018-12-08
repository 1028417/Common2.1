
#pragma once

#include "Page.h"

#include "Guide.h"

#define __DXView 4

enum class E_ViewTabStyle
{
	VTS_HideTab = 0,
	VTS_TabTop,
	VTS_TabBottom,
	VTS_TabLeft,
	VTS_TabRight
};

struct tagViewTabStyle
{
	E_ViewTabStyle eTabStyle = E_ViewTabStyle::VTS_HideTab;

	UINT uTabFontSize = 0;

	CImageList *pImglst = NULL;

	UINT uTabHeight = 0;

	UINT uTabWidth = 0;
};

class CTabCtrlEx : public CTabCtrl
{
public:
	CTabCtrlEx()
	{
	}

private:
	CFontGuide m_fontGuide;
	
	E_ViewTabStyle m_eTabStyle = E_ViewTabStyle::VTS_HideTab;

	CImageList m_Imglst;

	int m_iTrackMouseFlag = -1;

	CB_TrackMouseEvent m_cbMouseEvent;

	void OnTrackMouseEvent(E_TrackMouseEvent eMouseEvent, const CPoint& point);

public:
	BOOL init(const tagViewTabStyle& TabStyle);

	void SetTabStyle(E_ViewTabStyle eTabStyle);

	BOOL SetFontSize(UINT uFontSize);

	BOOL SetTabHeight(UINT uTabHeight);

	void SetTrackMouse(const CB_TrackMouseEvent& cbMouseEvent);

private:
	BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;
};

enum class E_ViewType
{
	VT_DockLeft,
	VT_DockTop,
	VT_DockBottom,
	VT_DockRight,
	VT_DockCenter,
};

struct tagViewStyle
{
	tagViewStyle(E_ViewType t_eViewType=E_ViewType::VT_DockCenter)
	{
		eViewType = t_eViewType;
	}

	E_ViewType eViewType;

	UINT uDockSize = 0;
	UINT uMaxDockSize = 0;
	UINT uMinDockSize = 0;

	UINT uStartPos = 0;
	UINT uEndPos = 0;

	tagViewTabStyle TabStyle;

	bool sizeable() const
	{
		return uDockSize != uMaxDockSize || uDockSize != uMinDockSize;
	}

	void setDockSize(UINT t_uDockSize)
	{
		if (0 != uMaxDockSize)
		{
			t_uDockSize = min(t_uDockSize, uMaxDockSize);
		}
		if (0 != uMinDockSize)
		{
			t_uDockSize = max(t_uDockSize, uMinDockSize);
		}
		else
		{
			return;
		}

		uDockSize = t_uDockSize;
	}
};

class CPage;

class __CommonExt CDockView : public CPropertySheet
{
protected:
	typedef vector<CPage*> PageVector;

public:
	CDockView(CWnd& wndParent);
	
	CDockView(CWnd& wndParent, const tagViewStyle& ViewStyle);

	virtual ~CDockView() {}

	DECLARE_MESSAGE_MAP()

private:
	tagViewStyle m_ViewStyle;
	
	CTabCtrlEx m_wndTabCtrl;

	PageVector m_vctPages;

public:
	void setViewStyle(const tagViewStyle& ViewStyle);

	const tagViewStyle& getViewStyle() const
	{
		return m_ViewStyle;
	}

	void setDockSize(UINT uDockSize)
	{
		m_ViewStyle.setDockSize(uDockSize);
	}

	BOOL AddPage(CPage& Page);

	BOOL ActivePage(CPage& pPage);

	BOOL SetPageTitle(CPage& Page, const CString& cstrTitle, int iImage = -1);

	void Resize(CRect& rcViewArea, bool bManual);

private:
	BOOL Create();

	afx_msg void OnSize(UINT nType, int, int);

	void resizePage();
};
