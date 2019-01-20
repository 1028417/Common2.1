#pragma once

#include "App.h"

#include "DockView.h"

#include "Page.h"

using CB_Sync = fn_voidvoid;

//CMainWnd

class CDockView;

class CPage;

struct tagMainWndInfo
{
	HICON hIcon = NULL;
	wstring strText;
	HMENU hMenu = NULL;

	BOOL bSizeable = FALSE;

	ULONG uWidth = 0;
	ULONG uHeight = 0;

	ULONG uMinWidth = 0;
	ULONG uMinHeight = 0;
};

class __CommonExt CMainWnd: public CWnd
{
public:
	CMainWnd(){}

	virtual ~CMainWnd()
	{
		for (auto& pr : m_mapDockViews)
		{
			delete pr.second;
		}
	}
	
	DECLARE_MESSAGE_MAP()

protected:
	tagMainWndInfo m_WndInfo;

private:	
	vector<int> m_vctStatusPartWidth;
	CStatusBarCtrl m_ctlStatusBar;
	UINT m_uStatusBarHeight = 0;

	int m_cx = 0;
	int m_cy = 0;

	SMap<E_DockViewType, CDockView*> m_mapDockViews;

	CB_Async m_cbAsync;
	NS_mtutil::CCSLock m_ccsLock;

public:
	virtual BOOL Create(tagMainWndInfo& MainWndInfo);

	BOOL CreateStatusBar(UINT uParts, ...);
	
	BOOL SetStatusText(UINT uPart, const CString& cstrText);

	BOOL AddView(CPage& Page, const tagViewStyle& ViewStyle);

	BOOL AddPage(CPage& Page, E_DockViewType eViewType);

	BOOL ActivePage(CPage& Page);

	BOOL SetPageTitle(CPage& Page, const CString& cstrTitle, int iImage = -1);

	int MsgBox(const CString& cstrText, const CString& cstrTitle, UINT uType = MB_OK);
	int MsgBox(const CString& cstrText, UINT uType = MB_OK)
	{
		return MsgBox(cstrText, L"", uType);
	}

	void Async(const CB_Async& cb);
	void Sync(const CB_Sync& cb);

	void show();

private:
	BOOL _AddView(CDockView& View, CPage& Page);

	void resizeView(bool bManual=false);

	virtual void onViewResize(CDockView& view) {}

	CDockView* hittestView(const CPoint& ptPos);

	BOOL HandleResizeViewMessage(UINT message, WPARAM wParam, LPARAM lParam);
	void setDockSize(CDockView &wndTargetView, UINT x, UINT y);
	
	void setAsyncCB(const CB_Async& cb);

	void fixWorkArea();

public:
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

	BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};
