
#include "stdafx.h"

#include <ProgressDlg.h>

#include "resource.h"


CProgressDlg::CProgressDlg(const CString& cstrTitle, const CString& cstrStatusText, UINT uMaxProgress, UINT uWorkThreadCount)
{
	m_hMutex = NULL;


	m_cstrTitle = cstrTitle;
	m_cstrStatusText = cstrStatusText;

	__Assert(0 != uMaxProgress);
	m_uMaxProgress = uMaxProgress;

	__Assert(0 != uWorkThreadCount);
	m_uWorkThreadCount = uWorkThreadCount;

	m_uProgress = 0;

	m_bFinished = FALSE;
}

CProgressDlg::~CProgressDlg()
{
}

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


// CExportor ��Ϣ�������

void CProgressDlg::WorkThreadProc(tagWorkThreadInfo& ThreadInfo)
{
	ASSERT(FALSE);
}

INT_PTR CProgressDlg::DoModal()
{
	m_hMutex = ::CreateMutex(NULL, FALSE, NULL);
	__AssertReturn(m_hMutex, -1);

	HRSRC hResource = ::FindResource(g_hInstance, MAKEINTRESOURCE(IDD_DLG_PROGRESS), RT_DIALOG);
	
	HGLOBAL hDialogTemplate = LoadResource(g_hInstance, hResource);
	
	LPCDLGTEMPLATE lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);
	__AssertReturn(lpDialogTemplate, -1);

	__AssertReturn(this->InitModalIndirect(lpDialogTemplate), -1);

	INT_PTR nResult = __super::DoModal();

	(void)::ReleaseMutex(m_hMutex);
	(void)::CloseHandle(m_hMutex);

	return nResult;
}

BOOL CProgressDlg::OnInitDialog()
{
	__super::OnInitDialog();

	(void)this->SetWindowText(m_cstrTitle);

	if (!m_cstrStatusText.IsEmpty())
	{
		this->SetStatusText(m_cstrStatusText);
	}

	m_wndProgressCtrl.SetRange(0, m_uMaxProgress);
	this->SetProgress(0);

	m_bFinished = FALSE;

	(void)this->RunWorkThread(m_uWorkThreadCount);

	return TRUE;
}

void CProgressDlg::SetStatusText(const CString& cstrStatusText)
{
	(void)::WaitForSingleObject(m_hMutex, INFINITE);

	m_cstrStatusText = cstrStatusText;
	
	(void)this->PostMessage(WM_SetStatusText);
	
	(void)::ReleaseMutex(m_hMutex);
}

LRESULT CProgressDlg::OnSetStatusText(WPARAM wParam, LPARAM lParam)
{
	//if (!this->GetExitSignal()) // ��ֹ����
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

	//if (!this->GetExitSignal()) // ��ֹ����
	{
		(void)this->SetDlgItemText(IDC_STATIC_PROGRESS, cstrProgress);

		(void)m_wndProgressCtrl.SetPos((int)uProgress);

		m_uProgress = uProgress;
	}

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


	if (IDYES != this->MessageBox(_T("ȷ��ȡ��?"), m_cstrTitle, MB_YESNO))
	{
		this->Pause(FALSE);

		return;
	}

	this->SetExitSignal();

	this->Pause(FALSE);

	CWaitCursor WaitCursor;

	this->WaitForExit();

	__super::OnCancel();
}
