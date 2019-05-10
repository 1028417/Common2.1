
#pragma once

class __UtilExt winfsutil
{
public:
	static bool ExistsFile(const wstring& strFile);
	static bool ExistsDir(const wstring& strDir);

	using CB_FindFile = const function<bool(const tagFileInfo&)>&;
	static bool FindFile(const wstring& strFindPath, CB_FindFile cb);

	static void GetSysDrivers(list<wstring>& lstDrivers);
	
	static bool DeletePath(const wstring& strPath, HWND hwndParent, const wstring& strTitle=L"");

	static void ExploreDir(const wstring& strDir, HWND hWnd = NULL);

	static void ExploreFiles(const list<wstring>& lstPath, HWND hWnd=NULL);
	static void ExploreFile(const wstring& strPath, HWND hWnd = NULL);

	static bool CreateDir(const wstring& strDir);

	// 获取文件夹图标
	static HICON getFolderIcon();
	// 获取文件图标
	static HICON getFileIcon(const wstring& extention);
};
