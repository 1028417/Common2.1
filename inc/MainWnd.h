#pragma once

#include "App.h"

#include "DockView.h"

#include "Page.h"

//CMainWnd

class CDockView;

class CPage;

struct tagMainWndInfo
{
	wstring strText;
	HICON hIcon = NULL;
	HMENU hMenu = NULL;

	BOOL bSizeable = FALSE;

	ULONG uWidth = 0;
	ULONG uHeight = 0;

	ULONG uMinWidth = 0;
	ULONG uMinHeight = 0;
};

class __CommonPrjExt CMainWnd: public CWnd
{
protected:
	typedef vector<CDockView*> TD_DockViewVector;

public:
	CMainWnd()
	{
	}

	virtual ~CMainWnd()
	{
		for (auto pView : m_vctDockViews)
		{
			delete pView;
		}
	}
	
	DECLARE_MESSAGE_MAP()

protected:
	vector<int> m_vctStatusPartWidth;

	CStatusBarCtrl m_ctlStatusBar;

	CRect m_rtBlankArea;

	TD_DockViewVector m_vctDockViews;
	
	tagMainWndInfo m_WndInfo;

public:
	virtual BOOL Create(tagMainWndInfo& MainWndInfo);

	virtual void Show();

	BOOL CreateStatusBar(UINT nParts, ...);
	
	BOOL SetStatusText(UINT nPart, const CString& cstrText);

	BOOL AddDockView(CPage& Page, ST_ViewStyle nStyle, UINT nDockSize
		, UINT uOffset = 0, UINT uTabFontSize=0, UINT uTabHeight = 0);

	BOOL AddDockView(CPage& Page, ST_ViewStyle nStyle, UINT nDockSize
		, UINT uOffset, UINT uTabFontSize, CImageList *pImglst=NULL);

	BOOL AddUndockView(CPage& Page, const CRect& rtPos);

	BOOL AddPage(CPage& Page, ST_ViewStyle nStyle);

	BOOL ActivePage(CPage& Page);

	BOOL SetPageTitle(CPage& Page, const CString& cstrTitle, int iImage = -1);
		
	int MsgBox(const CString& cstrText, const CString& cstrTitle=L"", UINT uType=MB_OK);
	
private:
	BOOL _AddView(CDockView& View, CPage& Page);

	void OnSize();

	CDockView* GetDockView(const CPoint& ptPos);

	BOOL HandleResizeViewMessage(UINT message, WPARAM wParam, LPARAM lParam);
	void ResizeView(CDockView &wndTargetView, CPoint &ptPos);

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

	BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};
