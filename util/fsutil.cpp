﻿
#include "util.h"

#include "fsutil.h"

#include <sys/utime.h>
#include <sys/stat.h>

class ibstream : public ifstream
{
public:
	ibstream() {}

	ibstream(const wstring& strFile)
		: ifstream(
#ifdef _MSC_VER
			strFile
#else
			util::WStrToStr(strFile)
#endif
            , ios_base::binary)
	{
	}

	void open(const wstring& strFile)
	{
		ifstream::open(
#ifdef _MSC_VER
			strFile
#else
			util::WStrToStr(strFile)
#endif
            , ios_base::binary);
	}
};

class obstream : public ofstream
{
public:
	obstream() {}

	obstream(const wstring& strFile, bool bTrunc)
		: ofstream(
#ifdef _MSC_VER
			strFile
#else
			util::WStrToStr(strFile)
#endif
            , bTrunc ? ios_base::binary | ios_base::trunc : ios_base::binary)
	{
	}

	void open(const wstring& strFile, bool bTrunc)
	{
		ofstream::open(
#ifdef _MSC_VER
			strFile
#else
			util::WStrToStr(strFile)
#endif
            , bTrunc ? ios_base::binary | ios_base::trunc : ios_base::binary);
	}
};

bool fsutil::saveTxt(const wstring& strFile
	, const function<void(FN_WriteTxt fnWriteTxt)>& cb, bool bTrunc, bool bToUTF8)
{
	wstring strMode(bTrunc?L"w":L"a");
	if (bToUTF8)
	{
		strMode.append(L",ccs=UTF-8");
	}
	else
	{
		strMode.append(L"b");
	}

	FILE* pFile = NULL;
	if (0 != _wfopen_s(&pFile, strFile.c_str(), strMode.c_str()) || NULL == pFile)
	{
		return false;
	}

	if (!bToUTF8)
	{
		BYTE chUnicodeHead[] = { 0xff, 0xfe }; // Unicode头
		fwrite(chUnicodeHead, sizeof(chUnicodeHead), 1, pFile);
	}

	auto fnWrite = [&](const wstring& strData) {
		if (!strData.empty())
		{
			fwrite(strData.c_str(), strData.size() * sizeof(wchar_t), 1, pFile);
		}
	};

	cb(fnWrite);

	fclose(pFile);

	return true;
}

bool fsutil::saveTxt(const wstring& strFile, const wstring& strData, bool bTrunc, bool bToUTF8)
{
	return saveTxt(strFile, [&](FN_WriteTxt cb) {
		cb(strData);
	}, bTrunc, bToUTF8);
}

bool fsutil::loadBinary(const wstring& strFile, vector<char>& vecData, UINT uReadSize)
{
	ibstream fs(strFile);
	if (!fs || !fs.is_open())
	{
		return false;
	}

	if (0 != uReadSize)
	{
		vecData.resize(uReadSize);
		fs.read(&vecData.front(), vecData.size());
		size_t size = (size_t)fs.gcount();
		if (size < uReadSize)
		{
			vecData.resize(size);
		}
	}
	else
	{
		while (!fs.eof())
		{
			char lpBuff[256] = { 0 };
			fs.read(lpBuff, sizeof(lpBuff));
			size_t size = (size_t)fs.gcount();
			if (size > 0)
			{
				size_t prevSize = vecData.size();
				vecData.reserve(size);

				memcpy(&vecData[prevSize], lpBuff, size);
			}
		}
	}

	fs.close();

	return true;
}

bool fsutil::loadTxt(const wstring& strFile, string& strData)
{
	ibstream fs(strFile);
	if (!fs || !fs.is_open())
	{
		return false;
	}

	while (!fs.eof())
	{
		char lpBuff[256] = { 0 };
		fs.read(lpBuff, sizeof(lpBuff) - 1);

		strData.append(lpBuff);
	}

	fs.close();

	return true;
}

bool fsutil::loadTxt(const wstring& strFile, const function<bool(const string&)>& cb, char cdelimiter)
{
	string strData;
	if (!loadTxt(strFile, strData))
	{
		return false;
	}

	size_t prePos = 0;
	size_t pos = strData.find(cdelimiter, prePos);
	while (string::npos != pos)
	{
		if (!cb(strData.substr(prePos, pos - prePos)))
		{
			return true;
		}
		
		prePos = pos + 1;
		pos = strData.find(cdelimiter, prePos);
	}

	if (prePos < strData.size())
	{
		cb(strData.substr(prePos));
	}

	return true;
}

bool fsutil::loadTxt(const wstring& strFile, SVector<string>& vecLineData, char cdelimiter)
{
	return loadTxt(strFile, [&](const string& strData) {
		vecLineData.add(strData);
		return true;
	}, cdelimiter);
}

int fsutil::GetFileSize(const wstring& strFilePath)
{
	struct _stat fileStat;
	if (0 == _wstat(strFilePath.c_str(), &fileStat))
	{
		return fileStat.st_size;
	}
	
	return -1;
}

bool fsutil::copyFile(const wstring& strSrcFile, const wstring& strDstFile, bool bSyncModifyTime)
{
	ibstream srcStream;
	try
	{
		srcStream.open(strSrcFile);
	}
	catch (...)
	{
	}
	__EnsureReturn(srcStream && srcStream.is_open(), false);

	obstream dstStream;
	try
	{
		dstStream.open(strDstFile, true);
	}
	catch (...)
	{
	}
	if (!dstStream || !dstStream.is_open())
	{
		srcStream.close();

		return false;
	}

	bool bResult = true;

	char lpBuffer[1024]{ 0 };
	try
	{
		while (!srcStream.eof())
		{
			srcStream.read(lpBuffer, sizeof lpBuffer);
			auto size = srcStream.gcount();
			if (size > 0)
			{
				dstStream.write(lpBuffer, size);
			}
		}
	}
	catch (...)
	{
		bResult = false;
	}

	srcStream.close();
	dstStream.close();

	if (bResult)
	{
		struct _stat fileStat;
		if (0 == _wstat(strSrcFile.c_str(), &fileStat))
		{
			struct _utimbuf timbuf { fileStat.st_atime, fileStat.st_mtime };
			(void)_wutime(strDstFile.c_str(), &timbuf);
		}
	}

	return bResult;
}

time64_t fsutil::GetFileModifyTime(const wstring& strFilePath)
{
	struct _stat fileStat;
	if (0 == _wstat(strFilePath.c_str(), &fileStat))
	{
		return fileStat.st_mtime;
	}

	return -1;
}

void fsutil::SplitPath(const wstring& strPath, wstring *pstrDir, wstring *pstrFile)
{
	int iPos = -1;
	auto pos = strPath.find_last_of(fsutil::backSlant);
	if (wstring::npos != pos)
	{
		iPos = pos;
	}

	if (iPos >= 0)
	{
		if (NULL != pstrDir)
		{
			*pstrDir = strPath.substr(0, iPos);
		}

		if (NULL != pstrFile)
		{
			*pstrFile = strPath.substr(iPos + 1);
		}
	}
	else
	{
		if (NULL != pstrFile)
		{
			*pstrFile = strPath;
		}
	}
}

wstring fsutil::GetFileName(const wstring& strPath)
{
	wstring strFileName;
	SplitPath(strPath, NULL, &strFileName);

	return strFileName;
}

void fsutil::GetFileName(const wstring& strPath, wstring *pstrTitle, wstring *pstrExtName)
{
	wstring strName = GetFileName(strPath);

	auto pos = strName.find_last_of(dot);
	if (wstring::npos != pos)
	{
		if (NULL != pstrExtName)
		{
			*pstrExtName = strName.substr(pos);
		}

		strName = strName.substr(0, pos);
	}

	if (NULL != pstrTitle)
	{
		*pstrTitle = strName;
	}
}

wstring fsutil::getFileTitle(const wstring& strPath)
{
	wstring strTitle;
	GetFileName(strPath, &strTitle, NULL);
	return strTitle;
}

wstring fsutil::GetFileExtName(const wstring& strPath)
{
	wstring strExtName;
	GetFileName(strPath, NULL, &strExtName);
	return strExtName;
}

wstring fsutil::GetParentDir(const wstring& strPath)
{
	__EnsureReturn(!strPath.empty(), L"");

	wstring strNewPath = strPath;
	if (fsutil::backSlant == strNewPath.back())
	{
		strNewPath.pop_back();
	}

	int nPos = (int)strNewPath.rfind(fsutil::backSlant);
	__EnsureReturn(0 <= nPos, L"");

	return strNewPath.substr(0, nPos);
}

bool fsutil::CheckSubPath(const wstring& strDir, const wstring& strSubPath)
{
	auto size = strDir.size();
	__EnsureReturn(size > 0, false);
	__EnsureReturn(size < strSubPath.size(), false);

	__EnsureReturn(0 == _wcsnicmp(strDir.c_str(), strSubPath.c_str(), size), false);

	__EnsureReturn(fsutil::backSlant == *strDir.rbegin() || fsutil::backSlant == strSubPath[size], false);

	return true;
}

wstring fsutil::GetOppPath(const wstring& strPath, const wstring strBaseDir)
{
	if (strBaseDir.empty())
	{
		return strPath;
	}
	
	if (!CheckSubPath(strBaseDir, strPath))
	{
		return L"";
	}

	return strPath.substr(strBaseDir.size());
}
