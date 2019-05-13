
#include "util.h"

#include <sys/utime.h>
#include <sys/stat.h>

#include <fstream>

#ifndef _MSC_VER
#include <QFileInfo>
#include <QFile>
#include <QDir>
#else
#include <Windows.h>
#endif

static const unsigned char g_chUnicodeHead[] = { 0xff, 0xfe }; // Unicode头

class ibstream : public ifstream
{
public:
	ibstream() {}

	ibstream(const wstring& strFile)
	{
		open(strFile);
	}

	void open(const wstring& strFile)
	{
		ifstream::open(
#ifdef _MSC_VER
			strFile
#else
			util::WSToAsc(strFile)
#endif
            , ios_base::binary);
	}
};

class obstream : public ofstream
{
public:
	obstream() {}

	obstream(const wstring& strFile, bool bTrunc)
	{
		open(strFile, bTrunc);
	}

	void open(const wstring& strFile, bool bTrunc)
	{
		ofstream::open(
#ifdef _MSC_VER
			strFile
#else
			util::WSToAsc(strFile)
#endif
            , bTrunc ? ios_base::binary | ios_base::trunc : ios_base::binary);
	}
};

#ifdef __ANDROID__
using FileStat = struct stat;
#else
using FileStat = struct _stat;
#endif

static bool getFileStat(const wstring& strFile, FileStat& fileStat)
{
#ifdef __ANDROID__
	return 0 == stat(util::WSToAsc(strFile).c_str(), &fileStat);
#else
	return 0 == _wstat(strFile.c_str(), &fileStat);
#endif
}

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

	FILE *lpFile = NULL;
#ifdef __ANDROID__
	lpFile = fopen(util::WSToAsc(strFile).c_str(), util::WSToAsc(strMode).c_str());
#else
	__EnsureReturn(0 == _wfopen_s(&lpFile, strFile.c_str(), strMode.c_str()), false);
#endif
	__EnsureReturn(lpFile, false);

	if (!bToUTF8)
	{
		fwrite(g_chUnicodeHead, sizeof(g_chUnicodeHead), 1, lpFile);
	}

	auto fnWrite = [&](const wstring& strData) {
		if (!strData.empty())
		{
			fwrite(strData.c_str(), strData.size() * sizeof(wchar_t), 1, lpFile);
		}
	};

	cb(fnWrite);

	fclose(lpFile);

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

int fsutil::GetFileSize(const wstring& strFile)
{
	FileStat fileStat;
	if (!getFileStat(strFile, fileStat))
	{
		return -1;
	}

	return fileStat.st_size;
}

static bool _copyFile(const wstring& strSrcFile, const wstring& strDstFile)
{
#ifdef __ANDROID__
    return QFile::copy(__QStr(strSrcFile), __QStr(strDstFile));
#else
	return TRUE == ::CopyFileW(strSrcFile.c_str(), strDstFile.c_str(), FALSE);
#endif
}

/*ibstream srcStream;
try
{
	srcStream.open(strSrcFile);
}
catch (...)
{
}
__EnsureReturn(srcStream && srcStream.is_open(), false);

if (!removeFile(strDstFile))
{
	srcStream.close();
	return false;
}

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
dstStream.close();*/

bool fsutil::copyFile(const wstring& strSrcFile, const wstring& strDstFile, bool bSyncModifyTime)
{
	if (!_copyFile(strSrcFile, strDstFile))
	{
		return false;
	}

	if (bSyncModifyTime)
	{
		FileStat fileStat;
		if (getFileStat(strSrcFile, fileStat))
		{
#ifdef __ANDROID__
            struct timeval timeVal[] = {
                       {0,0}, {0,0}
                   };
            utimes(util::WSToAsc(strDstFile).c_str(), timeVal);
#else
			struct _utimbuf timbuf { fileStat.st_atime, fileStat.st_mtime };
			(void)_wutime(strDstFile.c_str(), &timbuf);
#endif
		}
	}

	return true;
}

time64_t fsutil::GetFileModifyTime(const wstring& strFile)
{
	FileStat fileStat;
	if (!getFileStat(strFile, fileStat))
	{
		return -1;
	}

	return fileStat.st_mtime;
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

	__EnsureReturn(fsutil::backSlant == *strDir.rbegin() || fsutil::backSlant == strSubPath[size], false);

#ifdef __ANDROID__
	const auto& _strDir = util::WSToAsc(strDir);
	return 0 == strncasecmp(_strDir.c_str(), util::WSToAsc(strSubPath).c_str(), _strDir.size());
#else
	return 0 == _wcsnicmp(strDir.c_str(), strSubPath.c_str(), size);
#endif
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

bool fsutil::fileExists(const wstring& strFile)
{
	if (strFile.empty())
	{
		return false;
	}

#ifndef _MSC_VER
	QFileInfo fi(__QStr(strFile));
	return fi.isFile();
#else
	DWORD dwFileAttr = ::GetFileAttributesW(strFile.c_str());
	return INVALID_FILE_ATTRIBUTES != dwFileAttr && 0 == (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY);
#endif
}

bool fsutil::dirExists(const wstring& strDir)
{
	if (strDir.empty())
	{
		return false;
	}

#ifndef _MSC_VER
	QFileInfo fi(__QStr(strDir));
	return fi.isDir();
#else
	DWORD dwFileAttr = ::GetFileAttributesW(strDir.c_str());
	return INVALID_FILE_ATTRIBUTES != dwFileAttr && (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY);
#endif
}

void fsutil::createDir(const wstring& strDir)
{
#ifdef __ANDROID__
    (void)QDir().mkpath(__QStr(strDir));
#else
	if (::CreateDirectory(strDir.c_str(), NULL) || ERROR_ALREADY_EXISTS == ::GetLastError())
	{
		return;
	}

	createDir(fsutil::GetParentDir(strDir));

	createDir(strDir);   
#endif
}

bool fsutil::removeDir(const wstring& strDir)
{
#ifdef __ANDROID__
	QDir dir(__QStr(strDir));
	return dir.rmpath(dir.absolutePath());
#else
	if (::RemoveDirectoryW(strDir.c_str()))
	{
		return true;
	}

	auto err = ::GetLastError();
	if (ERROR_FILE_NOT_FOUND == err || ERROR_PATH_NOT_FOUND == err)
	{
		return true;
	}
	
	return false;
#endif
}

bool fsutil::removeFile(const wstring& strFile)
{
#ifdef __ANDROID__
	return QFile::remove(__QStr(strFile));
#else
	if (::DeleteFileW(strFile.c_str()))
	{
		return true;
	}

	if (ERROR_FILE_NOT_FOUND == ::GetLastError())
	{
		return true;
	}

	return false;
#endif
}

bool fsutil::moveFile(const wstring& strSrcFile, const wstring& strDstFile)
{
#ifdef __ANDROID__
    return QFile::rename(__QStr(strSrcFile), __QStr(strDstFile));
#else
	return TRUE == ::MoveFileEx(strSrcFile.c_str(), strDstFile.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);
#endif
}

wstring fsutil::currentDir()
{
#ifdef __ANDROID__
    return QDir::currentPath().toStdWString();
#else
    wchar_t pszCurrDir[MAX_PATH];
    memset(pszCurrDir, 0, sizeof pszCurrDir);
    ::GetCurrentDirectoryW(sizeof(pszCurrDir), pszCurrDir);
    return pszCurrDir;
#endif
}

#ifdef __ANDROID__
bool fsutil::findFile(const wstring& strFindPath, CB_FindFile cb)
{
    QDir dir(__QStr(strFindPath));
    if(!dir.exists())
    {
        return false;
    }
    dir.setFilter(QDir::Dirs | QDir::Files);
    dir.setSorting(QDir::DirsFirst);

    QFileInfoList list = dir.entryInfoList();
    for (int nIdx = 0; nIdx<list.size(); nIdx++)
    {
        const QFileInfo& fileInfo = list.at(nIdx);
        if(fileInfo.fileName() == "." | fileInfo.fileName() == "..")
        {
            continue;
        }

        if(!fileInfo.isDir())
        {
            // fileInfo.fileName() fileInfo.baseName() fileInfo.path() fileInfo.completeSuffix() fileInfo.suffix()
            // fileInfo.groupId() fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss") fileInfo.absoluteFilePath()
        }
    }

    return true;
}

#else

struct tagFindData : WIN32_FIND_DATAW
{
	bool isDir() const
	{
		return dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	}

	unsigned long getFileSize() const
	{
		return nFileSizeLow;
	}

	time64_t getModifyTime() const
	{
		return winfsutil::transFileTime(ftLastWriteTime);
	}

	time64_t getCreateTime() const
	{
		return winfsutil::transFileTime(ftCreationTime);
	}
};

bool fsutil::findFile(const wstring& strFindPath, CB_FindFile cb)
{
	tagFindData FindData;
	memset(&FindData, 0, sizeof(tagFindData));

	auto hFindFile = ::FindFirstFileW(strFindPath.c_str(), &FindData);
	if (INVALID_HANDLE_VALUE == hFindFile)
	{
		return false;
	}

	do
	{
		if (fsutil::dot == FindData.cFileName[0])
		{
			continue;
		}

		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
		{
			continue;
		}

		tagFileInfo FileInfo;
		FileInfo.m_bDir = FindData.isDir();
		FileInfo.m_strName = FindData.cFileName;
		FileInfo.m_uFileSize = FindData.getFileSize();
		FileInfo.m_tCreateTime = FindData.getCreateTime();
		FileInfo.m_tModifyTime = FindData.getModifyTime();
		if (!cb(FileInfo))
		{
			break;
		}
	} while (::FindNextFileW(hFindFile, &FindData));

	(void)::FindClose(hFindFile);

	return true;
}
#endif

//bool fsutil::FindFile(const wstring& strFindPath, SArray<tagFindData>& arrFindData)
//{
//	return fsutil::findFile(strFindPath, [&](const tagFindData& FindData) {
//		arrFindData.add(FindData);
//		return true;
//	});
//}
