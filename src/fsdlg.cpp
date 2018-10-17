
#include "stdafx.h"

#include <fsdlg.h>

//CFolderDlg

CString CFolderDlg::Show(CWnd& WndOwner, LPCTSTR lpszInitDir, LPCTSTR lpszTitle, LPCTSTR lpszMessage
							, LPCTSTR lpszOKButton, LPCTSTR lpszCancelButton, int nWidth, int nHeight)
{
	if (lpszInitDir)
	{
		m_cstrInitDir = lpszInitDir;
	}

	if (lpszTitle)
	{
		m_cstrTitle = lpszTitle;
	}

	if (lpszMessage)
	{
		m_cstrMessage = lpszMessage;
	}

	if (lpszOKButton)
	{
		m_cstrOKButton = lpszOKButton;
	}

	if (lpszCancelButton)
	{
		m_cstrCancelButton = lpszCancelButton;
	}

	if (nWidth)
	{
		m_nWidth = nWidth;
	}

	if (nHeight)
	{
		m_nHeight = nHeight;
	}

	//(void)::EnableWindow(WndOwner.m_hWnd, FALSE);

	BROWSEINFO browseInfo;
	::ZeroMemory(&browseInfo, sizeof browseInfo);
	browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN | BIF_STATUSTEXT | BIF_BROWSEFORCOMPUTER;

	browseInfo.hwndOwner = WndOwner.m_hWnd;
	browseInfo.lpfn = BrowseFolderCallBack;
	browseInfo.lParam = (LPARAM)this;

	LPITEMIDLIST lpItemIDList = SHBrowseForFolder(&browseInfo);

	(void)::EnableWindow(WndOwner.m_hWnd, TRUE);

	if(lpItemIDList)
	{
		TCHAR pszPath[512];
		if (SHGetPathFromIDList(lpItemIDList, pszPath))
		{
			m_cstrInitDir = pszPath;
			m_cstrInitDir.Trim('\\');

			return m_cstrInitDir;
		}
	}

	::DoEvents();

	return (LPCTSTR)NULL;
}

int CFolderDlg::BrowseFolderCallBack(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpParam)
{
	CWnd *pWnd = CWnd::FromHandle(hWnd);
	CWnd *pWndOkButton = pWnd->GetDlgItem(IDOK);
	CWnd *pWndCancelButton = pWnd->GetDlgItem(IDCANCEL);

	CFolderDlg* pInstance = (CFolderDlg*)lpParam;

	switch (uMsg)
	{
		case BFFM_INITIALIZED:
			{
				if (!pInstance->m_cstrInitDir.IsEmpty())
				{
					(void)::SendMessage(hWnd, BFFM_SETSELECTION, TRUE, (LPARAM)(LPCTSTR)pInstance->m_cstrInitDir);
				}

				if (!pInstance->m_cstrTitle.IsEmpty())
				{
					pWnd->SetWindowText(pInstance->m_cstrTitle);
				}

				CString cstrMessage = pInstance->m_cstrMessage;
				cstrMessage += _T("\n\n");
				cstrMessage += pInstance->m_cstrInitDir;
				(void)pWnd->SendMessage(BFFM_SETSTATUSTEXT, 1, (LPARAM)(LPCTSTR)cstrMessage);

				if (!pInstance->m_cstrOKButton.IsEmpty())
				{
					pWndOkButton->SetWindowText(pInstance->m_cstrOKButton);
				}
				if (!pInstance->m_cstrCancelButton.IsEmpty())
				{
					pWndCancelButton->SetWindowText(pInstance->m_cstrCancelButton);
				}


				if (0 == pInstance->m_nWidth || 0 == pInstance->m_nHeight)
				{
					break;
				}

				CRect rcPreClient;
				pWnd->GetClientRect(&rcPreClient);

				pWnd->MoveWindow(0, 0, pInstance->m_nWidth, pInstance->m_nHeight, FALSE);
				pWnd->CenterWindow();

				CRect rcClient;
				pWnd->GetClientRect(&rcClient);
				
				int nWidthOff = rcClient.Width() - rcPreClient.Width();
				int nHeightOff = rcClient.Height() - rcPreClient.Height();


				HWND hWndStatic = NULL;
				CRect rcStatic;
				while (TRUE)
				{
					hWndStatic = ::FindWindowEx(hWnd, hWndStatic, L"Static", NULL);
					if (!hWndStatic)
					{
						break;
					}


					(void)::GetWindowRect(hWndStatic, &rcStatic);
					pWnd->ScreenToClient(&rcStatic);
					(void)::MoveWindow(hWndStatic, rcStatic.left, rcStatic.left
						, rcStatic.Width() + nWidthOff, rcStatic.bottom - rcStatic.left, FALSE);
				}


				HWND hWndTreeCtrl = ::FindWindowEx(hWnd, NULL, L"SysTreeView32", NULL);
				CWnd *pWndTreeCtrl = CWnd::FromHandle(hWndTreeCtrl);
				if (!pWndTreeCtrl)
				{
					ASSERT(FALSE);
					break;
				}

				CRect rcTreeCtrl;
				pWndTreeCtrl->GetWindowRect(&rcTreeCtrl);
				pWnd->ScreenToClient(&rcTreeCtrl);
				pWndTreeCtrl->MoveWindow(rcTreeCtrl.left, rcTreeCtrl.top
					, rcTreeCtrl.Width() + nWidthOff, rcTreeCtrl.Height() + nHeightOff, FALSE);


				CRect rcOkButton;
				pWndOkButton->GetWindowRect(&rcOkButton);
				pWnd->ScreenToClient(&rcOkButton);
				pWndOkButton->MoveWindow(rcOkButton.left + nWidthOff, rcOkButton.top + nHeightOff
					, rcOkButton.Width() , rcOkButton.Height(), FALSE);

				CRect rcCancelButton;
				pWndCancelButton->GetWindowRect(&rcCancelButton);
				pWnd->ScreenToClient(&rcCancelButton);
				pWndCancelButton->MoveWindow(rcCancelButton.left + nWidthOff, rcCancelButton.top + nHeightOff
					, rcCancelButton.Width(), rcCancelButton.Height(), FALSE);

				break;
			}
		case BFFM_SELCHANGED:
			{
				CString cstrMessage = pInstance->m_cstrMessage;

				TCHAR pszPath[512];
				if (SHGetPathFromIDList((LPITEMIDLIST)lParam, pszPath))
				{
					if (!fsutil::ExistsPath(pszPath))
					{
						pWndOkButton->EnableWindow(FALSE);
					}
					else
					{
						pWndOkButton->EnableWindow();
						cstrMessage = cstrMessage + _T("\n\n") + pszPath;
					}
				}

				(void)pWnd->SendMessage(BFFM_SETSTATUSTEXT, 0, (LPARAM)(LPCTSTR)cstrMessage);

				break;
			}
	}

	return 0;
}


CFileDlg::CFileDlg(const wstring& strTitle, const wstring& strFilter
	, const wstring& strInitialDir, const wstring& strFileName, CWnd *pWndOwner)
	: m_strTitle(strTitle)
	, m_strInitialDir(strInitialDir)
{
	::ZeroMemory(&m_ofn, sizeof(m_ofn));
	m_ofn.lStructSize = sizeof(m_ofn);

	m_ofn.hInstance = AfxGetInstanceHandle();
	
	m_ofn.lpstrTitle = m_strTitle.c_str();

	if (NULL == pWndOwner)
	{
		pWndOwner = AfxGetMainWnd();
	}
	m_ofn.hwndOwner = pWndOwner->GetSafeHwnd();
	
	::ZeroMemory(m_lpstrFilter, sizeof m_lpstrFilter);
	m_ofn.lpstrFilter = m_lpstrFilter;
	if (!strFilter.empty())
	{
		::lstrcat(m_lpstrFilter, strFilter.c_str());

		LPTSTR pch = m_lpstrFilter;

		while (TRUE)
		{
			pch = wcschr(pch, '|');
			if (!pch)
			{
				break;
			}

			*pch = '\0';

			pch++;
		}
	}

	::ZeroMemory(m_lpstrFileName, sizeof(m_lpstrFileName));
	m_ofn.lpstrFile = m_lpstrFileName;
	m_ofn.nMaxFile = sizeof(m_lpstrFileName)/sizeof(m_lpstrFileName[0]);
	if (!strFileName.empty())
	{
		::wcscpy_s(m_lpstrFileName, m_ofn.nMaxFile, strFileName.c_str());
	}

	m_ofn.lpstrInitialDir = m_strInitialDir.c_str();
	
	m_ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;//标志如果是多选要加上OFN_ALLOWMULTISELECT
}

wstring CFileDlg::Show()
{
	bool bResult = ::GetOpenFileName(&m_ofn);

	(void)::EnableWindow(m_ofn.hwndOwner, TRUE);

	(void)::SetFocus(m_ofn.hwndOwner);

	__EnsureReturn(bResult, L"");
	
	return m_lpstrFileName;
}

void CFileDlg::ShowMultiSel(list<wstring>& lstFiles)
{
	m_ofn.Flags |= OFN_ALLOWMULTISELECT;

	bool bResult = ::GetOpenFileName(&m_ofn);

	(void)::EnableWindow(m_ofn.hwndOwner, TRUE);

	(void)::SetFocus(m_ofn.hwndOwner);

	__Ensure(bResult);

	for (TCHAR *p = m_lpstrFileName; ; p++)
	{
		if ('\0' == *p)
		{
			if ('\0' == *(p + 1))
			{
				break;
			}

			lstFiles.push_back(p+1);
		}
	}
	if (lstFiles.empty())
	{
		lstFiles.push_back(m_lpstrFileName);
	}
	else
	{
		wstring strDir = m_lpstrFileName;
		for (list<wstring>::iterator itrFile = lstFiles.begin()++; itrFile!=lstFiles.end(); itrFile++)
		{
			*itrFile = strDir + __BackSlant + *itrFile;
		}
	}
}
