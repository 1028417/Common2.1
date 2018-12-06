
#pragma once

#include "Page.h"

#include "Guide.h"

#define __DXView 5

enum ViewStyle
{
	VS_FixSize		= 0x1000,
	VS_DockCenter	= 0x2000,

	VS_DockLeft		= 0x0001,
	VS_DockTop		= 0x0002,
	VS_DockBottom	= 0x0004,
	VS_DockRight	= 0x0008,
	
	VS_TabTop		= 0x0010,
	VS_TabBottom	= 0x0020,
	VS_TabLeft		= 0x0040,
	VS_TabRight		= 0x0080,
};
typedef UINT ST_ViewStyle;

enum class E_ViewDockStyle
{
	VDS_NoDock = 0,
	VDS_DockCenter = VS_DockCenter,
	VDS_DockLeft = VS_DockLeft,
	VDS_DockTop = VS_DockTop,
	VDS_DockBottom = VS_DockBottom,
	VDS_DockRight = VS_DockRight,
};

enum class E_ViewTabStyle
{
	VTS_HideTab = 0,
	VTS_TabTop = VS_TabTop,
	VTS_TabBottom = VS_TabBottom,
	VTS_TabLeft = VS_TabLeft,
	VTS_TabRight = VS_TabRight
};

#define __DockStyle(_Style) E_ViewDockStyle(_Style & (VS_DockCenter | VS_DockLeft | VS_DockTop | VS_DockBottom | VS_DockRight))

#define __TabStyle(_Style) E_ViewTabStyle(_Style & (VS_TabTop | VS_TabBottom | VS_TabLeft | VS_TabRight))

class CTabCtrlEx : public CTabCtrl
{
public:
	CTabCtrlEx()
	{
	}

private:
	CFontGuide m_fontGuide;
	
	E_ViewTabStyle m_eTabStyle = (E_ViewTabStyle)0;

	CImageList m_Imglst;

	int m_iTrackMouseFlag = -1;

	CB_TrackMouseEvent m_cbMouseEvent;

	void OnTrackMouseEvent(E_TrackMouseEvent eMouseEvent, const CPoint& point);

public:
	void SetTabStyle(E_ViewTabStyle eTabStyle, UINT cx=0, UINT cy=0);

	BOOL SetFontSize(UINT uFontSize);

	BOOL SetTabHeight(UINT uTabHeight);

	void SetTrackMouse(const CB_TrackMouseEvent& cbMouseEvent);

private:
	BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;
};

class CPage;

class __CommonExt CDockView: public CPropertySheet
{
protected:
	typedef vector<CPage*> PageVector;

public:
	CDockView(CWnd& wndParent, ST_ViewStyle eStyle, UINT eDockSize = 0
		, UINT uOffset = 0, UINT uTabFontSize=0, UINT uTabHeight=0);

	CDockView(CWnd& wndParent, ST_ViewStyle eStyle, UINT uDockSize
		, UINT uOffset = 0, UINT uTabFontSize = 0, CImageList *pImglst=NULL);

	CDockView(CWnd& wndParent, ST_ViewStyle eStyle, const CRect& rtPos);

	virtual ~CDockView() {}

	DECLARE_MESSAGE_MAP()

public:
	ST_ViewStyle m_eStyle = 0;

	UINT m_uDockSize = 0;

private:
	UINT m_uOffset = 0;
	
	CTabCtrlEx m_wndTabCtrl;

	UINT m_uTabFontSize = 0;
	UINT m_uTabHeight = 0;

	CRect m_rtPos;

	CImageList *m_pImglst = NULL;

	PageVector m_vctPages;

public:
	BOOL AddPage(CPage& Page);

	BOOL ActivePage(CPage& pPage);

	BOOL SetPageTitle(CPage& Page, const CString& cstrTitle, int iImage = -1);

	void Resize(CRect& rcRestrict);

private:
	BOOL Create();

	afx_msg void OnSize(UINT nType, int, int);

	void OnSize();
};
