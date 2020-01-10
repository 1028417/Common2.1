﻿
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

void fsutil::trimPathTail(string& strPath)
{
    if (!strPath.empty())
    {
        if (checkPathTail(strPath.back()))
        {
            strPath.pop_back();
        }
    }
}

FILE* fsutil::fopen(const wstring& strFile, const string& strMode)
{
#if __windows
	FILE *pf = NULL;
	wstring t_strMode(strMode.begin(), strMode.end());
	(void)_wfopen_s(&pf, strFile.c_str(), t_strMode.c_str());
	return pf;
#else
	return ::fopen(strutil::toStr(strFile).c_str(), strMode.c_str());
#endif
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

bool fsutil::copyFile(const wstring& strSrcFile, const wstring& strDstFile)
{
#if __windows
	return TRUE == ::CopyFileW(strSrcFile.c_str(), strDstFile.c_str(), FALSE);
#else
    return QFile::copy(__WS2Q(strSrcFile), __WS2Q(strDstFile));
#endif
}

#if __windows
bool fsutil::copyFile(const string& strSrcFile, const string& strDstFile)
{
    return TRUE == ::CopyFileA(strSrcFile.c_str(), strDstFile.c_str(), FALSE);
}
#endif

bool fsutil::copyFileEx(const wstring& strSrcFile, const wstring& strDstFile, const CB_CopyFile& cb, const string& strHeadData)
{
    IFStream ifs(strSrcFile);
    __EnsureReturn(ifs, false);

    OFStream ofs(strDstFile, true);
    __EnsureReturn(ofs, false);

	if (!strHeadData.empty())
	{
		__EnsureReturn(ofs.writex(strHeadData.c_str(), strHeadData.length()), false);
	}

    char lpBuff[4096] {0};
    while (true)
    {
        size_t size = ifs.read(lpBuff, 1, sizeof(lpBuff));
        if (0 == size)
        {
            break;
        }

        if (cb)
        {
            if (!cb(lpBuff, size))
            {
				ofs.close();
                (void)removeFile(strDstFile);
                return false;
            }
        }

        if (!ofs.writex(lpBuff, size))
        {
            return false;
        }
    }

    return true;
}

bool fsutil::fStat64(FILE *pf, tagFileStat64& stat)
{
#if __windows
    return 0 == _fstat64(_fileno(pf), &stat);
#elif __android
    return 0 == fstat64(_fileno(pf), &stat);
#else
    return 0 == fstat(_fileno(pf), &stat);
#endif
}

bool fsutil::lStat64(const wstring& strFile, tagFileStat64& stat)
{
#if __windows
    return 0 == _wstat64(strFile.c_str(), &stat);
#elif __android
    return 0 == lstat64(strutil::toStr(strFile).c_str(), &stat);
#else
    return 0 == lstat(strutil::toStr(strFile).c_str(), &stat);
#endif
}

long long fsutil::GetFileSize64(const wstring& strFile)
{
    tagFileStat64 stat;
    memzero(stat);
    if (!lStat64(strFile, stat))
    {
        return -1;
    }

    return stat.st_size;
}

time64_t fsutil::GetFileModifyTime64(FILE *pf)
{
    tagFileStat64 stat;
    memzero(stat);
    if (!fStat64(pf, stat))
    {
        return -1;
    }

    return stat.st_mtime;
}

time64_t fsutil::GetFileModifyTime64(const wstring& strFile)
{
    tagFileStat64 stat;
    memzero(stat);
    if (!lStat64(strFile, stat))
    {
        return -1;
    }

    return stat.st_mtime;
}

template <class T>
static void _SplitPath(const T& strPath, T *pstrDir, T *pstrFile)
{
    int size = strPath.size();
    for (int pos = size - 1; pos>=0; pos--)
    {
        if (fsutil::checkPathTail(strPath[pos]))
        {
            if (pstrDir)
            {
                *pstrDir = strPath.substr(0, pos);
            }

            if (pstrFile)
            {
                *pstrFile = strPath.substr(pos + 1);
            }

            return;
        }
    }

    if (pstrFile)
    {
        *pstrFile = strPath;
    }
}

void fsutil::SplitPath(const wstring& strPath, wstring *pstrDir, wstring *pstrFile)
{
    _SplitPath(strPath, pstrDir, pstrFile);
}

void fsutil::SplitPath(const string& strPath, string *pstrDir, string *pstrFile)
{
    _SplitPath(strPath, pstrDir, pstrFile);
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

string fsutil::GetParentDir(const string& strPath)
{
    __EnsureReturn(!strPath.empty(), "");

    string strParentDir;
    SplitPath(trimPathTail_r(strPath), &strParentDir, NULL);

    return strParentDir;
}

wstring fsutil::GetFileName(const wstring& strPath)
{
    wstring strFileName;
    SplitPath(strPath, NULL, &strFileName);

    return strFileName;
}

template <class T>
static void _GetFileName(const T& strPath, T *pstrTitle, T *pstrExtName)
{
    T strFileName;
    _SplitPath<T>(strPath, NULL, &strFileName);

    auto pos = strFileName.find_last_of(__wcDot);
    if (T::npos != pos)
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
    _GetFileName<wstring>(strPath, &strTitle, NULL);
    return strTitle;
}

string fsutil::getFileTitle(const string& strPath)
{
    string strTitle;
    _GetFileName<string>(strPath, &strTitle, NULL);
    return strTitle;
}

wstring fsutil::GetFileExtName(const wstring& strPath)
{
    wstring strExtName;
    _GetFileName<wstring>(strPath, NULL, &strExtName);
    return strExtName;
}

bool fsutil::CheckSubPath(const wstring& strDir, const wstring& strSubPath)
{
    auto size = strDir.size();
    __EnsureReturn(size > 0, false);
    __EnsureReturn(size < strSubPath.size(), false);

    __EnsureReturn(checkPathTail(*strDir.rbegin()) || checkPathTail(strSubPath[size]), false);

    return strutil::matchIgnoreCase(strDir, strSubPath, size);
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
#if __windows
    DWORD dwFileAttr = ::GetFileAttributesW(strPath.c_str());
    if (INVALID_FILE_ATTRIBUTES == dwFileAttr)
    {
        return false;
    }

    return bool(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) == bDir;

#else
    QFileInfo fi(__WS2Q(strPath));
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

#if __windows
inline static BOOL CreateDirectoryT(const wstring& strDir)
{
	return ::CreateDirectoryW(strDir.c_str(), NULL);
}

inline static BOOL CreateDirectoryT(const string& strDir)
{
	return ::CreateDirectoryA(strDir.c_str(), NULL);
}
#endif

template <class T>
static bool _createDirT(const T& strDir)
{
#if __windows
	if (!CreateDirectoryT(strDir.c_str()))
	{
		auto ret = ::GetLastError();
		if (ERROR_ALREADY_EXISTS == ret)
		{
			return true;
		}

		if (ERROR_PATH_NOT_FOUND == ret)
		{
			if (!_createDirT(fsutil::GetParentDir(strDir)))
			{
				return false;
			}

			if (!CreateDirectoryT(strDir.c_str()))
			{
				ret = ::GetLastError();
				return false;
			}

			return true;
		}

		return false;
	}

	return true;
#else
	return QDir().mkpath(strutil::toQstr(strDir));
#endif
}

bool fsutil::createDir(const wstring& strDir)
{
	return _createDirT(strDir);
}

bool fsutil::createDir(const string& strDir)
{
	return _createDirT(strDir);
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
    QDir dir(__WS2Q(strDir));
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
    if (!QFile::remove(__WS2Q(strFile)))
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

    if (!QFile::rename(__WS2Q(strSrcFile), __WS2Q(strDstFile)))
    {
        return false;
    }
#endif

    return true;
}

long fsutil::fSeekTell(FILE *pf, long offset, int origin)
{
    if (fseek(pf, offset, origin))
    {
        return -1;
    }

    return ftell(pf);
}

long long fsutil::fSeekTell64(FILE *pf, long long offset, int origin)
{
    if (fseek64(pf, offset, origin))
    {
        return -1;
    }

    return ftell64(pf);
}

#if __winvc
#include <direct.h>
#define getcwd _getcwd
#define chdir _chdir
#endif

static string _getCwd()
{
    //return QDir::currentPath().toStdWString();

    char pszCwd[MAX_PATH];
    memzero(pszCwd);
    if (getcwd(pszCwd, sizeof(pszCwd)) == NULL)
    {
        return "";
    }

    return pszCwd;
}

static string g_strWorkDir;

bool fsutil::setWorkDir(const string& strWorkDir)
{
    if (chdir(strWorkDir.c_str()))
    {
        return false;
    }

    g_strWorkDir = _getCwd();
    return true;
}

string fsutil::workDir()
{
    if (g_strWorkDir.empty())
    {
        g_strWorkDir = _getCwd();
    }

    return g_strWorkDir;
}

#if __windows
string fsutil::getModuleDir(char *pszModuleName)
{
    //readlink("/proc/self/exe",

    char pszPath[MAX_PATH];
    memzero(pszPath);
    ::GetModuleFileNameA(::GetModuleHandleA(pszModuleName), pszPath, sizeof(pszPath));
    return GetParentDir(pszPath);
}
#endif

#if !__winvc
#include <QStandardPaths>
wstring fsutil::getHomeDir()
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
        cb(tagFileInfo(true, strDriver));
    }
*/

bool fsutil::findFile(const wstring& strDir, CB_FindFile cb, E_FindFindFilter eFilter, const wchar_t *pstrFilter)
{
    if (strDir.empty())
    {
        return false;
    }

    auto t_strDir = strDir;
    if (!checkPathTail(t_strDir.back()))
    {
        t_strDir.push_back(__wcDirSeparator);
    }

#if __winvc
    wstring strFind(t_strDir);
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

		bool bDir = FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		tagFileInfo fileInfo(bDir, strFileName);
        fileInfo.uFileSize = FindData.nFileSizeLow; // TODO
        fileInfo.tCreateTime = tmutil::transFileTime(FindData.ftCreationTime.dwLowDateTime, FindData.ftCreationTime.dwHighDateTime);
        fileInfo.tModifyTime = tmutil::transFileTime(FindData.ftLastWriteTime.dwLowDateTime, FindData.ftLastWriteTime.dwHighDateTime);

        cb(fileInfo);
    } while (::FindNextFileW(hFindFile, &FindData));

    (void)::FindClose(hFindFile);

#else
    QDir dir(__WS2Q(t_strDir));
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

        tagFileInfo fileInfo(bDir, strFileName);
        if (!bDir)
        {
            fileInfo.uFileSize = (uint64_t)fi.size();
        }
		
#if (QT_VERSION >= QT_VERSION_CHECK(5,13,0))
        cauto createTime = fi.birthTime();
#else
        cauto createTime = fi.created();
#endif
        fileInfo.tCreateTime = createTime.toTime_t(); //.toString("yyyy-MM-dd hh:mm:ss");
        fileInfo.tModifyTime = fi.lastModified().toTime_t();

        cb(fileInfo);
    }
#endif

    return true;
}
