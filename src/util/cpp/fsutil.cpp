
#include "util.h"

void fsutil::trimPathTail(wstring& strPath)
{
	if (!strPath.empty())
	{
        if (checkPathTail(strPath.back()))
		{
			strPath.pop_back();
		}
	}
}

wstring fsutil::trimPathTail_r(const wstring& strPath)
{
	if (!strPath.empty())
	{
        if (checkPathTail(strPath.back()))
		{
			return strPath.substr(0, strPath.size() - 1);
		}
	}

	return strPath;
}

FILE* fsutil::fopen(const string& strFile, const string& strMode)
{
#if __windows
    FILE *pf = NULL;
    (void)fopen_s(&pf, strFile.c_str(), strMode.c_str());
    return pf;
#else
    return ::fopen(strFile.c_str(), strMode.c_str());
#endif
}

FILE* fsutil::fopen(const wstring& strFile, const string& strMode)
{
#if __windows
    FILE *pf = NULL;
    (void)_wfopen_s(&pf, strFile.c_str(), strutil::toWstr(strMode).c_str());
    return pf;
#else
    return ::fopen(strutil::toStr(strFile).c_str(), strMode.c_str());
#endif
}

template <class T>
static int _loadFile(const wstring& strFile, T& buff, UINT uReadSize=0)
{
	IBStream ibs(strFile);
	__EnsureReturn(ibs, -1);

	if (0 == uReadSize)
	{
		uReadSize = ibs.size();
	}
	else
	{
		uReadSize = MIN(uReadSize, ibs.size());
	}
	if (0 == uReadSize)
	{
		return 0;
	}

	auto ptr = buff.resizeMore(uReadSize);
	if (!ibs.readex(ptr, uReadSize))
	{
		buff.resizeLess(uReadSize);
		return -1;
	}

	return uReadSize;
}

bool fsutil::loadFile(const wstring& strFile, CByteBuff& btbBuff, UINT uReadSize)
{
	return _loadFile(strFile, btbBuff, uReadSize) >= 0;
}

bool fsutil::loadTxt(const wstring& strFile, string& strText)
{
	CCharBuff chbBuff;
	if (_loadFile(strFile, chbBuff) < 0)
	{
		return false;
	}

	strText.swap(chbBuff.str());
    cauto strHead = CTxtWriter::__UTF8Bom;
    if (strText.substr(0, strHead.size()) == strHead)
    {
        strText.erase(0, strHead.size());
    }
    else
    {
        cauto strHead = CTxtWriter::__UnicodeHead_LittleEndian;
        if (strText.substr(0, strHead.size()) == strHead)
        {
            strText.erase(0, strHead.size());
        }
        else
        {
            cauto strHead = CTxtWriter::__UnicodeHead_BigEndian;
            if (strText.substr(0, strHead.size()) == strHead)
            {
                strText.erase(0, strHead.size());
            }
        }
    }

	return true;
}

bool fsutil::loadTxt(const wstring& strFile, cfn_bool_t<const string&> cb)
{
	string strText;
	if (!loadTxt(strFile, strText))
	{
		return false;
	}

	size_t prePos = 0;
	size_t pos = 0;
	while (true)
	{
		pos = strText.find('\n', prePos);
		if (string::npos == pos)
		{
			break;
		}

		string strSub = strText.substr(prePos, pos - prePos);
		if (!strSub.empty())
		{
			if ('\r' == *strText.rbegin())
			{
				strSub.pop_back();
			}
		}

		if (!cb(strSub))
		{
			return true;
		}
		
		prePos = pos + 1;
	}

	if (prePos < strText.size())
	{
		cb(strText.substr(prePos));
	}

	return true;
}

bool fsutil::loadTxt(const wstring& strFile, SVector<string>& vecLineText)
{
	return loadTxt(strFile, [&](const string& strText) {
		vecLineText.add(strText);
		return true;
	});
}

bool fsutil::copyFile(const wstring& strSrcFile, const wstring& strDstFile)
{
#if __winvc
    return TRUE == ::CopyFileW(strSrcFile.c_str(), strDstFile.c_str(), FALSE);
#else
    return QFile::copy(strutil::toQstr(strSrcFile), strutil::toQstr(strDstFile));
#endif
}

bool fsutil::copyFileEx(const wstring& strSrcFile, const wstring& strDstFile, const CB_CopyFile& cb)
{
	IBStream ibs(strSrcFile);
	__EnsureReturn(ibs, false);

	OBStream obs(strDstFile, true);
	__EnsureReturn(obs, false);

    char lpBuff[4096] {0};
	while (true)
	{
        size_t uCount = ibs.read(lpBuff, 1, sizeof(lpBuff));
		if (0 == uCount)
		{
			break;
		}

		if (cb)
		{
			if (!cb(lpBuff, uCount))
			{
				obs.close();
				(void)removeFile(strDstFile);
                return false;
			}
		}

        if (!obs.writeex(lpBuff, uCount))
		{
			return false;
		}
	}

	return true;
}

bool fsutil::fileStat(FILE *pf, tagFileStat& stat)
{
#if __windows
    return 0 == _fstat(_fileno(pf), &stat);
#else
    return 0 == ::fstat(_fileno(pf), &stat);
#endif
}

bool fsutil::fileStat(const wstring& strFile, tagFileStat& stat)
{
#if __windows
    return 0 == _wstat(strFile.c_str(), &stat);
#else
    return 0 == ::stat(strutil::toStr(strFile).c_str(), &stat);
#endif
}

bool fsutil::fileStat32(FILE *pf, tagFileStat32& stat)
{
#if __windows
    return 0 == _fstat32(_fileno(pf), &stat);
#else
    return fileStat(pf, stat);
#endif
}

bool fsutil::fileStat32(const wstring& strFile, tagFileStat32& stat)
{
#if __windows
    return 0 == _wstat32(strFile.c_str(), &stat);
#else
    return fileStat(strFile, stat);
#endif
}

bool fsutil::fileStat32_64(FILE *pf, tagFileStat32_64& stat)
{
#if __windows
    return 0 == _fstat32i64(_fileno(pf), &stat);
#else
    return fileStat(pf, stat);
#endif
}

bool fsutil::fileStat32_64(const wstring& strFile, tagFileStat32_64& stat)
{
#if __windows
    return 0 == _wstat32i64(strFile.c_str(), &stat);
#else
    return fileStat(strFile, stat);
#endif
}

bool fsutil::fileStat64(FILE *pf, tagFileStat64& stat)
{
#if __windows
    return 0 == _fstat64(_fileno(pf), &stat);
#else
    return fileStat(pf, stat);
#endif
}

bool fsutil::fileStat64(const wstring& strFile, tagFileStat64& stat)
{
#if __windows
    return 0 == _wstat64(strFile.c_str(), &stat);
#else
    return fileStat(strFile, stat);
#endif
}

bool fsutil::fileStat64_32(FILE *pf, tagFileStat64_32& stat)
{
#if __windows
    return 0 == _fstat64i32(_fileno(pf), &stat);
#else
    return fileStat(pf, stat);
#endif
}

bool fsutil::fileStat64_32(const wstring& strFile, tagFileStat64_32& stat)
{
#if __windows
    return 0 == _wstat64i32(strFile.c_str(), &stat);
#else
    return fileStat(strFile, stat);
#endif
}

long fsutil::GetFileSize(const wstring& strFile)
{
	tagFileStat32 stat;
	memzero(stat);
	if (!fileStat32(strFile, stat))
	{
		return -1;
	}

	return stat.st_size;
}

long long fsutil::GetFileSize64(const wstring& strFile)
{
	tagFileStat32_64 stat;
	memzero(stat);
	if (!fileStat32_64(strFile, stat))
	{
		return -1;
	}

	return stat.st_size;
}

time32_t fsutil::GetFileModifyTime(FILE *pf)
{
	tagFileStat32 stat;
	memzero(stat);
    if (!fileStat32(pf, stat))
	{
		return -1;
	}

	return stat.st_mtime;
}

time32_t fsutil::GetFileModifyTime(const wstring& strFile)
{
	tagFileStat32 stat;
	memzero(stat);
	if (!fileStat32(strFile, stat))
	{
		return -1;
	}

	return stat.st_mtime;
}

time64_t fsutil::GetFileModifyTime64(FILE *pf)
{
	tagFileStat64_32 stat;
	memzero(stat);
    if (!fileStat64_32(pf, stat))
	{
		return -1;
	}

	return stat.st_mtime;
}

time64_t fsutil::GetFileModifyTime64(const wstring& strFile)
{
	tagFileStat64_32 stat;
	memzero(stat);
	if (!fileStat64_32(strFile, stat))
	{
		return -1;
	}

	return stat.st_mtime;
}

void fsutil::SplitPath(const wstring& strPath, wstring *pstrDir, wstring *pstrFile)
{
	int size = strPath.size();
	for (int pos = size - 1; pos>=0; pos--)
	{
        if (checkPathTail(strPath[pos]))
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
        if (checkPathTail(strPath[pos]))
		{
			return strPath.substr(1, pos-1);
		}
	}

	return L"";
}

wstring fsutil::GetParentDir(const wstring& strPath)
{
	__EnsureReturn(!strPath.empty(), L"");

	wstring strParentDir;
	SplitPath(trimPathTail_r(strPath), &strParentDir, NULL);

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
			*pstrExtName = strFileName.substr(pos+1);
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

    __EnsureReturn(checkPathTail(*strDir.rbegin()) || checkPathTail(strSubPath[size]), false);

#if __windows
    return 0 == _wcsnicmp(strDir.c_str(), strSubPath.c_str(), size);
#else
	cauto _strDir = strutil::toStr(strDir);
    return 0 == strncasecmp(_strDir.c_str(), strutil::toStr(strSubPath).c_str(), _strDir.size());
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
#if __winvc
    DWORD dwFileAttr = ::GetFileAttributesW(strPath.c_str());
    if (INVALID_FILE_ATTRIBUTES == dwFileAttr)
    {
        return false;
    }

    return bool(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) == bDir;

#else
    QFileInfo fi(strutil::toQstr(strPath));
    if (!fi.exists())
    {
        return false;
    }

    return fi.isDir() == bDir;
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
#if __windows
	if (!::CreateDirectory(strDir.c_str(), NULL))
	{
		auto ret = ::GetLastError();
		if (ERROR_ALREADY_EXISTS == ret)
		{
			return true;
		}

		if (ERROR_PATH_NOT_FOUND == ret)
		{
			if (!createDir(fsutil::GetParentDir(strDir)))
			{
				return false;
			}

			if (!::CreateDirectory(strDir.c_str(), NULL))
			{
				ret = ::GetLastError();
				return false;
			}

			return true;
		}
		
		return false;
	}

#else
    if (!QDir().mkpath(strutil::toQstr(strDir)))
    {
        return false;
    }
#endif

    return true;
}

bool fsutil::removeDir(const wstring& strDir)
{
#if __windows
    if (!::RemoveDirectoryW(strDir.c_str()))
    {
        auto err = ::GetLastError();
        if (ERROR_FILE_NOT_FOUND != err && ERROR_PATH_NOT_FOUND != err)
        {
            return false;
        }
    }

#else
    QDir dir(strutil::toQstr(strDir));
    if (!dir.rmpath(dir.absolutePath()))
    {
        return false;
    }
#endif

    return true;
}

bool fsutil::removeFile(const wstring& strFile)
{
#if __windows
    if (!::DeleteFileW(strFile.c_str()))
    {
        if (ERROR_FILE_NOT_FOUND != ::GetLastError())
        {
            return false;
        }
    }

#else
    if (!QFile::remove(strutil::toQstr(strFile)))
    {
        return false;
    }
#endif

    return true;
}

bool fsutil::moveFile(const wstring& strSrcFile, const wstring& strDstFile)
{
#if __windows
    if (!::MoveFileEx(strSrcFile.c_str(), strDstFile.c_str()
        , MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED))
    {
        return false;
    }

#else
    if (existFile(strDstFile))
    {
		wstring t_strSrcFile = strSrcFile;
		wstring t_strDstFile = strDstFile;
		transFSSlant(t_strSrcFile);
		transFSSlant(t_strDstFile);
		if (!strutil::matchIgnoreCase(t_strSrcFile, t_strDstFile))
		{
			if (!removeFile(strDstFile))
			{
				return false;
			}
		}
    }

    if (!QFile::rename(strutil::toQstr(strSrcFile), strutil::toQstr(strDstFile)))
    {
        return false;
    }
#endif

    return true;
}

long fsutil::lSeek(FILE *pf, long offset, int origin)
{
#if __ios || __mac
    return (long)lseek(pf, offset, origin);

#elif __winvc
    if (fseek(pf, offset, origin))
    {
        return -1;
    }
    return ftell(pf);

#else
    /*if (feof(pf))
    {
       rewind(pf);
    }
    else
    {
        setbuf(pf, NULL);
    }*/

    return lseek(_fileno(pf), offset, origin);
#endif
}

long long fsutil::lSeek64(FILE *pf, long long offset, int origin)
{
#if __ios || __mac
    return lseek(pf, offset, origin);

#elif __winvc
    if (_fseeki64(pf, offset, origin))
    {
        return -1;
    }
    return _ftelli64(pf);

#else
    /*if (feof(pf))
    {
       rewind(pf);
    }
    else
    {
        setbuf(pf, NULL);
    }*/

    return lseek64(_fileno(pf), offset, origin);
#endif
}

long fsutil::GetFileSize(FILE *pf)
{
	long pos = ftell(pf);
	long len = lSeek(pf, 0, SEEK_END);
	(void)seek(pf, pos);
	return len;
}

long long fsutil::GetFileSize64(FILE *pf)
{
	long long pos = ftell64(pf);
	long long len = lSeek64(pf, 0, SEEK_END);
	(void)seek64(pf, pos);
	return len;
}

#if __winvc
#include <direct.h>
#define getcwd _getcwd
#define chdir _chdir
#endif

static wstring _getCwd()
{
    //return QDir::currentPath().toStdWString();

    wstring strCwd;
    char *pCwd = getcwd(NULL, 0);
    if (NULL != pCwd)
    {
        strCwd = strutil::toWstr(pCwd);
        free(pCwd);
    }

    return strCwd;
}

static wstring g_strWorkDir;

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
    if (chdir(strutil::toStr(strWorkDir).c_str()))
    {
        return false;
    }

    g_strWorkDir = _getCwd();
    return true;
}

#if __windows
wstring fsutil::getModuleDir(wchar_t *pszModuleName)
{
	wchar_t pszPath[MAX_PATH];
	memzero(pszPath);
	::GetModuleFileNameW(::GetModuleHandleW(pszModuleName), pszPath, sizeof(pszPath));
	return GetParentDir(pszPath);
}
#endif

#if !__winvc
#include <QCoreApplication>
wstring fsutil::getAppDir()
{
    return QCoreApplication::applicationDirPath().toStdWString();
}
#endif

#if __mac
#include <QStandardPaths>
wstring fsutil::getMacHomeDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::HomeLocation).toStdWString();
}
#endif

static const wstring g_wsDot(1, __wcDot);
static const wstring g_wsDotDot(2, __wcDot);

/*  std::list<std::wstring> lstDrivers;
    winfsutil::getSysDrivers(lstDrivers);
    for (cauto strDriver : lstDrivers)
    {
        tagFileInfo fileInfo;
        fileInfo.bDir = true;
        fileInfo.strName = strDriver;
        cb(fileInfo);
    }
*/

bool fsutil::findFile(const wstring& strDir, CB_FindFile cb, E_FindFindFilter eFilter, const wchar_t *pstrFilter)
{
    if (strDir.empty())
	{
		return false;
    }

#if __winvc
    wstring strFind(strDir);
    if (!checkPathTail(strDir.back()))
    {
        strFind.append(1, __wcDirSeparator);
    }

    if (E_FindFindFilter::FFP_ByPrefix == eFilter && pstrFilter)
    {
        strFind.append(pstrFilter).append(L"*");
    }
    else if (E_FindFindFilter::FFP_ByExt == eFilter && pstrFilter)
    {
        strFind.append(L"*.").append(pstrFilter);
    }
    else
    {
        strFind.append(L"*");
    }

    WIN32_FIND_DATAW FindData;
	memzero(FindData);
    auto hFindFile = ::FindFirstFileW(strFind.c_str(), &FindData);
    if (INVALID_HANDLE_VALUE == hFindFile)
    {
        return false;
    }

    wstring strFileName;
    do
    {
        if ((FindData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
                || (FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
        {
            continue;
        }

        strFileName = FindData.cFileName;
        if(g_wsDot == strFileName || g_wsDotDot == strFileName)
        {
            continue;
        }

        tagFileInfo fileInfo;
        fileInfo.bDir = FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        fileInfo.strName = strFileName;
        fileInfo.uFileSize = FindData.nFileSizeLow;
        fileInfo.tCreateTime = tmutil::transFileTime(FindData.ftCreationTime.dwLowDateTime, FindData.ftCreationTime.dwHighDateTime);
        fileInfo.tModifyTime = tmutil::transFileTime(FindData.ftLastWriteTime.dwLowDateTime, FindData.ftLastWriteTime.dwHighDateTime);

        cb(fileInfo);
    } while (::FindNextFileW(hFindFile, &FindData));

    (void)::FindClose(hFindFile);

#else
    QDir dir(strutil::toQstr(strDir));
    if(!dir.exists())
    {
        return false;
    }

    //cauto filter = QDir::Filter::NoFilter;
    cauto filter = QDir::Filter::Dirs | QDir::Filter::Files
            | QDir::Filter::NoSymLinks
            | QDir::Filter::Readable //| QDir::Filter::Hidden
            | QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot;

    cauto sortFlag = QDir::SortFlag::NoSort;
    /*cauto sortFlag = QDir::SortFlag::DirsFirst | QDir::SortFlag::IgnoreCase
                   | QDir::SortFlag::LocaleAware;*/

    QFileInfoList list = dir.entryInfoList(filter, sortFlag);
    for (int nIdx = 0; nIdx<list.size(); nIdx++)
    {
        const QFileInfo& fi = list.at(nIdx);
        /*if (fi.isSymLink())
        {
            continue;
        }
        if (!fi.isReadable())
        {
            continue;
        }
        if(fi.isHidden())
        {
            continue;
        }*/
        cauto strFileName = fi.fileName().toStdWString();
        /*if (g_wsDot == strFileName || g_wsDotDot == strFileName)
		{
			continue;
        }*/

        bool bDir = fi.isDir();

		if (pstrFilter)
		{
			if (E_FindFindFilter::FFP_ByPrefix == eFilter)
			{
                wstring strFilter = pstrFilter;
                if (!strutil::matchIgnoreCase(strFileName.substr(0, strFilter.size()), strFilter))
				{
					continue;
				}
			}
            else if (!bDir)
			{
				if (E_FindFindFilter::FFP_ByExt == eFilter)
				{
					if (!strutil::matchIgnoreCase(fi.suffix().toStdWString(), pstrFilter))
					{
						continue;
					}
				}
			}
        }

        tagFileInfo fileInfo;
        fileInfo.bDir = bDir;
        if (!bDir)
        {
            fileInfo.uFileSize = (UINT)fi.size();
        }

        fileInfo.strName = strFileName;
        fileInfo.tCreateTime = fi.created().toTime_t(); //.toString("yyyy-MM-dd hh:mm:ss");
        fileInfo.tModifyTime = fi.lastModified().toTime_t();

        cb(fileInfo);
    }
#endif

    return true;
}
