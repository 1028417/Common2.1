
#include "util.h"

#include <ShlObj.h>

#define __browseInfoFlag BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN | BIF_BROWSEFORCOMPUTER | BIF_STATUSTEXT
//| BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON | BIF_UAHINT;

static int __stdcall _handleCallBack_Init(HWND hWnd, UINT, LPARAM, LPARAM)
{
	::ShowWindow(hWnd, SW_HIDE);
    ::PostMessage(hWnd, WM_CLOSE, 0, 0);
    return 0;
}

void CFolderDlg::preInit()
{
    BROWSEINFO browseInfo;
    ::ZeroMemory(&browseInfo, sizeof(browseInfo));
    browseInfo.ulFlags = __browseInfoFlag;
    browseInfo.lpfn = _handleCallBack_Init;

    (void)SHBrowseForFolder(&browseInfo);
}

int CFolderDlg::_handleCallBack(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	CFolderDlg* pFolderDlg = (CFolderDlg*)lpData;
	pFolderDlg->_handleCallBack(hWnd, uMsg, lParam);
	return 0;
}

void CFolderDlg::_initDlg()
{
	if (!m_strInitialDir.empty())
	{
		(void)::SendMessage(m_hWnd, BFFM_SETSELECTIONW, TRUE, (LPARAM)m_strInitialDir.c_str());
	}

	if (!m_strTitle.empty())
	{
		SetWindowText(m_hWnd, m_strTitle.c_str());
	}

	(void)::SendMessage(m_hWnd, BFFM_SETSTATUSTEXTW, 1, (LPARAM)m_strTip.c_str());

	if (!m_strOKButton.empty())
	{
		::SetWindowText(m_hWndOkButton, m_strOKButton.c_str());
	}
	if (!m_strCancelButton.empty())
	{
		::SetWindowText(m_hWndCancelButton, m_strCancelButton.c_str());
	}


	::GetClientRect(m_hWnd, &m_rcPreClient);

	if (m_nWidth > 0 && m_nHeight > 0)
	{
		RECT rcWnd{ 0,0,0,0 };
		GetWindowRect(m_hWnd, &rcWnd);
		(void)::MoveWindow(m_hWnd, (rcWnd.left + rcWnd.right) / 2 - m_nWidth / 2
			, (rcWnd.top + rcWnd.bottom) / 2 - m_nHeight / 2, m_nWidth, m_nHeight, FALSE);
	}

	_relayout();
}

void CFolderDlg::_relayout()
{
	RECT rcClient{ 0,0,0,0 };
    ::GetClientRect(m_hWnd, &rcClient);
    int nWidthOff = rcClient.right - (m_rcPreClient.right - m_rcPreClient.left);
    int nHeightOff = rcClient.bottom - (m_rcPreClient.bottom - m_rcPreClient.top);

	RECT rcCancelButton{ 0,0,0,0 };
	::GetWindowRect(m_hWndCancelButton, &rcCancelButton);
	::ScreenToClient(m_hWnd, (LPPOINT)&rcCancelButton.left);
	::ScreenToClient(m_hWnd, (LPPOINT)&rcCancelButton.right);
    auto yButton = rcCancelButton.top + nHeightOff;
	auto nBtnWidth = rcCancelButton.right - rcCancelButton.left;
    ::MoveWindow(m_hWndCancelButton, rcCancelButton.left + nWidthOff, yButton
		, nBtnWidth, rcCancelButton.bottom - rcCancelButton.top, FALSE);

    ::MoveWindow(m_hWndOkButton, rcCancelButton.left + nWidthOff - nBtnWidth - 20, yButton
		, nBtnWidth, rcCancelButton.bottom - rcCancelButton.top, FALSE);

    HWND hWndStatic = NULL;
    RECT rcStatic{ 0,0,0,0 };
    while (true)
    {
        hWndStatic = ::FindWindowExA(m_hWnd, hWndStatic, "Static", NULL);
        if (!hWndStatic)
        {
            break;
        }

        (void)::GetWindowRect(hWndStatic, &rcStatic);
        ScreenToClient(m_hWnd, (LPPOINT)&rcStatic.left);
        ScreenToClient(m_hWnd, (LPPOINT)&rcStatic.right);
        rcStatic.top = rcStatic.left;
        rcStatic.bottom = rcStatic.top + 40;
        (void)::MoveWindow(hWndStatic, rcStatic.left, yButton+8
            , rcStatic.right - rcStatic.left + nWidthOff, rcStatic.bottom - rcStatic.left, FALSE);
    }

	HWND hWndTreeCtrl = ::FindWindowExA(m_hWnd, NULL, "SysTreeView32", NULL);
	RECT rcTreeCtrl{ 0,0,0,0 };
    ::GetWindowRect(hWndTreeCtrl, &rcTreeCtrl);

    rcTreeCtrl.left = 0;
    rcTreeCtrl.right = rcClient.right-1;

    rcTreeCtrl.bottom = rcCancelButton.top + nHeightOff - 25;

    /*ScreenToClient(m_hWnd, (LPPOINT)&rcTreeCtrl.left);
    ScreenToClient(m_hWnd, (LPPOINT)&rcTreeCtrl.right);
    rcTreeCtrl.right += nWidthOff;
	if (!m_strTitle.empty())
	{
        rcTreeCtrl.top = rcStatic.bottom;
	}
	else
    {
        //rcTreeCtrl.top = rcStatic.top;
        rcTreeCtrl.top = 0;
    }*/

    rcTreeCtrl.top = 0;

	::SetWindowPos(hWndTreeCtrl, NULL, rcTreeCtrl.left, rcTreeCtrl.top
        , rcTreeCtrl.right - rcTreeCtrl.left + 1, rcTreeCtrl.bottom - rcTreeCtrl.top + 1
        , SWP_NOZORDER);
        //,SWP_HIDEWINDOW);
	
//	__async(10, [=]() {
//		::ShowWindow(hWndTreeCtrl, SW_SHOW);
//		::SetFocus(hWndTreeCtrl);
//	});
}

void CFolderDlg::_handleCallBack(HWND hWnd, UINT uMsg, LPARAM lParam)
{
	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		m_hWnd = hWnd;
		m_hWndOkButton = GetDlgItem(hWnd, IDOK);
		m_hWndCancelButton = GetDlgItem(hWnd, IDCANCEL);

		_initDlg();

		break;
	case BFFM_SELCHANGED:
	{
		//wstring strTip = m_strTip;

		auto pidl = (LPITEMIDLIST)lParam;
		wchar_t pszPath[512]{ 0 };
		if (SHGetPathFromIDList(pidl, pszPath))
		{
			if (!fsutil::existDir(pszPath))
			{
				::EnableWindow(m_hWndOkButton, FALSE);
			}
			else
			{
				::EnableWindow(m_hWndOkButton, TRUE);
				//strTip.append(L" ").append(pszPath);
			}
		}

		//(void)::SendMessage(hWnd, BFFM_SETSTATUSTEXTW, 0, (LPARAM)strTip.c_str());
	}
	
	break;
	default:
		break;
	}
}

wstring CFolderDlg::Show(HWND hWndOwner, LPCWSTR lpszInitialDir, LPCWSTR lpszTitle, LPCWSTR lpszTip
	, LPCWSTR lpszOKButton, LPCWSTR lpszCancelButton, UINT uWidth, UINT uHeight)
{
	if (lpszInitialDir)
	{
		m_strInitialDir = lpszInitialDir;
	}

	if (lpszTitle)
	{
		m_strTitle = lpszTitle;
	}

	if (lpszTip)
	{
		m_strTip = lpszTip;
	}

	if (lpszOKButton)
	{
		m_strOKButton = lpszOKButton;
	}

	if (lpszCancelButton)
	{
		m_strCancelButton = lpszCancelButton;
	}

	cauto rcWorkArea = getWorkArea(true);
	if (0 == uWidth)
	{
		uWidth = rcWorkArea.right * ((rcWorkArea.right > rcWorkArea.bottom) ? 38 : 83) / 100;
	}
	if (0 == uHeight)
	{
		uHeight = getWorkArea(true).bottom * 83 / 100;
	}
	m_nWidth = uWidth;
	m_nHeight = uHeight;

	BROWSEINFO browseInfo;
    ::ZeroMemory(&browseInfo, sizeof(browseInfo));
    browseInfo.ulFlags = __browseInfoFlag;

	browseInfo.hwndOwner = hWndOwner;
    browseInfo.lpfn = _handleCallBack;
	browseInfo.lParam = (LPARAM)this;

	LPITEMIDLIST lpItemIDList = SHBrowseForFolder(&browseInfo);

	m_hWnd = NULL;
	m_hWndOkButton = NULL;
	m_hWndCancelButton = NULL;

	if (NULL == lpItemIDList)
	{
		return L"";
	}

	wchar_t pszPath[MAX_PATH]{ 0 };
    BOOL bRet = SHGetPathFromIDListW(lpItemIDList, pszPath);
    ::CoTaskMemFree(lpItemIDList);
    if (!bRet)
    {
		return L"";
    }

	m_strInitialDir = strutil::rtrim_r(wstring(pszPath), __wcPathSeparator);
	return m_strInitialDir;
}

void CFileDlg::_setOpt(const tagFileDlgOpt& opt)
{
	::ZeroMemory(&m_ofn, sizeof(m_ofn));
	m_ofn.lStructSize = sizeof(m_ofn);

	m_strTitle = opt.strTitle;
	m_ofn.lpstrTitle = m_strTitle.c_str();

	m_strInitialDir = opt.strInitialDir;
	m_ofn.lpstrInitialDir = m_strInitialDir.c_str();

	memzero(m_lpstrFileName);
	if (!opt.strFileName.empty())
	{
		::lstrcat(m_lpstrFileName, opt.strFileName.c_str());
	}
	m_ofn.lpstrFile = m_lpstrFileName;
	m_ofn.nMaxFile = sizeof(m_lpstrFileName) / sizeof(m_lpstrFileName[0]);

	memzero(m_lpstrFilter);
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
	cauto strWorkDir = fsutil::workDir();

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

	fsutil::setWorkDir(strWorkDir);

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

	wstring strFileName = m_lpstrFileName;
	memzero(m_lpstrFileName);

	return strFileName;
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

	wstring strDir;
	if (lstFiles.empty())
	{
		lstFiles.push_back(m_lpstrFileName);

		strDir = fsutil::GetParentDir(m_lpstrFileName);
	}
	else
	{
		strDir = m_lpstrFileName;

		for (auto& strFile : lstFiles)
		{
			strFile = strDir + __wcPathSeparator + strFile;
		}
	}

	memzero(m_lpstrFileName);

	m_strInitialDir = strDir;
	m_ofn.lpstrInitialDir = m_strInitialDir.c_str();

	return strDir;
}

UINT CFileDlg::GetSelFilterIndex() const
{
	return m_ofn.nFilterIndex;
}
