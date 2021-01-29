
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

INT_PTR CProgressDlg::DoModal(cwstr strTitle, CWnd *pWndParent)
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
		__usleep(100);
		m_fnWork(*this);
		if (!m_bFinished)
		{
			m_bFinished = true;

			(void)::SetDlgItemText(m_hWnd, IDCANCEL, L"���");

			if (m_uMaxProgress)
			{
				m_wndProgressCtrl.SetPos(m_uMaxProgress);
			}
			else
			{
				m_wndProgressCtrl.SetRange(0, 1);
				m_wndProgressCtrl.SetPos(1);
			}
		}
	}, false);

	return TRUE;
}

inline void CProgressDlg::SetStatusText(const CString& cstrStatusText)
{
	m_csLock.lock();	
	m_cstrStatusText = cstrStatusText;
	m_csLock.unlock();
	(void)this->PostMessage(WM_SetStatusText);
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
	if (!m_csLock.try_lock())
	{
		return TRUE;
	}
	auto cstrStatusText = m_cstrStatusText;
	m_csLock.unlock();

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
	if (!m_bFinished)
	{
		_updateProgress();
		(void)CMainApp::removeMsg(WM_SetProgress);
	}
	return TRUE;
}

void CProgressDlg::_updateProgress()
{
	m_wndProgressCtrl.SetRange(0, m_uMaxProgress);
	m_wndProgressCtrl.SetPos(m_uProgress);

	CString cstrProgress;
	if (m_uMaxProgress)
	{
		cstrProgress.Format(_T("%d/%d"), m_uProgress, m_uMaxProgress);
	}
	else
	{
		if (0 == m_uProgress)
		{
			return;
		}
		cstrProgress.Format(_T("%d"), m_uProgress);
	}
	(void)this->SetDlgItemText(IDC_STATIC_PROGRESS, cstrProgress);
}

void CProgressDlg::OnCancel()
{
	if (m_bFinished)
	{
		CDialog::OnOK();
		return;
	}
	
	this->pause(true);

	if (IDYES != this->msgBox(L"ȷ��ȡ��?", MB_YESNO))
	{
		this->pause(false);
		return;
	}

	this->cancel();

	this->pause(false);

	while (0 != this->getActiveCount())
	{
		if (__app->DoEvents() == E_DoEventsResult::DER_None)
		{
			__usleep(50);
		}
	}

	CDialog::OnCancel();
}

void CProgressDlg::Close()
{
	m_bFinished = true;
	this->PostMessage(WM_SYSCOMMAND, SC_CLOSE);
}
