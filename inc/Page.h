
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

	bool m_bAutoActive = false;

	set<HWND> m_setDragableCtrls;

	UINT_PTR m_idTimer = 0;

	CB_AsyncLoop m_cbAsyncLoop;

private:
	virtual bool onAsyncLoop();

public:
	BOOL Active();

	BOOL SetTitle(const CString& cstrTitle, int iImage = -1);

	virtual int MsgBox(const CString& cstrText, UINT uType=MB_OK);

protected:
	BOOL OnSetActive() override;
	BOOL OnKillActive() override;

	virtual void OnActive(BOOL bActive) {}

	virtual BOOL PreTranslateMessage(MSG* pMsg) override;

	virtual BOOL GetCtrlDragData(CWnd *pwndCtrl, const CPoint& point, LPVOID& pDragData)
	{
		return FALSE;
	}
	
	BOOL RegDragableCtrl(CWnd& wndCtrl)
	{
		__AssertReturn(wndCtrl.GetSafeHwnd(), FALSE);

		m_setDragableCtrls.insert(wndCtrl.GetSafeHwnd());
		
		return TRUE;
	}

	void AsyncLoop(UINT uDelayTime, const CB_AsyncLoop& cb=NULL);
};
