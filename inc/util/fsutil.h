
#pragma once

#define __Dot L'.'

#define	__Slant L'/'

#define	__BackSlant L'\\'

struct __UtilExt tagFindData : WIN32_FIND_DATAW
{
	tagFindData()
	{
	}

	wstring getFileName() const
	{
		return data.cFileName;
	}

	bool isDir() const
	{
		return data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	}

	WIN32_FIND_DATAW data;
};

class __UtilExt fsutil
{
public:
	static int GetFileSize(const wstring& strFilePath);

	static void SplitPath(const wstring& strPath, wstring *pstrDir, wstring *pstrFile);

	static wstring GetFileName(const wstring& strPath);

	static void GetFileName(const wstring& strPath, wstring *pstrTitle, wstring *pstrExtName);

	static wstring getFileTitle(const wstring& strPath);

	static wstring GetFileExtName(const wstring& strPath);

	static wstring GetParentDir(const wstring& strPath);

	static bool CheckSubPath(const wstring& strDir, const wstring& strSubPath);

	static wstring GetOppPath(const wstring& strPath, const wstring strBaseDir);
};

class __UtilExt fsutil_win
{
public:
	static bool ExistsFile(const wstring& strFile);
	static bool ExistsPath(const wstring& strDir);

	static bool FindFile(const wstring& strFindPath, const function<bool(const tagFindData&)>& cb);
	static bool FindFile(const wstring& strFindPath, SArray<tagFindData>& arrFindData);

	static void GetSysDrivers(list<wstring>& lstDrivers);
	
	static bool DeletePath(const wstring& strPath, HWND hwndParent, const wstring& strTitle=L"");

	static bool CopyFile(const wstring& strSrcFile, const wstring& strSnkFile);

	static void ExplorePath(const list<wstring>& lstPath);

	static bool CreateDir(const wstring& strDir);

	// 获取文件夹图标
	static HICON getFolderIcon();
	// 获取文件图标
	static HICON getFileIcon(const wstring& extention);
};
