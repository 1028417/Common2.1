#pragma once

#define WM_SetProgress WM_USER+1
#define WM_SetStatusText WM_USER+2
#define WM_EndProgress WM_USER+3


using FN_Work = function<void(class CProgressDlg& ProgressDlg)>;

class __CommonExt CProgressDlg : public CDialog, public CWorkThread
{
public:
	CProgressDlg(const CString& cstrTitle, UINT uMaxProgress, const FN_Work& fnWork=NULL)
		: m_cstrTitle(cstrTitle)
		, m_fnWork(fnWork)
	{
		m_uMaxProgress = uMaxProgress;
	}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
	FN_Work m_fnWork;

	CString m_cstrTitle;

	UINT m_uMaxProgress = 0;

	UINT m_uProgress = 0;

	HANDLE m_hMutex = NULL;

	BOOL m_bFinished = false;

	CString m_cstrStatusText;

	CProgressCtrl m_wndProgressCtrl;

public:
	virtual INT_PTR DoModal();

	void SetStatusText(const CString& cstrStatusText, UINT uOffsetProgress=0);
	LRESULT OnSetStatusText(WPARAM wParam, LPARAM lParam);

	void SetProgress(UINT uProgress);
	LRESULT OnSetProgress(WPARAM wParam, LPARAM lParam);

	UINT ForwardProgress(UINT uOffSet=1);

	void EndProgress(BOOL bClose, const CString& cstrButton=_T("完成"));

	LRESULT OnEndProgress(WPARAM wParam, LPARAM lParam);

private:
	virtual void WorkThreadProc(tagWorkThreadInfo& ThreadInfo) override;

	virtual BOOL OnInitDialog();

	virtual void OnCancel();
};
