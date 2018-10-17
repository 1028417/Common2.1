
#pragma once

#include "DragDrop.h"

#include "App.h"


class IModuleApp;

class CMainWnd;

using CB_Async = function<void()>;
using CB_AsyncLoop = function<bool()>;

class __CommonPrjExt CPage: public CPropertyPage
{
	friend class CDockView;

	DECLARE_DYNAMIC(CPage);

	DECLARE_MESSAGE_MAP();

public:
	CPage(IModuleApp& Module, UINT nIDDlgRes, const CString& cstrTitle=L"", bool bAutoActive=false);

	virtual ~CPage() {}

	virtual void DoDataExchange(CDataExchange* pDX);

	IModuleApp& m_Module;

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
	void AsyncLoop(const CB_AsyncLoop& cb, UINT uDelayTime);

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
