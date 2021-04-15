
#include "util.h"

FILE* fsutil::fopen(cwstr strFile, const string& strMode)
{
#if __windows
	return fsopen(strFile, strMode);
    /*auto t_strMode = strutil::fromAsc(strMode);

	FILE *pf = NULL;
	errno_t eno = _wfopen_s(&pf, strFile.c_str(), t_strMode.c_str());
	(void)eno;
	return pf;*/

#else
	return ::fopen(strutil::toUtf8(strFile).c_str(), strMode.c_str());
#endif
}

FILE* fsutil::fopen(const string& strFile, const string& strMode)
{
#if __windows
	return fsopen(strFile, strMode);
    /*FILE *pf = NULL;
    (void)fopen_s(&pf, strFile.c_str(), strMode.c_str());
    return pf;*/
#else
    return ::fopen(strFile.c_str(), strMode.c_str());
#endif
}

bool fsutil::copyFile(cwstr strSrcFile, cwstr strDstFile)
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

bool fsutil::copyFileEx(cwstr strSrcFile, cwstr strDstFile, const CB_CopyFile& cb, const string& strHeadData)
{
    IFStream ifs(strSrcFile);
    __EnsureReturn(ifs, false);

    OFStream ofs(strDstFile, true);
    __EnsureReturn(ofs, false);

	if (!strHeadData.empty())
	{
        if (!ofs.writex(strHeadData.c_str(), strHeadData.size()))
		{
			ofs.close();
			(void)removeFile(strDstFile);
			return false;
		}
	}

    char lpBuff[4096] {0};
    while (true)
    {
        size_t size = ifs.read(lpBuff, 1, sizeof(lpBuff));
        if (0 == size)
        {
            return true;
        }

        if (cb)
        {
            if (!cb(lpBuff, size))
            {
                break;
            }
        }

        if (!ofs.writex(lpBuff, size))
        {
			break;
        }
    }

	ofs.close();
	(void)removeFile(strDstFile);

    return false;
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

bool fsutil::lStat64(cwstr strFile, tagFileStat64& stat)
{
#if __windows
    return 0 == _wstat64(strFile.c_str(), &stat);
#elif __android
    return 0 == lstat64(strutil::toUtf8(strFile).c_str(), &stat);
#else
    return 0 == lstat(strutil::toUtf8(strFile).c_str(), &stat);
#endif
}

int64_t fsutil::GetFileSize64(cwstr strFile)
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

time64_t fsutil::GetFileModifyTime64(cwstr strFile)
{
    tagFileStat64 stat;
    memzero(stat);
    if (!lStat64(strFile, stat))
    {
        return -1;
    }

    return stat.st_mtime;
}

template <class S>
static void _SplitPath(const S& strPath, S *pstrDir, S *pstrFile)
{
    int size = strPath.size();
    for (int pos = size - 1; pos>=0; pos--)
    {
        if (fsutil::checkSeparator(strPath[pos]))
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

void fsutil::SplitPath(cwstr strPath, wstring *pstrDir, wstring *pstrFile)
{
    _SplitPath(strPath, pstrDir, pstrFile);
}

void fsutil::SplitPath(const string& strPath, string *pstrDir, string *pstrFile)
{
    _SplitPath(strPath, pstrDir, pstrFile);
}

wstring fsutil::GetParentDir(cwstr strPath)
{
    __EnsureReturn(!strPath.empty(), L"");

    wstring strParentDir;
	_SplitPath<wstring>(trimPathTail_r(strPath), &strParentDir, NULL);

    return strParentDir;
}

string fsutil::GetParentDir(const string& strPath)
{
    __EnsureReturn(!strPath.empty(), "");

    string strParentDir;
	_SplitPath<string>(trimPathTail_r(strPath), &strParentDir, NULL);

    return strParentDir;
}

wstring fsutil::GetFileName(cwstr strPath)
{
	wstring strFileName;
	_SplitPath<wstring>(strPath, NULL, &strFileName);

	return strFileName;
}

string fsutil::GetFileName(const string& strPath)
{
	string strFileName;
	_SplitPath<string>(strPath, NULL, &strFileName);

	return strFileName;
}

template <class S>
static bool _matchPath(const S& str1, const S& str2, size_t maxlen = (size_t)-1)
{
    auto ptr1 = str1.c_str();
    auto ptr2 = str2.c_str();
    auto len1 = str1.size();
    auto len2 = str2.size();

    auto AaDiff = 'A'-'a';

    for (UINT uIdx = 0; ; uIdx++)
    {
        if (uIdx == maxlen)
        {
            return true;
        }
        if (uIdx >= len1 || uIdx >= len2)
        {
            break;
        }

        auto& chr1 = ptr1[uIdx];
        auto& chr2 = ptr2[uIdx];
        if (chr1 == chr2)
        {
            continue;
        }
        if (('\\' == chr1 && '/' == chr2) || ('\\' == chr2 && '/' == chr1))
        {
            continue;
        }

        if (chr1 >= 'a' && chr1 <= 'z')
        {
            if (chr1 + AaDiff == chr2)
            {
                continue;
            }
        }
        else if (chr1 >= 'A' && chr1 <= 'Z')
        {
            if (chr1 - AaDiff == chr2)
            {
                continue;
            }
        }
        return false;
    }

    return len1 == len2;
}

bool fsutil::MatchPath(cwstr strPath1, cwstr strPath2)
{
	return _matchPath(strPath1, strPath2);
}
bool fsutil::MatchPath(const string& strPath1, const string& strPath2)
{
	return _matchPath(strPath1, strPath2);
}

bool fsutil::CheckSubPath(cwstr strDir, cwstr strSubPath)
{
	auto size = strDir.size();
	__EnsureReturn(size > 0, false);

	if (!checkSeparator(strDir.back()))
	{
		__EnsureReturn(strSubPath.size() > size + 1, false);

		if (!checkSeparator(strSubPath[size]))
		{
			return false;
		}
	}
	else
	{
		__EnsureReturn(strSubPath.size() > size, false);
	}

    return _matchPath(strDir, strSubPath, size);
}

wstring fsutil::GetOppPath(const wstring strBaseDir, cwstr strSubPath)
{
    if (strBaseDir.empty())
    {
        return strSubPath;
    }

    if (!CheckSubPath(strBaseDir, strSubPath))
    {
        return L"";
    }

    return strSubPath.substr(strBaseDir.size());
}

#if __windows
inline static DWORD _getFileAttr(cwstr strPath)
{
    return ::GetFileAttributesW(strPath.c_str());
}

inline static DWORD _getFileAttr(const string& strPath)
{
    return ::GetFileAttributesA(strPath.c_str());
}

#else
inline static QString _toQstr(cwstr str)
{
    return __WS2Q(str);
}

inline static QString _toQstr(const string& str)
{
    return __WS2Q(strutil::fromStr(str));
}
#endif

template <class S>
inline static bool _existPath(const S& strPath, bool bDir)
{
#if __windows
    DWORD dwFileAttr = _getFileAttr(strPath);
    if (INVALID_FILE_ATTRIBUTES == dwFileAttr)
    {
        return false;
    }

    return bool(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) == bDir;

#else
    QFileInfo fi(_toQstr(strPath));
    if (!fi.exists())
    {
        return false;
    }

    return fi.isDir() == bDir;
#endif
}

bool fsutil::existPath(cwstr strPath, bool bDir)
{
    return _existPath(strPath, bDir);
}

bool fsutil::existPath(const string& strPath, bool bDir)
{
    return _existPath(strPath, bDir);
}

#if __windows
inline static BOOL CreateDirectoryT(cwstr strDir)
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
	if (strDir.size() <= 3)
	{
		return false;
	}

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
    return QDir().mkpath(_toQstr(strDir));
#endif
}

bool fsutil::createDir(cwstr strDir)
{
	return _createDirT(strDir);
}

bool fsutil::createDir(const string& strDir)
{
	return _createDirT(strDir);
}

bool fsutil::removeDirTree(cwstr strDir)
{
    (void)fsutil::findSubFile(strDir, [&](tagFileInfo& fi) {
        (void)removeFile(strDir + __wcPathSeparator + fi.strName);
	});

    (void)fsutil::findSubDir(strDir, [&](tagFileInfo& fi) {
        (void)removeDirTree(strDir + __wcPathSeparator + fi.strName);
	});

	return removeDir(strDir);
}

bool fsutil::removeDir(cwstr strDir)
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

bool fsutil::removeDir(const string& strDir)
{
#if !__windows
#define _rmdir rmdir
#endif
    return 0 == ::_rmdir(strDir.c_str());
}

bool fsutil::removeFile(const string& strFile)
{
    return 0 == ::remove(strFile.c_str());
}

bool fsutil::removeFile(cwstr strFile)
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

// ::rename好像实现不了移动
bool fsutil::moveFile(cwstr strSrcFile, cwstr strDstFile, bool bReplaceExisting)
{
#if __windows
	DWORD dwFlags = MOVEFILE_COPY_ALLOWED; //如移动到一个不同的卷，则复制文件并删除原文件
	if (bReplaceExisting)
	{
		dwFlags |= MOVEFILE_REPLACE_EXISTING;
	}
    if (!::MoveFileEx(strSrcFile.c_str(), strDstFile.c_str(), dwFlags))
    {
        return false;
    }

#else
	if (strutil::matchIgnoreCase(transSeparator_r(strSrcFile), transSeparator_r(strDstFile)))
	{
		return true;
	}

    if (existFile(strDstFile))
    {
		if (!bReplaceExisting)
		{
			return false;
		}
        if (!removeFile(strDstFile))
        {
            return false;
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

bool fsutil::setWorkDir(const string& strWorkDir)
{
    if (chdir(strWorkDir.c_str()))
    {
        return false;
    }

    return true;
}

string fsutil::workDir()
{
    //return QDir::currentPath().toUTF8().toStdString();

    char pszCwd[MAX_PATH];
    memzero(pszCwd);
    if (getcwd(pszCwd, sizeof(pszCwd)) == NULL)
    {
        return "";
    }

    return pszCwd;
}

//readlink("/proc/self/exe",
#if __windows
string fsutil::getModuleDir()
{
	return getModuleDir((char*)NULL);
}

string fsutil::getModuleDir(const char *pszModuleName)
{
	char pszPath[MAX_PATH];
	memzero(pszPath);
	::GetModuleFileNameA(::GetModuleHandleA(pszModuleName), pszPath, sizeof(pszPath));
	return GetParentDir(pszPath);
}

wstring fsutil::getModuleDir(const wchar_t *pszModuleName)
{
	wchar_t pszPath[MAX_PATH];
	memzero(pszPath);
	::GetModuleFileNameW(::GetModuleHandleW(pszModuleName), pszPath, sizeof(pszPath));
	return GetParentDir(pszPath);
}

template <class T, class C>
inline static T _getModuleSubPathT(const T& strSubPath, C pszModuleName)
{
	auto strModulePath = fsutil::getModuleDir(pszModuleName);

	if (!strSubPath.empty())
	{
		if (!fsutil::checkSeparator(strSubPath.front()))
		{
            strModulePath.push_back(__cPathSeparator);
		}

		strModulePath.append(strSubPath);
	}

	return strModulePath;
}

string fsutil::getModuleSubPath(const string& strSubPath, const char *pszModuleName)
{
	return _getModuleSubPathT(strSubPath, pszModuleName);
}

wstring fsutil::getModuleSubPath(cwstr strSubPath, const wchar_t *pszModuleName)
{
	return _getModuleSubPathT(strSubPath, pszModuleName);
}
#endif

#if !__winvc
#include <QStandardPaths>
wstring fsutil::getHomeDir()
{
    auto strHomeDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).toStdWString();
#if __windows
    strutil::replaceChar(strHomeDir, L'/', L'\\');
#endif
    return strHomeDir;
}
#endif

static const wstring g_wsDot = L".";
static const wstring g_wsDotDot = L"..";

bool fsutil::findFile(cwstr strDir, CB_FindFile cb, E_FindFindFilter eFilter, const wchar_t *pstrFilter)
{
#if __windows //__winvc
    wstring strFind(strDir);
    appendPathTail(strFind);

    /*if (E_FindFindFilter::FFP_ByPrefix == eFilter && pstrFilter)
    {
        strFind.append(pstrFilter).append(L"*");
    }
    else if (E_FindFindFilter::FFP_ByExt == eFilter && pstrFilter)
    {
        strFind.append(L"*.").append(pstrFilter);
    }
    else*/
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
	tagFileInfo fileInfo;
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

        if (pstrFilter)
        {
            if (E_FindFindFilter::FFP_ByPrefix == eFilter)
            {
                //wstring strFilter = pstrFilter;
                //cauto strPrefix = strFileName.substr(0, strFilter.size());
                if (!strutil::matchIgnoreCase(strFileName.c_str(), pstrFilter, wcslen(pstrFilter)))
                {
                    continue;
                }
            }
            else if (!bDir)
            {
                if (E_FindFindFilter::FFP_ByExt == eFilter)
                {
                    if (!strutil::matchIgnoreCase(fsutil::GetFileExtName(strFileName), pstrFilter))
                    {
                        continue;
                    }
                }
            }
        }

		fileInfo.strName.swap(strFileName);
        fileInfo.bDir = bDir;
        if (bDir)
        {
			fileInfo.uFileSize = 0;
			fileInfo.tCreateTime = 0;
			fileInfo.tModifyTime = 0;
		}
		else
		{
			fileInfo.uFileSize = FindData.nFileSizeLow; // TODO
			fileInfo.tCreateTime = tmutil::transFileTime(FindData.ftCreationTime.dwLowDateTime
				, FindData.ftCreationTime.dwHighDateTime);
			fileInfo.tModifyTime = tmutil::transFileTime(FindData.ftLastWriteTime.dwLowDateTime
				, FindData.ftLastWriteTime.dwHighDateTime);
		}

        cb(fileInfo);
    } while (::FindNextFileW(hFindFile, &FindData));

    (void)::FindClose(hFindFile);

#else // windows系统盘检索不了
    QDir dir(__WS2Q(strDir));
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

	tagFileInfo fileInfo;
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
        auto strFileName = fi.fileName().toStdWString();
        /*if (g_wsDot == strFileName || g_wsDotDot == strFileName)
        {
            continue;
        }*/

        bool bDir = fi.isDir();

        if (pstrFilter)
        {
            if (E_FindFindFilter::FFP_ByPrefix == eFilter)
            {
                //wstring strFilter = pstrFilter;
                //cauto strPrefix = strFileName.substr(0, strFilter.size());
                if (!strutil::matchIgnoreCase(strFileName.c_str(), pstrFilter, wcslen(pstrFilter)))
                {
                    continue;
                }
            }
            else if (!bDir)
            {
                if (E_FindFindFilter::FFP_ByExt == eFilter)
                {
                    //if (!strutil::matchIgnoreCase(fi.suffix().toStdWString().c_str(), pstrFilter))
                    if (0 != fi.suffix().compare(QString::fromWCharArray(pstrFilter), Qt::CaseSensitivity::CaseInsensitive))
                    {
                        continue;
                    }
                }
            }
        }

        fileInfo.strName.swap(strFileName);
        fileInfo.bDir = bDir;
		if (bDir)
        {
			fileInfo.uFileSize = 0;
			fileInfo.tCreateTime = 0;
			fileInfo.tModifyTime = 0;
		}
		else
		{
			fileInfo.uFileSize = (uint64_t)fi.size();

#if (QT_VERSION >= QT_VERSION_CHECK(5,13,0))
			cauto createTime = fi.birthTime();
#else
			cauto createTime = fi.created();
#endif
			fileInfo.tCreateTime = createTime.toTime_t(); //.toString("yyyy-MM-dd hh:mm:ss");
			fileInfo.tModifyTime = fi.lastModified().toTime_t();
		}

        cb(fileInfo);
    }
#endif

    return true;
}
