
#pragma once

#define MAX_PATH 260

#ifndef _MSC_VER
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTime>
#endif

#define __UnicodeHead_Lit string({(char)0xff, (char)0xfe})
#define __UnicodeHead_Big string({(char)0xfe, (char)0xff})

#define __UTF8Bom string({(char)0xef, (char)0xbb, (char)0xbf})

#ifdef __ANDROID__
#define __DefEOL E_EOLFlag::eol_n
#else
#define __DefEOL E_EOLFlag::eol_rn
#endif

enum class E_EOLFlag
{
	eol_n = 0,
	eol_rn,
	eol_r
};

class ITxtWriter
{
public:
    virtual bool open(const wstring& strFile, bool bTrunc) = 0;

	virtual size_t write(const wstring& strText) const = 0;
	virtual size_t write(const string& strText) const = 0;
	
	virtual size_t writeln(const wstring& strText) const = 0;
	virtual size_t writeln(const string& strText) const = 0;

    virtual bool close() = 0;
};

class __UtilExt CTxtWriter : public ITxtWriter
{
public:
	CTxtWriter() {}

	CTxtWriter(E_EOLFlag eEOLFlag)
	{
		m_eEOLFlag = eEOLFlag;
	}

	CTxtWriter(bool bUTF8)
	{
		m_bUTF8 = bUTF8;
	}

	CTxtWriter(E_EOLFlag eEOLFlag, bool bUTF8)
	{
		m_eEOLFlag = eEOLFlag;

		m_bUTF8 = bUTF8;
	}

	~CTxtWriter()
	{
		close();
	}

private:
	E_EOLFlag m_eEOLFlag = __DefEOL;

	bool m_bUTF8 = false;

	FILE *m_lpFile = NULL;

protected:
	size_t _write(const void *pData, size_t size) const;

	inline size_t _write(const wstring& strText) const
	{
		return _write(strText.c_str(), strText.size() * sizeof(wchar_t));
	}

	inline size_t _write(const string& strText) const
	{
		return _write(strText.c_str(), strText.size());
	}

public:
	virtual bool open(const wstring& strFile, bool bTrunc) override;

	virtual size_t write(const wstring& strText) const override;

	virtual size_t write(const string& strText) const override
	{
		return _write(strText);
	}

	virtual size_t writeln(const wstring& strText) const override;

	virtual size_t writeln(const string& strText) const override;

	bool close() override;
};

class __UtilExt CUnicodeTxtWriter : public CTxtWriter
{
public:
	CUnicodeTxtWriter(E_EOLFlag eEOLFlag = __DefEOL)
		: CTxtWriter(eEOLFlag)
	{
	}

public:
	bool open(const wstring& strFile, bool bTrunc) override;

	size_t write(const wstring& strText) const override
	{
		return CTxtWriter::_write(strText);
	}

	size_t writeln(const wstring& strText) const override
	{
		return CTxtWriter::writeln(strText);
	}

	size_t write(const string& strText) const override
	{
		return write(wsutil::fromStr(strText));
	}

	size_t writeln(const string& strText) const override
	{
		return writeln(wsutil::fromStr(strText));
	}
};

class __UtilExt CUTF8TxtWriter : public CTxtWriter
{
public:
	CUTF8TxtWriter()
		: CTxtWriter(true)
	{
	}

	CUTF8TxtWriter(E_EOLFlag eEOLFlag)
		: CTxtWriter(eEOLFlag, true)
	{
	}

	CUTF8TxtWriter(bool bWithBom)
		: CTxtWriter(true)
	{
		m_bWithBom = bWithBom;
	}

	CUTF8TxtWriter(E_EOLFlag eEOLFlag, bool bWithBom)
		: CTxtWriter(eEOLFlag)
	{
		m_bWithBom = bWithBom;
	}

private:
	bool m_bWithBom = false;

public:
	bool open(const wstring& strFile, bool bTrunc) override;
};

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
