
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

INT_PTR CProgressDlg::DoModal(const wstring& strTitle, CWnd *pWndParent)
{
	m_strTitle = strTitle;

	LPCDLGTEMPLATE lpDialogTemplate = g_ResModule.loadDialog(IDD_DLG_PROGRESS);
	__AssertReturn(lpDialogTemplate, -1);

	__AssertReturn(this->InitModalIndirect(lpDialogTemplate, pWndParent), -1);

	INT_PTR nResult = __super::DoModal();

	return nResult;
}

BOOL CProgressDlg::OnInitDialog()
{
	__super::OnInitDialog();

	(void)this->SetWindowText(m_strTitle.c_str());

	m_bFinished = FALSE;

	(void)this->Run(1);

	return TRUE;
}

void CProgressDlg::WorkThreadProc(tagWorkThreadInfo& ThreadInfo)
{
	if (m_fnWork)
	{
		m_fnWork(*this);
	}

	if (!m_bFinished)
	{
		(void)this->PostMessage(WM_EndProgress);
	}
}

void CProgressDlg::SetStatusText(const CString& cstrStatusText, UINT uOffsetProgress)
{
	m_csLock.lock();
	m_cstrStatusText = cstrStatusText;
	m_csLock.unlock();

	(void)this->PostMessage(WM_SetStatusText);

	if (0 != uOffsetProgress)
	{
		this->ForwardProgress(uOffsetProgress);
	}
}

LRESULT CProgressDlg::OnSetStatusText(WPARAM wParam, LPARAM lParam)
{
	MSG msg;
	memset(&msg, 0, sizeof msg);
	(void)CMainApp::peekMsg(WM_SetStatusText, msg, true);

	CString cstrStatusText;
	if (m_csLock.try_lock())
	{
		cstrStatusText = m_cstrStatusText;
		m_csLock.unlock();

		(void)this->SetDlgItemText(IDC_STATIC_STATUS, cstrStatusText);
	}

	return TRUE;
}

void CProgressDlg::SetProgress(UINT uProgress)
{
	m_uProgress = uProgress;
	
	(void)this->PostMessage(WM_SetProgress);
}

void CProgressDlg::SetProgress(UINT uProgress, UINT uMaxProgress)
{
	m_uMaxProgress = uMaxProgress;
	
	SetProgress(uProgress);
}

void CProgressDlg::ForwardProgress(UINT uOffSet)
{
	this->SetProgress(m_uProgress + uOffSet);
}

LRESULT CProgressDlg::OnSetProgress(WPARAM wParam, LPARAM lParam)
{
	MSG msg;
	memset(&msg, 0, sizeof msg);
	(void)CMainApp::peekMsg(WM_SetProgress, msg, true);
	
	_updateProgress();
	
	return TRUE;
}

void CProgressDlg::_updateProgress()
{
	CString cstrProgress;
	cstrProgress.Format(_T("%d/%d"), m_uProgress, m_uMaxProgress);
	(void)this->SetDlgItemText(IDC_STATIC_PROGRESS, cstrProgress);

	m_wndProgressCtrl.SetRange(0, m_uMaxProgress);
	(void)m_wndProgressCtrl.SetPos(m_uProgress);
}

LRESULT CProgressDlg::OnEndProgress(WPARAM wParam, LPARAM lParam)
{
	_endProgress();
	
	m_bFinished = TRUE;
	
	(void)::SetDlgItemText(m_hWnd, IDCANCEL, L"完成");

	return 0;
}

void CProgressDlg::_endProgress()
{
	if (0 != m_uMaxProgress)
	{
		m_uProgress = m_uMaxProgress;
		_updateProgress();
	}
	else
	{
		m_wndProgressCtrl.SetRange(0, 1);
		(void)m_wndProgressCtrl.SetPos(1);
	}
}

void CProgressDlg::OnCancel()
{
	if (m_bFinished)
	{
		__super::OnOK();
		return;
	}

	//__Ensure(this->IsWindowVisible());
	
	this->Pause(true);

	if (IDYES != this->showMsgBox(L"确认取消?", MB_YESNO))
	{
		this->Pause(false);
		return;
	}

	this->Cancel();

	this->Pause(false);

	//CWaitCursor WaitCursor;

	while (0 != this->GetActiveCount())
	{
		(void)::DoEvents(); // 必须的
	}

	__super::OnCancel();
}

void CProgressDlg::Close()
{
	m_bFinished = TRUE;

	this->PostMessage(WM_SYSCOMMAND, SC_CLOSE);
}
