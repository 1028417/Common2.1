
#include <util.h>

#include <fsdlg.h>

#include <ShlObj.h>

//CFolderDlg

wstring CFolderDlg::Show(HWND hWndOwner, LPCWSTR lpszInitDir, LPCWSTR lpszTitle, LPCWSTR lpszMessage
							, LPCWSTR lpszOKButton, LPCWSTR lpszCancelButton, int nWidth, int nHeight)
{
	if (lpszInitDir)
	{
		m_strInitDir = lpszInitDir;
	}

	if (lpszTitle)
	{
		m_strTitle = lpszTitle;
	}

	if (lpszMessage)
	{
		m_strMessage = lpszMessage;
	}

	if (lpszOKButton)
	{
		m_strOKButton = lpszOKButton;
	}

	if (lpszCancelButton)
	{
		m_strCancelButton = lpszCancelButton;
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

	browseInfo.hwndOwner = hWndOwner;
	browseInfo.lpfn = BrowseFolderCallBack;
	browseInfo.lParam = (LPARAM)this;

	LPITEMIDLIST lpItemIDList = SHBrowseForFolder(&browseInfo);

	(void)::EnableWindow(hWndOwner, TRUE);

	if(lpItemIDList)
	{
		TCHAR pszPath[512];
		if (SHGetPathFromIDList(lpItemIDList, pszPath))
		{
			m_strInitDir = pszPath;
			util::trim(m_strInitDir, '\\');

			return m_strInitDir;
		}
	}

	return L"";
}

int CFolderDlg::BrowseFolderCallBack(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpParam)
{
	HWND hWndOkButton = GetDlgItem(hWnd, IDOK);
	HWND hWndCancelButton = GetDlgItem(hWnd, IDCANCEL);

	CFolderDlg* pInstance = (CFolderDlg*)lpParam;

	switch (uMsg)
	{
		case BFFM_INITIALIZED:
			{
				if (!pInstance->m_strInitDir.empty())
				{
					(void)::SendMessage(hWnd, BFFM_SETSELECTION, TRUE, (LPARAM)pInstance->m_strInitDir.c_str());
				}

				if (!pInstance->m_strTitle.empty())
				{
					SetWindowText(hWnd, pInstance->m_strTitle.c_str());
				}

				wstring strMessage = pInstance->m_strMessage + L"\n\n" + pInstance->m_strInitDir;
				(void)::SendMessage(hWnd, BFFM_SETSTATUSTEXT, 1, (LPARAM)strMessage.c_str());

				if (!pInstance->m_strOKButton.empty())
				{
					::SetWindowText(hWndOkButton, pInstance->m_strOKButton.c_str());
				}
				if (!pInstance->m_strCancelButton.empty())
				{
					::SetWindowText(hWndCancelButton, pInstance->m_strCancelButton.c_str());
				}


				if (0 == pInstance->m_nWidth || 0 == pInstance->m_nHeight)
				{
					break;
				}

				RECT rcPreClient{ 0,0,0,0 };
				::GetClientRect(hWnd, &rcPreClient);

				::MoveWindow(hWnd, 0, 0, pInstance->m_nWidth, pInstance->m_nHeight, FALSE);
				//::CenterWindow(hWnd);

				RECT rcClient{0,0,0,0};
				::GetClientRect(hWnd, &rcClient);
				
				int nWidthOff = rcClient.right - rcClient.left - (rcPreClient.right - rcPreClient.left);
				int nHeightOff = rcClient.bottom - rcClient.top - (rcPreClient.bottom - rcPreClient.top);


				HWND hWndStatic = NULL;
				RECT rcStatic{0,0,0,0};
				while (TRUE)
				{
					hWndStatic = ::FindWindowEx(hWnd, hWndStatic, L"Static", NULL);
					if (!hWndStatic)
					{
						break;
					}

					(void)::GetWindowRect(hWndStatic, &rcStatic);
					ScreenToClient(hWnd, (LPPOINT)&rcStatic.left);
					ScreenToClient(hWnd, (LPPOINT)&rcStatic.right);
					(void)::MoveWindow(hWndStatic, rcStatic.left, rcStatic.left
						, rcStatic.right - rcStatic.left + nWidthOff, rcStatic.bottom - rcStatic.left, FALSE);
				}


				HWND hWndTreeCtrl = ::FindWindowEx(hWnd, NULL, L"SysTreeView32", NULL);
				
				RECT rcTreeCtrl{0,0,0,0};
				::GetWindowRect(hWndTreeCtrl, &rcTreeCtrl);
				ScreenToClient(hWnd, (LPPOINT)&rcTreeCtrl.left);
				ScreenToClient(hWnd, (LPPOINT)&rcTreeCtrl.right);
				::MoveWindow(hWndTreeCtrl, rcTreeCtrl.left, rcTreeCtrl.top
					, rcTreeCtrl.right - rcTreeCtrl.left + nWidthOff, rcTreeCtrl.bottom - rcTreeCtrl.top + nHeightOff, FALSE);


				RECT rcOkButton{0,0,0,0};
				::GetWindowRect(hWndOkButton, &rcOkButton);
				::ScreenToClient(hWnd, (LPPOINT)&rcOkButton.left);
				::ScreenToClient(hWnd, (LPPOINT)&rcOkButton.right);
				::MoveWindow(hWndOkButton, rcOkButton.left + nWidthOff, rcOkButton.top + nHeightOff
					, rcOkButton.right - rcOkButton.left , rcOkButton.bottom - rcOkButton.top, FALSE);

				RECT rcCancelButton{0,0,0,0};
				::GetWindowRect(hWndCancelButton, &rcCancelButton);
				::ScreenToClient(hWnd, (LPPOINT)&rcCancelButton.left);
				::ScreenToClient(hWnd, (LPPOINT)&rcCancelButton.right);
				::MoveWindow(hWndCancelButton, rcCancelButton.left + nWidthOff, rcCancelButton.top + nHeightOff
					, rcCancelButton.right - rcCancelButton.left, rcCancelButton.bottom - rcCancelButton.top, FALSE);

				break;
			}
		case BFFM_SELCHANGED:
			{
				wstring strMessage = pInstance->m_strMessage;

				TCHAR pszPath[512];
				if (SHGetPathFromIDList((LPITEMIDLIST)lParam, pszPath))
				{
					if (!fsutil_win::ExistsPath(pszPath))
					{
						::EnableWindow(hWndOkButton, FALSE);
					}
					else
					{
						::EnableWindow(hWndOkButton, TRUE);
						strMessage = strMessage + L"\n\n" + pszPath;
					}
				}

				(void)::SendMessage(hWnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)strMessage.c_str());

				break;
			}
	}

	return 0;
}

void CFileDlg::_setOpt(const tagFileDlgOpt& opt)
{
	::ZeroMemory(&m_ofn, sizeof(m_ofn));
	m_ofn.lStructSize = sizeof(m_ofn);

	m_strTitle = opt.strTitle;
	m_ofn.lpstrTitle = m_strTitle.c_str();

	m_strInitialDir = opt.strInitialDir;
	m_ofn.lpstrInitialDir = m_strInitialDir.c_str();

	::ZeroMemory(m_lpstrFileName, sizeof(m_lpstrFileName));
	if (!opt.strFileName.empty())
	{
		::lstrcat(m_lpstrFileName, opt.strFileName.c_str());
	}
	m_ofn.lpstrFile = m_lpstrFileName;
	m_ofn.nMaxFile = sizeof(m_lpstrFileName) / sizeof(m_lpstrFileName[0]);

	::ZeroMemory(m_lpstrFilter, sizeof m_lpstrFilter);
	m_ofn.lpstrFilter = m_lpstrFilter;
	if (!opt.strFilter.empty())
	{
		::lstrcat(m_lpstrFilter, opt.strFilter.c_str());

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

	m_ofn.hwndOwner = opt.hWndOwner;

	m_ofn.Flags = OFN_EXPLORER;

	if (opt.bMultiSel)
	{
		m_ofn.Flags |= OFN_ALLOWMULTISELECT;
	}

	if (opt.bMustExist)
	{
		m_ofn.Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	}
}

wstring CFileDlg::Show(list<wstring>& lstFiles)
{
	BOOL bRet = ::GetOpenFileName(&m_ofn);

	(void)::EnableWindow(m_ofn.hwndOwner, TRUE);

	(void)::SetFocus(m_ofn.hwndOwner);

	if (!bRet)
	{
		return L"";
	}

	if (m_ofn.Flags & OFN_ALLOWMULTISELECT)
	{
		return _getMultSel(lstFiles);
	}
	else
	{
		lstFiles.push_back(m_lpstrFileName);

		return fsutil::GetParentPath(m_lpstrFileName);
	}
}

wstring CFileDlg::Show(const tagFileDlgOpt& opt, list<wstring>& lstFiles)
{
	_setOpt(opt);

	return Show(lstFiles);
}

wstring CFileDlg::_getMultSel(list<wstring>& lstFiles)
{
	for (TCHAR *p = m_lpstrFileName; ; p++)
	{
		if ('\0' == *p)
		{
			if ('\0' == *(p + 1))
			{
				break;
			}

			lstFiles.push_back(p + 1);
		}
	}
	if (lstFiles.empty())
	{
		lstFiles.push_back(m_lpstrFileName);
	
		return fsutil::GetParentPath(m_lpstrFileName);
	}
	else
	{
		wstring strDir = m_lpstrFileName;
		for (list<wstring>::iterator itrFile = lstFiles.begin()++; itrFile != lstFiles.end(); itrFile++)
		{
			*itrFile = strDir + __BackSlant + *itrFile;
		}

		return strDir;
	}
}
