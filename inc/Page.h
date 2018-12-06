
#pragma once

#include "DragDrop.h"

#include "App.h"


class CModuleApp;

class CMainWnd;

using CB_AsyncLoop = function<bool()>;

class __CommonExt CPage: public CPropertyPage
{
	friend class CDockView;

public:
	CPage(CResModule& resModule, UINT uIDDlgRes, const CString& cstrTitle=L"", bool bAutoActive=false);

	virtual ~CPage() {}

	virtual void DoDataExchange(CDataExchange* pDX);

	CResModule& m_resModule;

private:
	CString m_cstrTitle;

	bool m_bAutoActive;

private:
	set<HWND> m_setDragableCtrls;

	CB_Async m_cbAsync;

	CB_AsyncLoop m_cbAsyncLoop;

	void _AsyncLoop(UINT uDelayTime);

public:
	BOOL Active();

	BOOL SetTitle(const CString& cstrTitle, int iImage = -1);

	virtual int MsgBox(const CString& cstrText, UINT uType=MB_OK);

	void Async(const CB_Async& cb, UINT uDelayTime=0);
	void AsyncLoop(UINT uDelayTime, const CB_AsyncLoop& cb);

protected:
	BOOL OnSetActive();
	BOOL OnKillActive();

	virtual void OnActive(BOOL bActive);

	BOOL RegDragableCtrl(CWnd& wndCtrl)
	{
		__AssertReturn(wndCtrl.GetSafeHwnd(), FALSE);

		m_setDragableCtrls.insert(wndCtrl.GetSafeHwnd());
		
		return TRUE;
	}

	virtual BOOL GetCtrlDragData(CWnd *pwndCtrl, const CPoint& point, LPVOID& pDragData)
	{
		return FALSE;
	}
	
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;
};
