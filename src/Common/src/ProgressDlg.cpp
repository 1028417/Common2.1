
#include "stdafx.h"

#include "Common/ProgressDlg.h"

#define WM_UpateStatus WM_USER+1

void CProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_wndProgressCtrl);
}

BEGIN_MESSAGE_MAP(CProgressDlg, CDialog)
	ON_MESSAGE(WM_UpateStatus, &CProgressDlg::OnUpateStatus)
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
	__async(10, [&] {
		(void)this->start(1, [&](UINT) {
			m_fnWork(*this);
			if (!m_bFinished)
			{
				m_bFinished = true;

				(void)::SetDlgItemText(m_hWnd, IDCANCEL, L"完成");

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
	});

	return TRUE;
}

inline void CProgressDlg::SetStatusText(cwstr strStatusText)
{
	m_mutex.lock();
	m_strStatusText.assign(strStatusText.begin(), strStatusText.end());
	m_mutex.unlock();
	(void)this->PostMessage(WM_UpateStatus);
}

void CProgressDlg::SetStatusText(cwstr strStatusText, UINT uOffsetProgress)
{
	SetStatusText(strStatusText);

	if (0 != uOffsetProgress)
	{
		this->ForwardProgress(uOffsetProgress);
	}
}

inline void CProgressDlg::SetProgress(UINT uProgress)
{
	m_uProgress = uProgress;
	(void)this->PostMessage(WM_UpateStatus);
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

UINT g_uProgress = 0;
UINT g_uMaxProgress = 0;
static wstring g_strStatusText;

LRESULT CProgressDlg::OnUpateStatus(WPARAM wParam, LPARAM lParam)
{
	if (!m_bFinished)
	{
		(void)CMainApp::removeMsg(WM_UpateStatus);
		__app->DoEvent(false);
		_updateProgress();
	}

	if (!m_strStatusText.empty())
	{
		if (m_mutex.try_lock())
		{
			g_strStatusText.swap(m_strStatusText);
			m_mutex.unlock();

			(void)this->SetDlgItemText(IDC_STATIC_STATUS, g_strStatusText.c_str());
			g_strStatusText.clear();
		}
	}
	
	return TRUE;
}

void CProgressDlg::_updateProgress()
{
	if (g_uProgress == m_uProgress && g_uMaxProgress == m_uMaxProgress)
	{
		return;
	}
	g_uProgress = m_uProgress;
	g_uMaxProgress = m_uMaxProgress;

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

	if (IDYES != this->msgBox(L"确认取消?", MB_YESNO))
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
