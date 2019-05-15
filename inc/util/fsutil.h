
#pragma once

#define MAX_PATH 260

#ifndef _MSC_VER
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QTime>
#endif

#ifdef __ANDROID__
#define __WriteLineFlag E_WriteLineFlag::WLF_N
#else
#define __WriteLineFlag E_WriteLineFlag::WLF_RN
#endif

enum class E_WriteLineFlag
{
	WLF_N = 0,
	WLF_RN,
	WLF_R
};

class ITxtWriter
{
public:
    virtual 	size_t write(const wstring& strData) = 0;
	virtual 	size_t writeln(const wstring& strData) = 0;
};

#define __UnicodeBOM string({(char)0xff, (char)0xfe})
#define __UTF8Head string({(char)0xef, (char)0xbb, (char)0xbf})

template <E_WriteLineFlag _Flag = __WriteLineFlag>
class CTxtWriter : public ITxtWriter
{
public:
	CTxtWriter(const wstring& strFile = L"", bool bTrunc = false)
	{
		if (!strFile.empty())
		{
			(void)open(strFile, bTrunc);
		}
	}

	~CTxtWriter()
	{
		close();
	}

private:
	FILE *m_lpFile = NULL;

public:
    bool open(const wstring& strFile, bool bTrunc = false, bool bToUTF8 = false)
	{
		wstring strMode(bTrunc ? L"w" : L"a");
        if (bToUTF8)
		{
			strMode.append(L",ccs=UTF-8");
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

        if (bTrunc)
        {
            (void)write(wstrutil::fromStr(bToUTF8?__UTF8Head:__UnicodeBOM));
        }

		return true;
	}

	size_t write(const wstring& strData) override
	{
		if (NULL == m_lpFile)
		{
			return 0;
		}

		if (strData.empty())
		{
			return 0;
		}

		return fwrite(strData.c_str(), strData.size() * sizeof(wchar_t), 1, m_lpFile);
	}

	size_t writeln(const wstring& strData) override
	{
		if (E_WriteLineFlag::WLF_N == _Flag)
		{
			return write(strData + L"\n");
		}
		else if (E_WriteLineFlag::WLF_RN == _Flag)
		{
			return write(strData + L"\r\n");
		}
		else
		{
			return write(strData + L"\r");
		}
	}
	
    bool isOpened()
	{
		return NULL != m_lpFile;
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

class __UtilExt fsutil
{
public:
	static const wchar_t wchDot = L'.';
	static const wchar_t wchBackSlant = L'\\';

    using FN_WriteTxt = const function<void(const wstring&)>&;
    template <E_WriteLineFlag _Flag = __WriteLineFlag>
	static bool saveTxt(const wstring& strFile
        , const function<void(FN_WriteTxt fnWriteTxt)>& cb, bool bTrunc = false, bool bToUTF8 = false)
	{
        CTxtWriter<_Flag> TxtWriter;
        if (!TxtWriter.open(strFile, bTrunc, bToUTF8))
		{
			return false;
		}

		auto fnWrite = [&](const wstring& strData) {
			(void)TxtWriter.write(strData);
		};
		cb(fnWrite);

		(void)TxtWriter.close();

		return true;
	}

    template <E_WriteLineFlag _Flag = __WriteLineFlag>
    static bool saveTxt(const wstring& strFile, const wstring& strData, bool bTrunc = false, bool bToUTF8 = false)
	{
        return saveTxt<_Flag>(strFile, [&](FN_WriteTxt cb) {
			cb(strData);
        }, bTrunc, bToUTF8);
	}

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
    static bool removeDir(const wstring& strDir);
    static bool removeFile(const wstring& strFile);

	static bool moveFile(const wstring& strSrcFile, const wstring& strDstFile);

	static wstring currentDir();

	using CB_FindFile = const function<bool(const tagFileInfo&)>&;
	static bool findFile(const wstring& strFindPath, CB_FindFile cb);
};
