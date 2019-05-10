
#pragma once

struct __UtilExt tagFindData : public WIN32_FIND_DATAW
{	
	tagFindData()
	{
		memset(this, 0, sizeof tagFindData);
	}

	inline bool isDir() const
	{
		return dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	}

	inline unsigned long getFileSize() const
	{
		return nFileSizeLow;
	}

	inline time_t getModifyTime() const
	{
		return wintime::transFileTime(ftLastWriteTime);
	}
	
	inline wstring getModifyTime(const wstring& strFormat) const
	{
		return wintime::formatTime(getModifyTime(), strFormat);
	}

	inline time_t getCreateTime() const
	{
		return wintime::transFileTime(ftCreationTime);
	}

	inline wstring getCreateTime(const wstring& strFormat) const
	{
		return wintime::formatTime(getCreateTime(), strFormat);
	}
};

class __UtilExt winfsutil
{
public:
	static bool ExistsFile(const wstring& strFile);
	static bool ExistsDir(const wstring& strDir);

	static bool FindFile(const wstring& strFindPath, const function<bool(const tagFindData&)>& cb);
	//static bool FindFile(const wstring& strFindPath, SArray<tagFindData>& arrFindData);

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
