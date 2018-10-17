
#pragma once

class __CommonPrjExt fsutil
{
public:
	static void SplitPath(const wstring& strPath, wstring *pstrDir, wstring *pstrFile);

	static wstring GetFileName(const wstring& strPath);

	static void GetFileName(const wstring& strPath, wstring *pstrTitle, wstring *pstrExtName);

	static wstring GetFileTitle(const wstring& strPath);

	static wstring GetFileExtName(const wstring& strPath);

	static wstring GetParentPath(const wstring& strPath);

	static bool CheckSubPath(const wstring& strPath, const wstring& strSubPath);

	static wstring GetOppPath(const wstring& strPath, const wstring strBasePath);

	static void GetSysDrivers(list<wstring>& lstDrivers);
	
	static bool DeletePath(const wstring& strPath, CWnd *pwndParent=NULL, const wstring& strTitle=L"");

	static bool CopyFile(const wstring& strSrcFile, const wstring& strSnkFile);

	static void ExplorePath(const list<wstring>& lstPath);

	static bool CreateDir(const wstring& strDir);

	static void FindFile(const wstring& strPath, map<wstring, wstring>& mapFiles);

	static bool ExistsFile(const wstring& strFile);
	static bool ExistsPath(const wstring& strDir);

	// 获取文件夹图标
	static HICON getFolderIcon();
	// 获取文件图标
	static HICON getFileIcon(const wstring& extention);
};

#include <Path.h>

#include <fsdlg.h>
