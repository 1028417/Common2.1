
#pragma once

#include "Page.h"

#include "Guide.h"

#define __DXView 5

enum ViewStyle
{
	VS_Undock = 0,

	VS_DockLeft		= 0x0001,
	VS_DockTop		= 0x0002,
	VS_DockBottom	= 0x0004,
	VS_DockRight	= 0x0008,
	VS_DockCenter	= 0x0010,
	
	VS_FixSize		= 0x0020, 

	VS_TopTab		= 0x0040,
	VS_BottomTab	= 0x0080,
};

typedef UINT ST_ViewStyle;

#define __DockStyle(_Style) (_Style & (VS_DockLeft | VS_DockTop | VS_DockBottom | VS_DockRight | VS_DockCenter))

#define VS_Tab (VS_TopTab|VS_BottomTab)
#define __TabStyle(_Style) (_Style & VS_Tab)

//CDockView

class CPage;

class CTabCtrlEx : public CTabCtrl
{
public:
	CTabCtrlEx()
	{
	}

private:
	int m_iTrackMouseFlag = -1;

	CB_TrackMouseEvent m_cbMouseEvent;

	void OnTrackMouseEvent(E_TrackMouseEvent eMouseEvent, const CPoint& point);

public:
	void SetTrackMouse(const CB_TrackMouseEvent& cbMouseEvent);

	BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;
};

class CDockView: public CPropertySheet
{
protected:
	typedef vector<CPage*> PageVector;

public:
	CDockView(CWnd& wndParent, ST_ViewStyle nStyle, UINT nDockSize
		, UINT uOffset = 0, UINT uTabFontSize=0, UINT uTabHeight=0);

	CDockView(CWnd& wndParent, ST_ViewStyle nStyle, UINT nDockSize
		, UINT uOffset = 0, UINT uTabFontSize = 0, CImageList *pImglst=NULL);

	CDockView(CWnd& wndParent, ST_ViewStyle nStyle, const CRect& rtPos);

	virtual ~CDockView() {}

	DECLARE_MESSAGE_MAP()

public:
	ST_ViewStyle m_nStyle = 0;

	UINT m_nDockSize = 0;

private:
	UINT m_uOffset = 0;
	
	CTabCtrlEx m_wndTabCtrl;

	UINT m_uTabFontSize = 0;
	UINT m_uTabHeight = 0;

	CRect m_rtPos;

	CImageList *m_pImglst = NULL;

	CImageList m_InnerImglst;

	PageVector m_vctPages;

	CFontGuide m_fontGuide;

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
