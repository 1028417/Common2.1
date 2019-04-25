
#pragma once

#include "DragDrop.h"

#include "App.h"


class CModuleApp;

class CMainWnd;

using CB_AsyncLoop = function<bool()>;

class __CommonExt CPage: public CPropertyPage
{
public:
	CPage(CResModule& resModule, UINT uIDDlgRes, const CString& cstrTitle=L"", bool bAutoActive=false);

	virtual ~CPage() {}

	virtual void DoDataExchange(CDataExchange* pDX);

	CResModule& m_resModule;

	bool m_bAutoActive = false;

private:
	CString m_cstrTitle;

	set<HWND> m_setDragableCtrls;

	bool m_bDragable = false;

	UINT_PTR m_idTimer = 0;

	CB_AsyncLoop m_cbAsyncLoop;
	
public:
	BOOL Active(bool bForceFocus=true);

	BOOL SetTitle(const CString& cstrTitle, int iImage = -1);

	const CString& GetTitle() const
	{
		return m_cstrTitle;
	}

	virtual int MsgBox(const CString& cstrText, UINT uType=MB_OK);

protected:
	BOOL OnSetActive() override;
	BOOL OnKillActive() override;

	virtual void OnActive(BOOL bActive) {}

	virtual BOOL PreTranslateMessage(MSG* pMsg) override;

	virtual BOOL GetCtrlDragData(HWND hwndCtrl, const CPoint& point, LPVOID& pDragData)
	{
		return FALSE;
	}
	
	BOOL RegDragableCtrl(CWnd& wndCtrl)
	{
		__AssertReturn(wndCtrl.m_hWnd, FALSE);

		m_setDragableCtrls.insert(wndCtrl.m_hWnd);
		
		return TRUE;
	}

	void AsyncLoop(UINT uDelayTime, const CB_AsyncLoop& cb=NULL);
};
