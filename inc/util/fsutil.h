﻿
#pragma once

#define MAX_PATH 260

#ifndef _MSC_VER
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTime>
#endif

struct tagFileInfo
{
	tagFileInfo()
	{
	}

	tagFileInfo(const tagFileInfo& FileInfo)
	{
		*this = FileInfo;
	}

	bool m_bDir = false;

	wstring m_strName;

	unsigned long m_uFileSize = 0;

	time64_t m_tCreateTime = 0;
	time64_t m_tModifyTime = 0;
};

class __UtilExt fsutil
{
public:
	static const wchar_t wcDot = L'.';
	static const wchar_t wcBackSlant = L'\\';
	static const wchar_t wcSlant = L'/';

	static bool loadBinary(const wstring& strFile, vector<char>& vecText, UINT uReadSize = 0);
	
	static bool loadTxt(const wstring& strFile, string& strText);
	static bool loadTxt(const wstring& strFile, const function<bool(const string&)>& cb, char cdelimiter = '\n');
	static bool loadTxt(const wstring& strFile, SVector<string>& vecLineText, char cdelimiter = '\n');

	static bool copyFile(const wstring& strSrcFile, const wstring& strDstFile, bool bSyncModifyTime = false);

	static int GetFileSize(const wstring& strFile);
	static time64_t GetFileModifyTime(const wstring& strFile);

	static void SplitPath(const wstring& strPath, wstring *pstrDir, wstring *pstrFile);

	static wstring GetRootDir(const wstring& strPath);
	static wstring GetParentDir(const wstring& strPath);

	static wstring GetFileName(const wstring& strPath);

	static void GetFileName(const wstring& strPath, wstring *pstrTitle, wstring *pstrExtName);

	static wstring getFileTitle(const wstring& strPath);

	static wstring GetFileExtName(const wstring& strPath);

	static bool CheckSubPath(const wstring& strDir, const wstring& strSubPath);

	static wstring GetOppPath(const wstring& strPath, const wstring strBaseDir);

	static bool existPath(const wstring& strPath, bool bDir);
    static bool existDir(const wstring& strDir);
	static bool existFile(const wstring& strFile);

    static bool createDir(const wstring& strDir);
    static bool removeDir(const wstring& strDir);
    static bool removeFile(const wstring& strFile);

	static bool moveFile(const wstring& strSrcFile, const wstring& strDstFile);

    static wstring startupDir();

	enum class E_FindFindFilter
	{
		FFP_None
		, FFP_ByPrefix
		, FFP_ByExt
	};
	using CB_FindFile = const function<void(const tagFileInfo&)>&;
	static bool findFile(const wstring& strDir, CB_FindFile cb
		, E_FindFindFilter eFilter = E_FindFindFilter::FFP_None, const wstring& strFilter = L"");
};
