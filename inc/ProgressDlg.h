#pragma once

#define WM_SetProgress WM_USER+1
#define WM_SetStatusText WM_USER+2
#define WM_EndProgress WM_USER+3


using FN_Work = function<void(class CProgressDlg&)>;

class __CommonExt CProgressDlg : private CDialog, public CWorkThread
{
public:
	CProgressDlg(const wstring& strTitle, UINT uMaxProgress, const FN_Work& fnWork=NULL)
		: m_strTitle(strTitle)
		, m_fnWork(fnWork)
	{
		m_uMaxProgress = uMaxProgress;
	}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

private:
	FN_Work m_fnWork;

	wstring m_strTitle;

	UINT m_uMaxProgress = 0;

	UINT m_uProgress = 0;

	NS_mtutil::CCSLock m_csLock;
	
	BOOL m_bFinished = false;

	CString m_cstrStatusText;

	CProgressCtrl m_wndProgressCtrl;

public:
	virtual INT_PTR DoModal(CWnd *pWndParent=NULL);

	int showMsgBox(const wstring& strText, const wstring& strTitle, UINT uType = 0)
	{
		return MessageBoxW(strText.c_str(), strTitle.c_str(), uType);
	}

	int showMsgBox(const wstring& strText, UINT uType = 0)
	{
		return showMsgBox(strText, m_strTitle, uType);
	}

	void SetStatusText(const CString& cstrStatusText, UINT uOffsetProgress=0);
	LRESULT OnSetStatusText(WPARAM wParam, LPARAM lParam);

	void SetProgress(UINT uProgress);
	LRESULT OnSetProgress(WPARAM wParam, LPARAM lParam);

	void ForwardProgress(UINT uOffSet=1);

	void Close();

	LRESULT OnEndProgress(WPARAM wParam, LPARAM lParam);

private:
	virtual void WorkThreadProc(tagWorkThreadInfo& ThreadInfo) override;

	virtual BOOL OnInitDialog();

	virtual void OnCancel();

	void _updateProgress();
};
