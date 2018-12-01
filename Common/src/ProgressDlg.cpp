
#include "stdafx.h"

#include <ProgressDlg.h>

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_wndProgressCtrl);
}


BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	ON_MESSAGE(WM_SetProgress, &CProgressDlg::OnSetProgress)

	ON_MESSAGE(WM_SetStatusText, &CProgressDlg::OnSetStatusText)

	ON_MESSAGE(WM_EndProgress, &CProgressDlg::OnEndProgress)
END_MESSAGE_MAP()


// CExportor 消息处理程序

void CProgressDlg::WorkThreadProc(tagWorkThreadInfo& ThreadInfo)
{
	if (m_fnWork)
	{
		m_fnWork(*this);
	}

	this->EndProgress(FALSE);
}

INT_PTR CProgressDlg::DoModal(CWnd *pWndParent)
{
	m_hMutex = ::CreateMutex(NULL, FALSE, NULL);
	__AssertReturn(m_hMutex, -1);

	LPCDLGTEMPLATE lpDialogTemplate = g_ResModule.loadDialog(IDD_DLG_PROGRESS);
	__AssertReturn(lpDialogTemplate, -1);

	__AssertReturn(this->InitModalIndirect(lpDialogTemplate, pWndParent), -1);

	INT_PTR nResult = __super::DoModal();

	(void)::ReleaseMutex(m_hMutex);
	(void)::CloseHandle(m_hMutex);

	return nResult;
}

BOOL CProgressDlg::OnInitDialog()
{
	__super::OnInitDialog();

	(void)this->SetWindowText(m_cstrTitle);

	m_wndProgressCtrl.SetRange(0, m_uMaxProgress);
	this->SetProgress(0);

	m_bFinished = FALSE;

	(void)this->RunWorkThread(1);

	return TRUE;
}

void CProgressDlg::SetStatusText(const CString& cstrStatusText, UINT uOffsetProgress)
{
	(void)::WaitForSingleObject(m_hMutex, INFINITE);

	m_cstrStatusText = cstrStatusText;
	
	(void)this->PostMessage(WM_SetStatusText);
	
	if (0 != uOffsetProgress)
	{
		m_uProgress += uOffsetProgress;
		this->SetProgress(m_uProgress);
	}

	(void)::ReleaseMutex(m_hMutex);
}

LRESULT CProgressDlg::OnSetStatusText(WPARAM wParam, LPARAM lParam)
{
	//if (!this->GetExitSignal()) // 防止死锁
	{
		(void)this->SetDlgItemText(IDC_STATIC_STATUS, m_cstrStatusText);
	}

	return TRUE;
}

void CProgressDlg::SetProgress(UINT uProgress)
{	
	(void)this->PostMessage(WM_SetProgress, uProgress);
}

LRESULT CProgressDlg::OnSetProgress(WPARAM wParam, LPARAM lParam)
{
	UINT uProgress = (UINT)wParam;

	CString cstrProgress;
	cstrProgress.Format(_T("%d/%d"), uProgress, m_uMaxProgress);

	(void)this->SetDlgItemText(IDC_STATIC_PROGRESS, cstrProgress);

	(void)m_wndProgressCtrl.SetPos((int)uProgress);

	return TRUE;
}

UINT CProgressDlg::ForwardProgress(UINT uOffSet)
{
	(void)::WaitForSingleObject(m_hMutex, INFINITE);

	m_uProgress += uOffSet;
	this->SetProgress(m_uProgress);

	(void)::ReleaseMutex(m_hMutex);

	return m_uProgress;
}

void CProgressDlg::EndProgress(BOOL bClose, const CString& cstrButton)
{
	this->SetProgress(m_uMaxProgress);

	m_bFinished = TRUE;
	
	if (bClose)
	{
		(void)this->PostMessage(WM_EndProgress);
	}
	else
	{
		(void)::SetDlgItemText(m_hWnd, IDCANCEL, cstrButton);
	}
}

LRESULT CProgressDlg::OnEndProgress(WPARAM wParam, LPARAM lParam)
{
	this->OnOK();
	
	return 0;
}

void CProgressDlg::OnCancel()
{
	__Ensure(this->IsWindowVisible());

	if (m_bFinished)
	{
		__super::OnOK();
		return;
	}

	this->Pause(TRUE);


	if (IDYES != this->MessageBox(_T("确认取消?"), m_cstrTitle, MB_YESNO))
	{
		this->Pause(FALSE);

		return;
	}

	this->Cancel();

	this->Pause(FALSE);

	//CWaitCursor WaitCursor;

	while (0 != this->GetActiveCount())
	{
		(void)::DoEvents(); // 必须的
	}

	__super::OnCancel();
}
