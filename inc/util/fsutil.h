
#pragma once

#define MAX_PATH 260

#ifndef _MSC_VER
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTime>
#endif

#define __UnicodeHead string({(char)0xff, (char)0xfe})
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
	virtual size_t write(const wstring& strText) const = 0;
	virtual size_t write(const string& strText) const = 0;
	
	virtual size_t writeln(const wstring& strText) const = 0;
	virtual size_t writeln(const string& strText) const = 0;
};

class __UtilExt fsutil
{
public:
class CTxtWriter : public ITxtWriter
{
public:
	CTxtWriter(E_EOLFlag eEOLFlag = __DefEOL)
	{
		m_eEOLFlag = eEOLFlag;
	}

	~CTxtWriter()
	{
		close();
	}

private:
	E_EOLFlag m_eEOLFlag;
	
	FILE *m_lpFile = NULL;

	bool m_bUTF8 = false;

protected:
	inline size_t _write(const void *pData, size_t size) const
    {
		if (NULL == m_lpFile)
		{
			return 0;
		}

		if (NULL == pData || 0 == size)
		{
			return 0;
		}

        size_t ret = fwrite(pData, size, 1, m_lpFile);

#ifdef _DEBUG
		(void)fflush(m_lpFile);
#endif

		return ret;
	}

	inline size_t _write(const wstring& strText) const
	{
		return _write(strText.c_str(), strText.size()*sizeof(wchar_t));
	}

	inline size_t _write(const string& strText) const
	{
		return _write(strText.c_str(), strText.size());
	}

public:
	bool open(const wstring& strFile, bool bTrunc = false, bool bUTF8 = false)
	{
		m_bUTF8 = bUTF8;

		wstring strMode(bTrunc ? L"w" : L"a");
		if (bUTF8)
		{
			strMode.append(L"b,ccs=UTF-8");
		}
		else
		{
			strMode.append(L"b");
		}

#ifdef __ANDROID__
		m_lpFile = fopen(wstrutil::toStr(strFile).c_str(), wstrutil::toStr(strMode).c_str());
#else
		__EnsureReturn(0 == _wfopen_s(&m_lpFile, strFile.c_str(), strMode.c_str()), false);
#endif
		__EnsureReturn(m_lpFile, false);
	
		return true;
	}

	virtual size_t write(const wstring& strText) const override
	{
		if (m_bUTF8)
		{
			return _write(wstrutil::toUTF8(strText));
		}
		else
		{
			return _write(strText);
		}
	}

	virtual size_t write(const string& strText) const override
	{
		return _write(strText);
	}

	virtual size_t writeln(const wstring& strText) const override
	{
		if (E_EOLFlag::eol_n == m_eEOLFlag)
		{
			return write(strText + L"\n");
		}
		else if (E_EOLFlag::eol_rn == m_eEOLFlag)
		{
			return write(strText + L"\r\n");
		}
		else
		{
			return write(strText + L"\r");
		}
	}

	virtual size_t writeln(const string& strText) const override
	{
		if (E_EOLFlag::eol_n == m_eEOLFlag)
		{
			return write(strText + "\n");
		}
		else if (E_EOLFlag::eol_rn == m_eEOLFlag)
		{
			return write(strText + "\r\n");
		}
		else
		{
			return write(strText + "\r");
		}
	}

    bool close()
	{
		if (NULL != m_lpFile)
		{
			if (-1 == fclose(m_lpFile))
			{
				return false;
			}

			m_lpFile = NULL;
		}

		return true;
	}
};

class CUnicodeTxtWriter : public CTxtWriter
{
public:
	CUnicodeTxtWriter(E_EOLFlag eEOLFlag = __DefEOL)
		: CTxtWriter(eEOLFlag)
	{
	}
	
public:
	bool open(const wstring& strFile, bool bTrunc = false)
	{
		bool bExists = fsutil::fileExists(strFile);

		if (!CTxtWriter::open(strFile, bTrunc))
		{
			return false;
		}

		if (!bExists || bTrunc)
		{
			(void)CTxtWriter::_write(__UnicodeHead);
		}

		return true;
	}

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
		return write(wstrutil::fromStr(strText));
	}

	size_t writeln(const string& strText) const override
	{
		return writeln(wstrutil::fromStr(strText));
	}
};

class CUTF8TxtWriter : public CTxtWriter
{
public:
	CUTF8TxtWriter(E_EOLFlag eEOLFlag = __DefEOL)
		: CTxtWriter(eEOLFlag)
	{
	}

public:
	bool open(const wstring& strFile, bool bTrunc = false, bool bWithBom = true)
	{
		bool bExists = fsutil::fileExists(strFile);

		if (!CTxtWriter::open(strFile, bTrunc, true))
		{
			return false;
		}

		if (bWithBom)
		{
			if (!bExists || bTrunc)
			{
				(void)CTxtWriter::write(__UTF8Bom);
			}
		}

		return true;
	}
};

public:
	static const wchar_t wchDot = L'.';
	static const wchar_t wchBackSlant = L'\\';

	static bool loadBinary(const wstring& strFile, vector<char>& vecText, UINT uReadSize = 0);
	
	static bool loadTxt(const wstring& strFile, string& strText);
	static bool loadTxt(const wstring& strFile, const function<bool(const string&)>& cb, char cdelimiter = '\n');
	static bool loadTxt(const wstring& strFile, SVector<string>& vecLineText, char cdelimiter = '\n');

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
    static bool removeDir(const wstring& strDir);
    static bool removeFile(const wstring& strFile);

	static bool moveFile(const wstring& strSrcFile, const wstring& strDstFile);

	static wstring currentDir();

	using CB_FindFile = const function<bool(const tagFileInfo&)>&;
	static bool findFile(const wstring& strFindPath, CB_FindFile cb);
};
