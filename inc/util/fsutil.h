﻿
#pragma once

#define MAX_PATH 260

#if !__winvc
#include <QFileInfo>
#include <QFile>
#include <QDir>

#include <QTime>
#endif

#if __windows
#include <direct.h>

#include <sys/utime.h>
#else
#include <utime.h>

using _utimbuf = struct utimbuf;
#define _wutime(f, t) utime(strutil::toStr(f).c_str(), t)
#endif

#include <sys/stat.h>

#if __windows
using tagFileStat = struct _stat;
using tagFileStat32 = struct _stat32;
using tagFileStat32_64 = struct _stat32i64;
using tagFileStat64_32 = struct _stat64i32;
using tagFileStat64 = struct _stat64;

#else
using tagFileStat = struct stat;
using tagFileStat32 = tagFileStat;
using tagFileStat32_64 = tagFileStat;
using tagFileStat64_32 = tagFileStat;
using tagFileStat64 = tagFileStat;

#define _fileno fileno
#endif

enum class E_SeekFileFlag
{
	SFF_Set = SEEK_SET,
	SFF_Cur = SEEK_CUR,
	SFF_End = SEEK_END
};

enum class E_FindFindFilter
{
	FFP_None
	, FFP_ByPrefix
	, FFP_ByExt
};

struct __UtilExt tagFileInfo
{
	tagFileInfo() {}

	tagFileInfo(const wstring& t_strName)
		: strName(t_strName)
	{
	}

	class CPath * pParent = NULL;

	bool bDir = true;

	wstring strName;

	unsigned long uFileSize = 0;

	time64_t tCreateTime = 0;
	time64_t tModifyTime = 0;
};


#define __wcSlant L'/'
#define __wcBackSlant L'\\'

#if __windows
	#define __wcFSSlant	__wcBackSlant
#else
	#define __wcFSSlant	__wcSlant
#endif

#define __wcDot L'.'

class __UtilExt fsutil
{
public:
    inline static bool checkPathTail(wchar_t wch)
    {
        return __wcBackSlant == wch || __wcSlant == wch;
    }

	static void trimPathTail(wstring& strPath);

	static wstring trimPathTail_r(const wstring& strPath);

	static void transFSSlant(wstring& strPath)
	{
#if __windows
        strutil::replaceChar(strPath, __wcSlant, __wcBackSlant);
#else
        strutil::replaceChar(strPath, __wcBackSlant, __wcSlant);
#endif
	}

    static FILE* fopen(const string& strFile, const string& strMode);
    static FILE* fopen(const wstring& strFile, const string& strMode);

    static bool loadBinary(const wstring& strFile, TD_ByteVector& vecBuff, UINT uReadSize = 0);

	static bool loadTxt(const wstring& strFile, string& strText);
	static bool loadTxt(const wstring& strFile, cfn_bool_t<const string&> cb);
	static bool loadTxt(const wstring& strFile, SVector<string>& vecLineText);

    static bool copyFile(const wstring& strSrcFile, const wstring& strDstFile);

	using CB_CopyFile = function <bool(char *lpData, size_t size)>;
    static bool copyFileEx(const wstring& strSrcFile, const wstring& strDstFile, const CB_CopyFile& cb=NULL);

    static bool fileStat(FILE *pf, tagFileStat& stat);
	static bool fileStat(const wstring& strFile, tagFileStat& stat);

    static bool fileStat32(FILE *pf, tagFileStat32& stat);
	static bool fileStat32(const wstring& strFile, tagFileStat32& stat);
    static bool fileStat32_64(FILE *pf, tagFileStat32_64& stat);
	static bool fileStat32_64(const wstring& strFile, tagFileStat32_64& stat);

    static bool fileStat64(FILE *pf, tagFileStat64& stat);
	static bool fileStat64(const wstring& strFile, tagFileStat64& stat);
    static bool fileStat64_32(FILE *pf, tagFileStat64_32& stat);
	static bool fileStat64_32(const wstring& strFile, tagFileStat64_32& stat);

    static int GetFileSize(FILE *pf);
	static int GetFileSize(const wstring& strFile);
    static int64_t GetFileSize64(FILE *pf);
	static int64_t GetFileSize64(const wstring& strFile);

    static time32_t GetFileModifyTime(FILE *pf);
	static time32_t GetFileModifyTime(const wstring& strFile);
    static time64_t GetFileModifyTime64(FILE *pf);
	static time64_t GetFileModifyTime64(const wstring& strFile);

	static void SplitPath(const wstring& strPath, wstring *pstrDir, wstring *pstrFile);

	static wstring GetRootDir(const wstring& strPath);
	static wstring GetParentDir(const wstring& strPath);

	static wstring GetFileName(const wstring& strPath);
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

    static int64_t seekFile(FILE *pf, int64_t offset, E_SeekFileFlag eFlag=E_SeekFileFlag::SFF_Set);

    static wstring workDir();
    static bool setWorkDir(const wstring& strWorkDir);

#if __windows
    static wstring getModuleDir(wchar_t *pszModuleName = NULL);
#endif

#if !__winvc
    static wstring getAppDir();
#endif

#if __mac
    static wstring getMacHomeDir();
#endif

	using CB_FindFile = cfn_void_t<tagFileInfo&>;
	static bool findFile(const wstring& strDir, CB_FindFile cb
		, E_FindFindFilter eFilter = E_FindFindFilter::FFP_None, const wchar_t *pstrFilter = NULL);
	
	static bool findSubDir(const wstring& strDir, cfn_void_t<const wstring&> cb
		, E_FindFindFilter eFilter = E_FindFindFilter::FFP_None, const wchar_t *pstrFilter = NULL)
	{
		return findFile(strDir, [&](tagFileInfo& fileInfo) {
			if (fileInfo.bDir)
			{
				cb(fileInfo.strName);
			}
		}, eFilter, pstrFilter);
	}

	static bool findSubFile(const wstring& strDir, cfn_void_t<const wstring&> cb
		, E_FindFindFilter eFilter = E_FindFindFilter::FFP_None, const wchar_t *pstrFilter = NULL)
	{
		return findFile(strDir, [&](tagFileInfo& fileInfo) {
			if (!fileInfo.bDir)
			{
				cb(fileInfo.strName);
			}
		}, eFilter, pstrFilter);
	}

#if !__winvc
    static long qCompressFile(const wstring& strSrcFile, const wstring& strDstFile, int nCompressLecvel=-1);
    static long qUncompressFile(const wstring& strSrcFile, const wstring& strDstFile);
#endif

    static long zCompressFile(const wstring& strSrcFile, const wstring& strDstFile, int level=0);
    static long zUncompressFile(const wstring& strSrcFile, const wstring& strDstFile);

    static bool zUncompressZip(const string& strZipFile, const string& strDstDir);
};


class __UtilExt bstream
{
protected:
    ~bstream()
    {
        close();
    }

	bstream() {}

	bstream(FILE *pf)
		: m_pf(pf)
		, m_uSize(fsutil::GetFileSize(m_pf))
	{
	}

    bstream(const wstring& strFile, const string& strMode)
    {
        (void)_open(strFile, strMode);
    }

protected:
    FILE *m_pf = NULL;

	size_t m_uSize = 0;

protected:
    bool _open(const wstring& strFile, const string& strMode)
    {
		m_pf = fsutil::fopen(strFile, strMode);
		if (NULL == m_pf)
		{
			return false;
		}
		
		m_uSize = fsutil::GetFileSize(m_pf);

		return true;
    }
	
public:
    operator bool() const
    {
        return m_pf != NULL;
    }

    bool is_open() const
    {
        return m_pf != NULL;
    }

	size_t size() const
	{
		return m_uSize;
	}

    bool eof() const
    {
        if (NULL == m_pf)
        {
            return true;
        }

        return feof(m_pf);
    }

    void close()
    {
        if (m_pf)
        {
            fclose(m_pf);            
            m_pf = NULL;
        }
    }
};

class __UtilExt ibstream : public bstream
{
public:
	ibstream(FILE *pf = NULL)
		: bstream(pf)
    {
	}

    ibstream(const wstring& strFile)
    {
        (void)open(strFile);
    }

public:
    bool open(const wstring& strFile)
    {
        return _open(strFile, "rb");
    }
	
    size_t read(void *buff, size_t buffSize) const
    {
        return fread(buff, 1, buffSize, m_pf);
    }
};

class __UtilExt obstream : public bstream
{
public:
    obstream(FILE *pf = NULL)
		: bstream(pf)
	{
	}

    obstream(const wstring& strFile, bool bTrunc)
    {
        (void)open(strFile, bTrunc);
    }
	
public:
    bool open(const wstring& strFile, bool bTrunc)
    {
        return _open(strFile, bTrunc?"wb":"ab");
    }

    bool write(const void *buff, size_t buffSize) const
    {
        return fwrite(buff, 1, buffSize, m_pf) == buffSize;
    }
};
