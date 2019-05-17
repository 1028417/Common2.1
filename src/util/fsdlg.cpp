
#include "util.h"

#include <ShlObj.h>

wstring CFolderDlg::Show(HWND hWndOwner, LPCWSTR lpszInitDir, LPCWSTR lpszTitle, LPCWSTR lpszMessage
	, LPCWSTR lpszOKButton, LPCWSTR lpszCancelButton, UINT uWidth, UINT uHeight)
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
	
	m_nWidth = uWidth;
	m_nHeight = uHeight;
	
	BROWSEINFO browseInfo;
	::ZeroMemory(&browseInfo, sizeof browseInfo);
	browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN | BIF_STATUSTEXT | BIF_BROWSEFORCOMPUTER;

	browseInfo.hwndOwner = hWndOwner;
	browseInfo.lpfn = BrowseFolderCallBack;
	browseInfo.lParam = (LPARAM)this;

	LPITEMIDLIST lpItemIDList = SHBrowseForFolder(&browseInfo);
	
	if(lpItemIDList)
	{
		TCHAR pszPath[512];
		if (SHGetPathFromIDList(lpItemIDList, pszPath))
		{
			m_strInitDir = pszPath;
			wstrutil::trim(m_strInitDir, fsutil::wchBackSlant);

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

				RECT rcWnd{0,0,0,0};
				GetWindowRect(hWnd, &rcWnd);
				(void)::MoveWindow(hWnd, (rcWnd.left + rcWnd.right)/2 - pInstance->m_nWidth / 2
					, (rcWnd.top+ rcWnd.bottom)/2 - pInstance->m_nHeight / 2
					, pInstance->m_nWidth, pInstance->m_nHeight, TRUE);
				
				RECT rcClient{0,0,0,0};
				::GetClientRect(hWnd, &rcClient);
				
				int nWidthOff = rcClient.right - rcClient.left - (rcPreClient.right - rcPreClient.left);
				int nHeightOff = rcClient.bottom - rcClient.top - (rcPreClient.bottom - rcPreClient.top);


				HWND hWndStatic = NULL;
				RECT rcStatic{0,0,0,0};
				while (true)
				{
					hWndStatic = ::FindWindowExA(hWnd, hWndStatic, "Static", NULL);
					if (!hWndStatic)
					{
						break;
					}

					(void)::GetWindowRect(hWndStatic, &rcStatic);
					ScreenToClient(hWnd, (LPPOINT)&rcStatic.left);
					ScreenToClient(hWnd, (LPPOINT)&rcStatic.right);
					rcStatic.top = rcStatic.left;
					rcStatic.bottom = rcStatic.top + 50;
					(void)::MoveWindow(hWndStatic, rcStatic.left, rcStatic.left
						, rcStatic.right - rcStatic.left + nWidthOff, rcStatic.bottom - rcStatic.left, FALSE);
				}


				RECT rcCancelButton{ 0,0,0,0 };
				::GetWindowRect(hWndCancelButton, &rcCancelButton);
				::ScreenToClient(hWnd, (LPPOINT)&rcCancelButton.left);
				::ScreenToClient(hWnd, (LPPOINT)&rcCancelButton.right);
				auto nBtnWidth = rcCancelButton.right - rcCancelButton.left;
				::MoveWindow(hWndCancelButton, rcCancelButton.left + nWidthOff, rcCancelButton.top + nHeightOff
					, nBtnWidth, rcCancelButton.bottom - rcCancelButton.top, FALSE);

				::MoveWindow(hWndOkButton, rcCancelButton.left + nWidthOff - nBtnWidth - 25, rcCancelButton.top + nHeightOff
					, nBtnWidth, rcCancelButton.bottom - rcCancelButton.top, FALSE);

				HWND hWndTreeCtrl = ::FindWindowExA(hWnd, NULL, "SysTreeView32", NULL);
				
				RECT rcTreeCtrl{0,0,0,0};
				::GetWindowRect(hWndTreeCtrl, &rcTreeCtrl);
				ScreenToClient(hWnd, (LPPOINT)&rcTreeCtrl.left);
				ScreenToClient(hWnd, (LPPOINT)&rcTreeCtrl.right);
				rcTreeCtrl.top = rcStatic.bottom;
				rcTreeCtrl.bottom = rcCancelButton.top + nHeightOff - 25;
				::MoveWindow(hWndTreeCtrl, rcTreeCtrl.left, rcTreeCtrl.top
					, rcTreeCtrl.right - rcTreeCtrl.left + nWidthOff, rcTreeCtrl.bottom-rcTreeCtrl.top, FALSE);
				
				break;
			}
		case BFFM_SELCHANGED:
			{
				wstring strMessage = pInstance->m_strMessage;

				TCHAR pszPath[512];
				if (SHGetPathFromIDList((LPITEMIDLIST)lParam, pszPath))
				{
					if (!fsutil::existDir(pszPath))
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
		m_ofn.nFilterIndex = 1;

		::lstrcat(m_lpstrFilter, opt.strFilter.c_str());

		LPTSTR pch = m_lpstrFilter;

		while (true)
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
	
	if (opt.bMustExist)
	{
		m_ofn.Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	}
}

bool CFileDlg::_show(bool bSaveFile)
{
	if (bSaveFile)
	{
		if (!::GetSaveFileName(&m_ofn))
		{
			return false;
		}
	}
	else
	{
		if (!::GetOpenFileName(&m_ofn))
		{
			return false;
		}
	}

	return true;
}

wstring CFileDlg::ShowSave()
{
	m_ofn.Flags |= OFN_OVERWRITEPROMPT;

	if (!_show(true))
	{
		return L"";
	}

	return m_lpstrFileName;
}

wstring CFileDlg::ShowOpenSingle()
{
	if (!_show(false))
	{
		return L"";
	}

	return m_lpstrFileName;
}

wstring CFileDlg::ShowOpenMulti(list<wstring>& lstFiles)
{
	m_ofn.Flags |= OFN_ALLOWMULTISELECT;

	if (!_show(false))
	{
		return L"";
	}

	return _getMultSel(lstFiles);
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

		return fsutil::GetParentDir(m_lpstrFileName);
	}

	wstring strDir = m_lpstrFileName;
	for (list<wstring>::iterator itrFile = lstFiles.begin()++; itrFile != lstFiles.end(); itrFile++)
	{
		*itrFile = strDir + fsutil::wchBackSlant + *itrFile;
	}

	return strDir;
}

UINT CFileDlg::GetSelFilterIndex() const
{
	return m_ofn.nFilterIndex;
}
