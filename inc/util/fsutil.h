
#pragma once

class __UtilExt fsutil
{
public:
	static const wchar_t dot = L'.';
	static const wchar_t backSlant = L'\\';

	using FN_WriteTxt = const function<void(const wstring&)>&;
	static bool saveTxt(const wstring& strFile
		, const function<void(FN_WriteTxt fnWriteTxt)>& cb, bool bTrunc=true, bool bToUTF8=false);
	static bool saveTxt(const wstring& strFile, const wstring& strData, bool bTrunc=true, bool bToUTF8 = false);

	static bool loadBinary(const wstring& strFile, vector<char>& vecstrData, UINT uReadSize = 0);
	
	static bool loadTxt(const wstring& strFile, string& strData);
	static bool loadTxt(const wstring& strFile, const function<bool(const string&)>& cb, char cdelimiter = '\n');
	static bool loadTxt(const wstring& strFile, SVector<string>& vecLineData, char cdelimiter = '\n');

	static int GetFileSize(const wstring& strFilePath);
	static __time64_t GetFileModifyTime(const wstring& strFilePath);

	static void SplitPath(const wstring& strPath, wstring *pstrDir, wstring *pstrFile);

	static wstring GetFileName(const wstring& strPath);

	static void GetFileName(const wstring& strPath, wstring *pstrTitle, wstring *pstrExtName);

	static wstring getFileTitle(const wstring& strPath);

	static wstring GetFileExtName(const wstring& strPath);

	static wstring GetParentDir(const wstring& strPath);

	static bool CheckSubPath(const wstring& strDir, const wstring& strSubPath);

	static wstring GetOppPath(const wstring& strPath, const wstring strBaseDir);
};

struct __UtilExt tagFindData
{
	WIN32_FIND_DATAW data;
	
	tagFindData()
	{
		memset(&data, 0, sizeof data);
	}

	inline wstring getFileName() const
	{
		return data.cFileName;
	}

	inline bool isDir() const
	{
		return data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	}

	inline unsigned long getFileSize() const
	{
		return data.nFileSizeLow;
	}

	inline time_t getModifyTime() const
	{
		return util::FileTimeToTime_t(data.ftLastWriteTime);
	}
	
	inline wstring getModifyTime(const wstring& strFormat) const
	{
		return util::FormatTime(getModifyTime(), strFormat);
	}

	inline time_t getCreateTime() const
	{
		return util::FileTimeToTime_t(data.ftCreationTime);
	}

	inline wstring getCreateTime(const wstring& strFormat) const
	{
		return util::FormatTime(getCreateTime(), strFormat);
	}
};

class __UtilExt fsutil_win
{
public:
	static bool ExistsFile(const wstring& strFile);
	static bool ExistsDir(const wstring& strDir);

	static bool FindFile(const wstring& strFindPath, const function<bool(const tagFindData&)>& cb);
	//static bool FindFile(const wstring& strFindPath, SArray<tagFindData>& arrFindData);

	static void GetSysDrivers(list<wstring>& lstDrivers);
	
	static bool DeletePath(const wstring& strPath, HWND hwndParent, const wstring& strTitle=L"");

	static bool copyFile(const wstring& strSrcFile, const wstring& strDstFile, bool bSyncModifyTime=false);

	static void ExplorePath(const list<wstring>& lstPath);
	static void ExplorePath(const wstring& strPath);

	static bool CreateDir(const wstring& strDir);

	// 获取文件夹图标
	static HICON getFolderIcon();
	// 获取文件图标
	static HICON getFileIcon(const wstring& extention);
};
