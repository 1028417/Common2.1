#pragma once

#include <WorkThread.h>


#define WM_SetProgress WM_USER+1
#define WM_SetStatusText WM_USER+2
#define WM_EndProgress WM_USER+3


class __CommonPrjExt CProgressDlg : public CDialog, public CWorkThread
{
public:
	CProgressDlg(const CString& cstrTitle=_T(""), const CString& cstrStatusText=_T("")
		, UINT nMaxProgress=100, UINT nWorkThreadCount=1);

	virtual ~CProgressDlg();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	HANDLE m_hMutex;

protected:
	CString m_cstrTitle;

	CString m_cstrStatusText;

	UINT m_nWorkThreadCount;

	UINT m_nMaxProgress;

	UINT m_nProgress;

	BOOL m_bFinished;

private:
	CProgressCtrl m_wndProgressCtrl;

public:
	virtual INT_PTR DoModal();

	void SetStatusText(const CString& cstrStatusText);
	LRESULT OnSetStatusText(WPARAM wParam, LPARAM lParam);

	void SetProgress(UINT nProgress);
	LRESULT OnSetProgress(WPARAM wParam, LPARAM lParam);

	UINT ForwardProgress(UINT nOff=1);

	void EndProgress(BOOL bClose, const CString& cstrButton=_T("完成"));

	LRESULT OnEndProgress(WPARAM wParam, LPARAM lParam);

public:
	virtual void WorkThreadProc(tagWorkThreadInfo& ThreadInfo) override;

public:
	virtual BOOL OnInitDialog();

	virtual void OnCancel();
};
