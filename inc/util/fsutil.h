
#pragma once

#define MAX_PATH 260

#define __strUnicodeBOM wstring({ 0xff, 0xfe }) // Unicode头

template <bool _ToUTF8 = false, bool _WithBOM = false>
class CTxtWriter
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
	bool open(const wstring& strFile, bool bTrunc = false)
	{
		wstring strMode(bTrunc ? L"w" : L"a");
		if (_ToUTF8)
		{
			strMode.append(L",ccs=UTF-8");
		}
		else
		{
			strMode.append(L"b");
		}

#ifdef __ANDROID__
		m_lpFile = fopen(util::WSToAsc(strFile).c_str(), util::WSToAsc(strMode).c_str());
#else
		__EnsureReturn(0 == _wfopen_s(&m_lpFile, strFile.c_str(), strMode.c_str()), false);
#endif
		__EnsureReturn(m_lpFile, false);

		if (_WithBOM)
		{
			(void)write(__strUnicodeBOM);
		}

		return true;
	}

	size_t write(const wstring& strData)
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
	static const wchar_t dot = L'.';
	static const wchar_t backSlant = L'\\';

	using FN_WriteTxt = const function<void(const wstring&)>&;
	template <bool _ToUTF8 = false , bool _WithBOM = false>
	static bool saveTxt(const wstring& strFile
		, const function<void(FN_WriteTxt fnWriteTxt)>& cb, bool bTrunc = false)
	{
		CTxtWriter<_ToUTF8, _WithBOM> TxtWriter;
		if (!TxtWriter.open(strFile, bTrunc))
		{
			return false;
		}

		auto fnWrite = [&](const wstring& strData) {
			TxtWriter.write(strData);
		};
		cb(fnWrite);

		TxtWriter.close();
	}

	template <bool _ToUTF8 = false, bool _WithBOM = false>
	static bool saveTxt(const wstring& strFile, const wstring& strData, bool bTrunc = false)
	{
		return saveTxt<_ToUTF8, _WithBOM>(strFile, [&](FN_WriteTxt cb) {
			cb(strData);
		}, bTrunc);
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
