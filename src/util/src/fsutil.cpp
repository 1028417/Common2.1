
#include "util.h"

#include <sys/utime.h>
#include <sys/stat.h>

#include <fstream>

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
			wsutil::toStr(strFile)
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
			wsutil::toStr(strFile)
#endif
            , bTrunc ? ios_base::binary | ios_base::trunc : ios_base::binary);
	}
};

#ifdef __ANDROID__
using FileStat = struct stat;
#else
using FileStat = struct _stat;
#endif

inline static bool getFileStat(const wstring& strFile, FileStat& fileStat)
{
#ifdef __ANDROID__
	return 0 == stat(wsutil::toStr(strFile).c_str(), &fileStat);
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

    cauto& strHead = CTxtWriter::__UTF8Bom;
    if (strText.substr(0, strHead.size()) == strHead)
    {
        strText.erase(0, strHead.size());
    }
    else
    {
        cauto& strHead = CTxtWriter::__UnicodeHead_LittleEndian;
        if (strText.substr(0, strHead.size()) == strHead)
        {
            strText.erase(0, strHead.size());
        }
        else
        {
            cauto& strHead = CTxtWriter::__UnicodeHead_BigEndian;
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

inline static bool _copyFile(const wstring& strSrcFile, const wstring& strDstFile)
{
#ifdef _MSC_VER
    return TRUE == ::CopyFileW(strSrcFile.c_str(), strDstFile.c_str(), FALSE);
#else
    return QFile::copy(wsutil::toQStr(strSrcFile), wsutil::toQStr(strDstFile));
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
            utimes(wsutil::toStr(strDstFile).c_str(), timeVal);
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

inline static bool _checkPathSplitor(wchar_t wch)
{
	return __wcBackSlant == wch || __wcSlant == wch;
}

void fsutil::SplitPath(const wstring& strPath, wstring *pstrDir, wstring *pstrFile)
{
	int size = strPath.size();
	for (int pos = size - 1; pos>=0; pos--)
	{
		if (_checkPathSplitor(strPath[pos]))
		{
			if (NULL != pstrDir)
			{
				*pstrDir = strPath.substr(0, pos);
			}

			if (NULL != pstrFile)
			{
				*pstrFile = strPath.substr(pos + 1);
			}

			return;
		}
	}

	if (NULL != pstrFile)
	{
		*pstrFile = strPath;
	}
}

wstring fsutil::GetRootDir(const wstring& strPath)
{
	int size = strPath.size();
	for (int pos = 1; pos < size; pos++)
	{
		if (_checkPathSplitor(strPath[pos]))
		{
			return strPath.substr(1, pos-1);
		}
	}

	return L"";
}

wstring fsutil::GetParentDir(const wstring& strPath)
{
	__EnsureReturn(!strPath.empty(), L"");

	wstring t_strPath = strPath;
	if (_checkPathSplitor(t_strPath.back()))
	{
		t_strPath.pop_back();
	}

	wstring strParentDir;
	SplitPath(t_strPath, &strParentDir, NULL);

	return strParentDir;
}

wstring fsutil::GetFileName(const wstring& strPath)
{
	wstring strFileName;
	SplitPath(strPath, NULL, &strFileName);

	return strFileName;
}

static void _GetFileName(const wstring& strPath, wstring *pstrTitle, wstring *pstrExtName)
{
	wstring strFileName;
	fsutil::SplitPath(strPath, NULL, &strFileName);

	auto pos = strFileName.find_last_of(__wcDot);
	if (wstring::npos != pos)
	{
		if (NULL != pstrExtName)
		{
			*pstrExtName = strFileName.substr(pos);
		}

		if (NULL != pstrTitle)
		{
			*pstrTitle = strFileName.substr(0, pos);
		}
	}
	else
	{
		if (NULL != pstrTitle)
		{
			*pstrTitle = strFileName;
		}
	}
}

wstring fsutil::getFileTitle(const wstring& strPath)
{
	wstring strTitle;
	_GetFileName(strPath, &strTitle, NULL);
	return strTitle;
}

wstring fsutil::GetFileExtName(const wstring& strPath)
{
	wstring strExtName;
	_GetFileName(strPath, NULL, &strExtName);
	return strExtName;
}

bool fsutil::CheckSubPath(const wstring& strDir, const wstring& strSubPath)
{
	auto size = strDir.size();
	__EnsureReturn(size > 0, false);
	__EnsureReturn(size < strSubPath.size(), false);

	__EnsureReturn(_checkPathSplitor(*strDir.rbegin()) || _checkPathSplitor(strSubPath[size]), false);

#ifdef __ANDROID__
	cauto& _strDir = wsutil::toStr(strDir);
	return 0 == strncasecmp(_strDir.c_str(), wsutil::toStr(strSubPath).c_str(), _strDir.size());
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

bool fsutil::existPath(const wstring& strPath, bool bDir)
{
#ifndef _MSC_VER
    QFileInfo fi(wsutil::toQStr(strPath));
    if (!fi.exists())
    {
        return false;
    }

    return fi.isDir() == bDir;

#else
	DWORD dwFileAttr = ::GetFileAttributesW(strPath.c_str());
	if (INVALID_FILE_ATTRIBUTES == dwFileAttr)
	{
		return false;
	}

	return bool(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) == bDir;	
#endif
}

bool fsutil::existDir(const wstring& strDir)
{
	return existPath(strDir, true);
}

bool fsutil::existFile(const wstring& strFile)
{
	return existPath(strFile, false);
}

bool fsutil::createDir(const wstring& strDir)
{
#ifdef __ANDROID__
    if (!QDir().mkpath(wsutil::toQStr(strDir)))
    {
        return false;
    }

#else
    if (!::CreateDirectory(strDir.c_str(), NULL))
    {
        if (ERROR_ALREADY_EXISTS != ::GetLastError())
        {
            if (!createDir(fsutil::GetParentDir(strDir)))
            {
                return false;
            }

            if (!::CreateDirectory(strDir.c_str(), NULL))
            {
                return false;
            }
        }
    }
#endif

    return true;
}

bool fsutil::removeDir(const wstring& strDir)
{
#ifdef __ANDROID__
    QDir dir(wsutil::toQStr(strDir));
    if (!dir.rmpath(dir.absolutePath()))
    {
        return false;
    }

#else
    if (!::RemoveDirectoryW(strDir.c_str()))
    {
        auto err = ::GetLastError();
        if (ERROR_FILE_NOT_FOUND != err && ERROR_PATH_NOT_FOUND != err)
        {
            return false;
        }
    }
#endif

    return true;
}

bool fsutil::removeFile(const wstring& strFile)
{
#ifdef __ANDROID__
    if (!QFile::remove(wsutil::toQStr(strFile)))
    {
        return false;
    }

#else
    if (!::DeleteFileW(strFile.c_str()))
    {
        if (ERROR_FILE_NOT_FOUND != ::GetLastError())
        {
            return false;
        }
    }
#endif

    return true;
}

bool fsutil::moveFile(const wstring& strSrcFile, const wstring& strDstFile)
{
#ifdef __ANDROID__
    if (existFile(strDstFile))
    {
        // TODO if (strSrcFile ==== strDstFile)
        if (!removeFile(strDstFile))
        {
            return false;
        }
    }

    if (!QFile::rename(wsutil::toQStr(strSrcFile), wsutil::toQStr(strDstFile)))
    {
        return false;
    }

#else
    if (!::MoveFileEx(strSrcFile.c_str(), strDstFile.c_str()
        , MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED))
    {
        return false;
    }
#endif

    return true;
}

#ifdef _MSC_VER
#include <direct.h>
#define getcwd _getcwd
#define chdir _chdir

#else
#include <unistd.h>
#endif

static wstring _getCwd()
{
    //return QDir::currentPath().toStdWString();

    wstring strCwd;
    char *pCwd = getcwd(NULL, 0);
    if (NULL != pCwd)
    {
        strCwd = wsutil::fromStr(pCwd);
        free(pCwd);
    }

    return strCwd;
}

static wstring g_strWorkDir;

#ifndef __ANDROID__
wstring fsutil::getModuleDir(wchar_t *pszModuleName)
{
	wchar_t pszPath[MAX_PATH];
	memset(pszPath, 0, sizeof pszPath);
	::GetModuleFileNameW(::GetModuleHandleW(pszModuleName), pszPath, sizeof(pszPath));
	return GetParentDir(pszPath);
}
#endif

wstring fsutil::workDir()
{
    if (g_strWorkDir.empty())
    {
        g_strWorkDir = _getCwd();
    }

    return g_strWorkDir;
}

bool fsutil::setWorkDir(const wstring& strWorkDir)
{
    if (chdir(wsutil::toStr(strWorkDir).c_str()))
    {
        return false;
    }

    g_strWorkDir = _getCwd();
    return true;
}

static const wstring g_wsDot(1, __wcDot);
static const wstring g_wsDotDot(2, __wcDot);

#ifdef __ANDROID__
bool fsutil::findFile(const wstring& strDir, CB_FindFile cb, E_FindFindFilter eFilter, const wstring& strFilter)
{
	if (strDir.empty())
	{
		return false;
	}

    QDir dir(wsutil::toQStr(strDir));
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
        if (!fi.isReadable() || fi.isHidden() || fi.isSymLink())
        {
            continue;
        }

        if (E_FindFindFilter::FFP_ByExt == eFilter)
        {
            if (!wsutil::matchIgnoreCase(fi.suffix().toStdWString(), strFilter))
            {
                continue;
            }
        }

        cauto& strFileName = fi.fileName().toStdWString();
        if(strFileName == g_wsDot || strFileName == g_wsDotDot)
        {
            continue;
        }

        if (E_FindFindFilter::FFP_ByPrefix == eFilter)
        {
            if (0 != wsutil::compareIgnoreCase(strFileName, strFilter, strFilter.size()))
            {
                continue;
            }
        }

        tagFileInfo FileInfo;
        FileInfo.m_bDir = fi.isDir();
        if (!FileInfo.m_bDir)
        {
            FileInfo.m_uFileSize = fi.size();
            // title = fi.completeBaseName()
            // parentdir = fi.path()
            // fullPath = fi.filePath()
            // ??? fi.absoluteFilePath()
        }

        FileInfo.m_strName = strFileName;

        FileInfo.m_tCreateTime = fi.created().toTime_t(); //.toString("yyyy-MM-dd hh:mm:ss");
        FileInfo.m_tModifyTime = fi.lastModified().toTime_t();

		cb(FileInfo);
    }

    return true;
}

#else
bool fsutil::findFile(const wstring& strDir, CB_FindFile cb, E_FindFindFilter eFilter, const wstring& strFilter)
{
	if (strDir.empty())
	{
		return false;
	}
	
	wstring strFind(strDir);
	if (!_checkPathSplitor(strDir.back()))
	{
		strFind.append(1, __wcBackSlant);
	}

	if (E_FindFindFilter::FFP_ByPrefix == eFilter)
	{
		strFind.append(strFilter).append(L"*");
	}
	else if (E_FindFindFilter::FFP_ByExt == eFilter)
	{
		strFind.append(L"*.").append(strFilter);
	}
	else
	{
		strFind.append(L"*");
	}

	WIN32_FIND_DATAW FindData;
	memset(&FindData, 0, sizeof(FindData));
	auto hFindFile = ::FindFirstFileW(strFind.c_str(), &FindData);
	if (INVALID_HANDLE_VALUE == hFindFile)
	{
		return false;
	}

    wstring strFileName;
	do
    {
        if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
		//	|| (FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
		)
        {
            continue;
        }

        strFileName = FindData.cFileName;
        if(strFileName == g_wsDot || strFileName == g_wsDotDot)
        {
            continue;
        }

		tagFileInfo FileInfo;
		FileInfo.m_bDir = FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        FileInfo.m_strName = strFileName;
		FileInfo.m_uFileSize = FindData.nFileSizeLow;
		FileInfo.m_tCreateTime = winfsutil::transFileTime(FindData.ftCreationTime);
		FileInfo.m_tModifyTime = winfsutil::transFileTime(FindData.ftLastWriteTime);

		cb(FileInfo);
	} while (::FindNextFileW(hFindFile, &FindData));

	(void)::FindClose(hFindFile);

	return true;
}
#endif
