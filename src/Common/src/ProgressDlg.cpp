
#include "stdafx.h"

#include "Common/ProgressDlg.h"

#define WM_SetProgress WM_USER+1
#define WM_SetStatusText WM_USER+2

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_wndProgressCtrl);
}

BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	ON_MESSAGE(WM_SetProgress, &CProgressDlg::OnSetProgress)

	ON_MESSAGE(WM_SetStatusText, &CProgressDlg::OnSetStatusText)
END_MESSAGE_MAP()

INT_PTR CProgressDlg::DoModal(const wstring& strTitle, CWnd *pWndParent)
{
	m_strTitle = strTitle;

	CResModule ResModule("Common");
	LPCDLGTEMPLATE lpDialogTemplate = ResModule.loadDialog(IDD_DLG_PROGRESS);
	__AssertReturn(lpDialogTemplate, -1);

	__AssertReturn(this->InitModalIndirect(lpDialogTemplate, pWndParent), -1);

	return __super::DoModal();
}

BOOL CProgressDlg::OnInitDialog()
{
	__super::OnInitDialog();

	(void)this->SetWindowText(m_strTitle.c_str());

	m_bFinished = false;

	(void)this->start(1, [&](UINT) {
		if (m_fnWork)
		{
			m_fnWork(*this);
		}

		if (!m_bFinished)
		{
			CMainApp::GetMainApp()->sync([=]() {
				_endProgress();
			});
			m_bFinished = true;
		}
	}, false);

	return TRUE;
}

inline void CProgressDlg::SetStatusText(const CString& cstrStatusText)
{
	if (m_csLock.try_lock())
	{
		m_mtx.lock();
		m_cstrStatusText = cstrStatusText;
		m_mtx.unlock();

		(void)this->PostMessage(WM_SetStatusText);

		m_csLock.unlock();
	}
}

void CProgressDlg::SetStatusText(const CString& cstrStatusText, UINT uOffsetProgress)
{
	SetStatusText(cstrStatusText);

	if (0 != uOffsetProgress)
	{
		this->ForwardProgress(uOffsetProgress);
	}
}

LRESULT CProgressDlg::OnSetStatusText(WPARAM wParam, LPARAM lParam)
{
	(void)CMainApp::removeMsg(WM_SetStatusText);

	CString cstrStatusText;
	m_mtx.lock();
	cstrStatusText.Append(m_cstrStatusText);
	m_mtx.unlock();
	
	(void)this->SetDlgItemText(IDC_STATIC_STATUS, cstrStatusText);
	
	return TRUE;
}

inline void CProgressDlg::SetProgress(UINT uProgress)
{
	m_uProgress = uProgress;
	
	(void)this->PostMessage(WM_SetProgress);
}

void CProgressDlg::SetProgress(UINT uProgress, UINT uMaxProgress)
{
	m_uMaxProgress = uMaxProgress;
	
	SetProgress(uProgress);
}

inline void CProgressDlg::ForwardProgress(UINT uOffSet)
{
	this->SetProgress(m_uProgress + uOffSet);
}

LRESULT CProgressDlg::OnSetProgress(WPARAM wParam, LPARAM lParam)
{
	(void)CMainApp::removeMsg(WM_SetProgress);
	
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

	(void)::SetDlgItemText(m_hWnd, IDCANCEL, L"完成");
}

void CProgressDlg::OnCancel()
{
	if (m_bFinished)
	{
		CDialog::OnOK();
		return;
	}
	
	this->pause(true);

	if (IDYES != this->msgBox(L"确认取消?", MB_YESNO))
	{
		this->pause(false);
		return;
	}

	this->cancel();

	this->pause(false);

	while (0 != this->getActiveCount())
	{
		(void)CMainApp::GetMainApp()->DoEvents(); // 必须的
	}

	CDialog::OnCancel();
}

void CProgressDlg::Close()
{
	m_bFinished = true;

	this->PostMessage(WM_SYSCOMMAND, SC_CLOSE);
}
