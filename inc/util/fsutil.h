
#pragma once

#define MAX_PATH 260

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

	static bool copyFile(const wstring& strSrcFile, const wstring& strDstFile, bool bSyncModifyTime = false);

	static int GetFileSize(const wstring& strFile);
	static time64_t GetFileModifyTime(const wstring& strFile);

	static void SplitPath(const wstring& strPath, wstring *pstrDir, wstring *pstrFile);

	static wstring GetFileName(const wstring& strPath);

	static void GetFileName(const wstring& strPath, wstring *pstrTitle, wstring *pstrExtName);

	static wstring getFileTitle(const wstring& strPath);

	static wstring GetFileExtName(const wstring& strPath);

	static wstring GetParentDir(const wstring& strPath);

	static bool CheckSubPath(const wstring& strDir, const wstring& strSubPath);

	static wstring GetOppPath(const wstring& strPath, const wstring strBaseDir);

	static bool fileExists(const wstring& strFile);
	static bool dirExists(const wstring& strDir);

	static void createDir(const wstring& strDir);
	static bool removeFile(const wstring& strFile);

	static wstring currentDir();
};
