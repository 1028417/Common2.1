
#pragma once

#include <Windows.h>

class __UtilExt CFolderDlg
{
public:
	CFolderDlg()
	{
	}

public:
	wstring m_strInitialDir;

	wstring m_strTitle;
	
	wstring m_strTip;
	
	wstring m_strOKButton;
	wstring m_strCancelButton;

	int m_nWidth = 0;
	int m_nHeight = 0;

public:
	wstring Show(HWND hWndOwner, LPCWSTR lpszInitialDir=NULL, LPCWSTR lpszTitle=NULL, LPCWSTR lpszTip=NULL
		, LPCWSTR lpszOKButton=NULL, LPCWSTR lpszCancelButton=NULL, UINT uWidth=0, UINT uHeight=0);

	static int __stdcall BrowseFolderCallBack(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpParam);
};


struct tagFileDlgOpt
{
	wstring strTitle;
	
	wstring strInitialDir;
	
	wstring strFileName;
	
	wstring strFilter;
		
	bool bMustExist = false;

	HWND hWndOwner = NULL;
};

class __UtilExt CFileDlg
{
public:
	CFileDlg() {}

	CFileDlg(const tagFileDlgOpt& opt)
	{
		_setOpt(opt);
	}

private:
	OPENFILENAME m_ofn;

	TCHAR m_lpstrFilter[512];

	wstring m_strInitialDir;

	wstring m_strTitle;

	TCHAR m_lpstrFileName[10240];

protected:
	void _setOpt(const tagFileDlgOpt& opt);

private:
	bool _show(bool bSaveFile);

	wstring _getMultSel(list<wstring>& lstFiles);

public:
	wstring ShowSave();

	wstring ShowOpenSingle();
	wstring ShowOpenMulti(list<wstring>& lstFiles);

	UINT GetSelFilterIndex() const;
};
