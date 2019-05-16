
#include "util.h"

#include <sys/utime.h>
#include <sys/stat.h>

#include <fstream>

#ifdef _MSC_VER
#include <Windows.h>
#endif

#ifdef __ANDROID__
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#include <QtAndroidExtras>
#endif

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
			wstrutil::toStr(strFile)
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
			wstrutil::toStr(strFile)
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
	return 0 == stat(wstrutil::toStr(strFile).c_str(), &fileStat);
#else
	return 0 == _wstat(strFile.c_str(), &fileStat);
#endif
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

bool fsutil::loadTxt(const wstring& strFile, string& strText)
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

		strText.append(lpBuff);
	}

    const auto& strHead = __UTF8Bom;
    if (strText.substr(0, strHead.size()) == strHead)
    {
        strText.erase(0, strHead.size());
    }
    else
    {
        const auto& strHead = __UnicodeHead_Lit;
        if (strText.substr(0, strHead.size()) == strHead)
        {
            strText.erase(0, strHead.size());
        }
        else
        {
            const auto& strHead = __UnicodeHead_Big;
            if (strText.substr(0, strHead.size()) == strHead)
            {
                strText.erase(0, strHead.size());
            }
        }
    }

	fs.close();

	return true;
}

bool fsutil::loadTxt(const wstring& strFile, const function<bool(const string&)>& cb, char cdelimiter)
{
	string strText;
	if (!loadTxt(strFile, strText))
	{
		return false;
	}

	size_t prePos = 0;
	size_t pos = strText.find(cdelimiter, prePos);
	while (string::npos != pos)
	{
		if (!cb(strText.substr(prePos, pos - prePos)))
		{
			return true;
		}
		
		prePos = pos + 1;
		pos = strText.find(cdelimiter, prePos);
	}

	if (prePos < strText.size())
	{
		cb(strText.substr(prePos));
	}

	return true;
}

bool fsutil::loadTxt(const wstring& strFile, SVector<string>& vecLineText, char cdelimiter)
{
	return loadTxt(strFile, [&](const string& strText) {
		vecLineText.add(strText);
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
#ifdef _MSC_VER
    return TRUE == ::CopyFileW(strSrcFile.c_str(), strDstFile.c_str(), FALSE);
#else
    return QFile::copy(__QStr(strSrcFile), __QStr(strDstFile));
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
            utimes(wstrutil::toStr(strDstFile).c_str(), timeVal);
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
	auto pos = strPath.find_last_of(fsutil::wchBackSlant);
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

	auto pos = strName.find_last_of(fsutil::wchDot);
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
	if (fsutil::wchBackSlant == strNewPath.back())
	{
		strNewPath.pop_back();
	}

	int nPos = (int)strNewPath.rfind(fsutil::wchBackSlant);
	__EnsureReturn(0 <= nPos, L"");

	return strNewPath.substr(0, nPos);
}

bool fsutil::CheckSubPath(const wstring& strDir, const wstring& strSubPath)
{
	auto size = strDir.size();
	__EnsureReturn(size > 0, false);
	__EnsureReturn(size < strSubPath.size(), false);

	__EnsureReturn(fsutil::wchBackSlant == *strDir.rbegin() || fsutil::wchBackSlant == strSubPath[size], false);

#ifdef __ANDROID__
	const auto& _strDir = wstrutil::toStr(strDir);
	return 0 == strncasecmp(_strDir.c_str(), wstrutil::toStr(strSubPath).c_str(), _strDir.size());
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

bool fsutil::createDir(const wstring& strDir)
{
#ifdef __ANDROID__
    return QDir().mkpath(__QStr(strDir));
#else
	if (::CreateDirectory(strDir.c_str(), NULL) || ERROR_ALREADY_EXISTS == ::GetLastError())
	{
        return true;
	}

    if (!createDir(fsutil::GetParentDir(strDir)))
    {
        return false;
    }

    if (::CreateDirectory(strDir.c_str(), NULL) || ERROR_ALREADY_EXISTS == ::GetLastError())
    {
        return true;
    }

    return fasle;
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

/*wstring fsutil::currentDir()
{
#ifdef __ANDROID__
    return QDir::currentPath().toStdWString();
#else
    wchar_t pszPath[MAX_PATH];
    memset(pszPath, 0, sizeof pszPath);
    ::GetCurrentDirectoryW(sizeof(pszPath), pszPath);
    return pszPath;
#endif
}*/

wstring fsutil::startupDir()
{
#ifdef __ANDROID__
    return QDir::currentPath().toStdWString();
#else
    wchar_t pszPath[MAX_PATH];
    memset(pszPath, 0, sizeof pszPath);
    ::GetModouleFileNameW(sizeof(pszPath), pszPath);
    return GetParentDir(pszCurrDir);
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
        const QFileInfo& fi = list.at(nIdx);
        if(fi.fileName() == "." || fi.fileName() == "..")
        {
            continue;
        }

        tagFileInfo FileInfo;
        if (fi.isDir())
        {
            FileInfo.m_bDir = true;
            FileInfo.m_strName = fi.fileName().toStdWString();
            FileInfo.m_uFileSize = fi.size();
            FileInfo.m_tCreateTime = fi.created().toTime_t(); //.toString("yyyy-MM-dd hh:mm:ss");
            FileInfo.m_tModifyTime = fi.lastModified().toTime_t();
            // fi.baseName() fi.path() fi.absoluteFilePath() fi.completeSuffix() fi.suffix()
        }

        if (!cb(FileInfo))
        {
            break;
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
		if (fsutil::wchDot == FindData.cFileName[0])
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
