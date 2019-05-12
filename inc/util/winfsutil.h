
#pragma once

#include <Windows.h>

class __UtilExt winfsutil
{
public:
	static time64_t transFileTime(const FILETIME& ft);
	static wstring formatFileTime(const FILETIME& fileTime, const wstring& strFormat);

	static void getSysDrivers(list<wstring>& lstDrivers);
	
	static bool removeDir(const wstring& strPath, HWND hwndParent, const wstring& strTitle=L"");

	static void exploreDir(const wstring& strDir, HWND hWnd = NULL);

	static void exploreFiles(const list<wstring>& lstPath, HWND hWnd=NULL);
	static void exploreFile(const wstring& strPath, HWND hWnd = NULL);

	// 获取文件夹图标
	static HICON getFolderIcon();
	// 获取文件图标
	static HICON getFileIcon(const wstring& extention);
};
