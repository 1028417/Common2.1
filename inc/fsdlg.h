
#pragma once

//CCFolderDlg

class __CommonPrjExt CFolderDlg
{
public:
	CFolderDlg(LPCTSTR lpszInitDir=NULL, LPCTSTR lpszTitle=NULL, LPCTSTR lpszMessage=NULL
		, LPCTSTR lpszOKButton=NULL, LPCTSTR lpszCancelButton=NULL, int nWidth=0, int nHeight=0)
	{
		m_cstrInitDir = lpszInitDir;

		m_cstrTitle = lpszTitle;
		m_cstrMessage = lpszMessage;
		m_cstrOKButton = lpszOKButton;
		m_cstrCancelButton = lpszCancelButton;

		m_nWidth = nWidth;
		m_nHeight = nHeight;
	}

public:
	CString m_cstrInitDir;

	CString m_cstrTitle;
	CString m_cstrMessage;
	CString m_cstrOKButton;
	CString m_cstrCancelButton;

	int m_nWidth;
	int m_nHeight;

public:
	CString Show(CWnd& WndOwner, LPCTSTR lpszInitDir=NULL, LPCTSTR lpszTitle=NULL, LPCTSTR lpszMessage=NULL
		, LPCTSTR lpszOKButton=NULL, LPCTSTR lpszCancelButton=NULL, int nWidth=0, int nHeight=0);

	static int __stdcall BrowseFolderCallBack(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpParam);
};


class __CommonPrjExt CFileDlg
{
public:
	CFileDlg(const wstring& strTitle, const wstring& strFilter=L""
		, const wstring& strInitialDir=L"", const wstring& strFileName=L"", CWnd *pWndOwner=NULL);
	
private:
	OPENFILENAME m_ofn;

	TCHAR m_lpstrFilter[512];

	wstring m_strInitialDir;

	wstring m_strTitle;

	TCHAR m_lpstrFileName[4096];

public:
	wstring Show();
	void ShowMultiSel(list<wstring>& lstFiles);
};
