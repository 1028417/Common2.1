
#pragma once

//CCFolderDlg

class __CommonExt CFolderDlg
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


struct tagFileDlgOpt
{
	wstring strTitle;
	
	wstring strInitialDir;
	
	wstring strFileName;
	
	wstring strFilter;

	bool bMultiSel = false;
	
	bool bMustExist = false;

	CWnd *pWndOwner = NULL;
};

class __CommonExt CFileDlg
{
public:
	CFileDlg() {}

	CFileDlg(const tagFileDlgOpt& opt)
	{
		_setOpt(opt);
	}

private:
	void _setOpt(const tagFileDlgOpt& opt);

	void _getMultSel(list<wstring>& lstFiles);

private:
	OPENFILENAME m_ofn;

	TCHAR m_lpstrFilter[512];

	wstring m_strInitialDir;

	wstring m_strTitle;

	TCHAR m_lpstrFileName[10240];

public:
	void Show(list<wstring>& lstFiles);

	void Show(const tagFileDlgOpt& opt, list<wstring>& lstFiles);
};
