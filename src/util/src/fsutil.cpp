
#include "util.h"

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

bool fsutil::loadTxt(const wstring& strFile, const function<bool(const string&)>& cb)
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
    return QFile::copy(wsutil::toQStr(strSrcFile), wsutil::toQStr(strDstFile));
#endif
}

static bool _copyFileEx(const wstring& strSrcFile, const wstring& strDstFile, const char *lpFileHead = NULL, size_t uHeadSize = 0)
{
	ibstream srcStream(strSrcFile);
	__EnsureReturn(srcStream, false);

	obstream dstStream(strDstFile, true);
	__EnsureReturn(dstStream, false);

	if (NULL != lpFileHead && uHeadSize != 0)
	{
		dstStream.write(lpFileHead, uHeadSize);
	}

	char lpBuffer[1024]{ 0 };
    while (!srcStream.eof())
    {
        srcStream.read(lpBuffer, sizeof lpBuffer);
        auto size = srcStream.gcount();
        if (size > 0)
        {
            dstStream.write(lpBuffer, size);
            if (!dstStream.good())
            {
                return false;
            }
        }
    }

	return true;
}

bool fsutil::copyFileEx(const wstring& strSrcFile, const wstring& strDstFile
	, bool bSyncModifyTime, const char *lpFileHead, size_t uHeadSize)
{
    if (!_copyFileEx(strSrcFile, strDstFile, lpFileHead, uHeadSize))
	{
		return false;
	}

	if (bSyncModifyTime)
	{
		tagFileStat stat;
		memset(&stat, 0, sizeof stat);
		if (fileStat(strSrcFile, stat))
		{
#if __android
            struct timeval timeVal[] = {
                       {0,0}, {0,0}
                   };
            utimes(wsutil::toStr(strDstFile).c_str(), timeVal);
#else
            struct _utimbuf timbuf { stat.st_atime, stat.st_mtime };
            (void)_wutime(strDstFile.c_str(), &timbuf);
#endif
		}
	}

	return true;
}

bool fsutil::fileStat(FILE *lpFile, tagFileStat& stat)
{
#if __android
    return 0 == ::fstat(_fileno(lpFile), &stat);
#else
    return 0 == _fstat(_fileno(lpFile), &stat);
#endif
}

bool fsutil::fileStat(const wstring& strFile, tagFileStat& stat)
{
#if __android
    return 0 == ::stat(wsutil::toStr(strFile).c_str(), &stat);
#else
	return 0 == _wstat(strFile.c_str(), &stat);
#endif
}

bool fsutil::fileStat32(FILE *lpFile, tagFileStat32& stat)
{
#if __android
    return fileStat(lpFile, stat);
#else
	return 0 == _fstat32(_fileno(lpFile), &stat);
#endif
}

bool fsutil::fileStat32(const wstring& strFile, tagFileStat32& stat)
{
#if __android
    return fileStat(strFile, stat);
#else
	return 0 == _wstat32(strFile.c_str(), &stat);
#endif
}

bool fsutil::fileStat32_64(FILE *lpFile, tagFileStat32_64& stat)
{
#if __android
    return fileStat(lpFile, stat);
#else
	return 0 == _fstat32i64(_fileno(lpFile), &stat);
#endif
}

bool fsutil::fileStat32_64(const wstring& strFile, tagFileStat32_64& stat)
{
#if __android
    return fileStat(strFile, stat);
#else
	return 0 == _wstat32i64(strFile.c_str(), &stat);
#endif
}

bool fsutil::fileStat64(FILE *lpFile, tagFileStat64& stat)
{
#if __android
    return fileStat(lpFile, stat);
#else
	return 0 == _fstat64(_fileno(lpFile), &stat);
#endif
}

bool fsutil::fileStat64(const wstring& strFile, tagFileStat64& stat)
{
#if __android
    return fileStat(strFile, stat);
#else
	return 0 == _wstat64(strFile.c_str(), &stat);
#endif
}

bool fsutil::fileStat64_32(FILE *lpFile, tagFileStat64_32& stat)
{
#if __android
    return fileStat(lpFile, stat);
#else
	return 0 == _fstat64i32(_fileno(lpFile), &stat);
#endif
}

bool fsutil::fileStat64_32(const wstring& strFile, tagFileStat64_32& stat)
{
#if __android
    return fileStat(strFile, stat);
#else
	return 0 == _wstat64i32(strFile.c_str(), &stat);
#endif
}

int fsutil::GetFileSize(FILE *lpFile)
{
	tagFileStat32 stat;
	memset(&stat, 0, sizeof stat);
	if (!fileStat32(lpFile, stat))
	{
		return -1;
	}

	return stat.st_size;
}

int fsutil::GetFileSize(const wstring& strFile)
{
	tagFileStat32 stat;
	memset(&stat, 0, sizeof stat);
	if (!fileStat32(strFile, stat))
	{
		return -1;
	}

	return stat.st_size;
}

int64_t fsutil::GetFileSize64(FILE *lpFile)
{
	tagFileStat32_64 stat;
	memset(&stat, 0, sizeof stat);
	if (!fileStat32_64(lpFile, stat))
	{
		return -1;
	}

	return stat.st_size;
}

int64_t fsutil::GetFileSize64(const wstring& strFile)
{
	tagFileStat32_64 stat;
	memset(&stat, 0, sizeof stat);
	if (!fileStat32_64(strFile, stat))
	{
		return -1;
	}

	return stat.st_size;
}

time32_t fsutil::GetFileModifyTime(FILE *lpFile)
{
	tagFileStat32 stat;
	memset(&stat, 0, sizeof stat);
	if (!fileStat32(lpFile, stat))
	{
		return -1;
	}

	return stat.st_mtime;
}

time32_t fsutil::GetFileModifyTime(const wstring& strFile)
{
	tagFileStat32 stat;
	memset(&stat, 0, sizeof stat);
	if (!fileStat32(strFile, stat))
	{
		return -1;
	}

	return stat.st_mtime;
}

time64_t fsutil::GetFileModifyTime64(FILE *lpFile)
{
	tagFileStat64_32 stat;
	memset(&stat, 0, sizeof stat);
	if (!fileStat64_32(lpFile, stat))
	{
		return -1;
	}

	return stat.st_mtime;
}

time64_t fsutil::GetFileModifyTime64(const wstring& strFile)
{
	tagFileStat64_32 stat;
	memset(&stat, 0, sizeof stat);
	if (!fileStat64_32(strFile, stat))
	{
		return -1;
	}

	return stat.st_mtime;
}

inline static bool _checkFSSlant(wchar_t wch)
{
	return fsutil::wcBackSlant == wch || fsutil::wcSlant == wch;
}

void fsutil::SplitPath(const wstring& strPath, wstring *pstrDir, wstring *pstrFile)
{
	int size = strPath.size();
	for (int pos = size - 1; pos>=0; pos--)
	{
		if (_checkFSSlant(strPath[pos]))
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
		if (_checkFSSlant(strPath[pos]))
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
	if (_checkFSSlant(t_strPath.back()))
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

	__EnsureReturn(_checkFSSlant(*strDir.rbegin()) || _checkFSSlant(strSubPath[size]), false);

#if __android
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
#if !__winvc
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
#if __android
	if (!QDir().mkpath(wsutil::toQStr(strDir)))
	{
		return false;
	}

#else
	if (!::CreateDirectory(strDir.c_str(), NULL))
	{
		auto ret = ::GetLastError();
		if (ERROR_PATH_NOT_FOUND == ret)
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
		else
		{
			return false;
		}
	}
#endif

    return true;
}

bool fsutil::removeDir(const wstring& strDir)
{
#if __android
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
#if __android
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
#if __android
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

#if __android
#include <unistd.h>
#endif
int64_t fsutil::seekFile(FILE *lpFile, int64_t offset, E_SeekFileFlag eFlag)
{
#if __android
    //(void)fseek(lpFile, (long)offset, (int)eFlag);
    //return ftell(lpFile);

    if (feof(lpFile))
    {
       rewind(lpFile);
    }
    else
    {
        setbuf(lpFile, NULL);
    }

    auto fno = _fileno(lpFile);
    auto nRet = lseek64(fno, offset, (int)eFlag);
    if (nRet >= 0)
    {
        return nRet;
    }
    return lseek64(fno, 0, SEEK_CUR);
#else
    (void)_fseeki64(lpFile, offset, (int)eFlag);
    return _ftelli64(lpFile);
#endif
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
        strCwd = wsutil::fromStr(pCwd);
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
    if (chdir(wsutil::toStr(strWorkDir).c_str()))
    {
        return false;
    }

    g_strWorkDir = _getCwd();
    return true;
}

#if !__android
wstring fsutil::getModuleDir(wchar_t *pszModuleName)
{
	wchar_t pszPath[MAX_PATH];
	memset(pszPath, 0, sizeof pszPath);
	::GetModuleFileNameW(::GetModuleHandleW(pszModuleName), pszPath, sizeof(pszPath));
	return GetParentDir(pszPath);
}
#endif

static const wstring g_wsDot(1, __wcDot);
static const wstring g_wsDotDot(2, __wcDot);

bool fsutil::_findFile(const wstring& strDir, CB_FindFile cb, E_FindFindFilter eFilter, const wstring& strFilter)
{
#if __android
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

    QString qsFilter = wsutil::toQStr(strFilter);

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
            if (0 != fi.fileName().left(qsFilter.size()).compare(qsFilter, Qt::CaseSensitivity::CaseInsensitive))
            {
                continue;
            }
        }

        tagFileInfo FileInfo;
        FileInfo.m_bDir = fi.isDir();
        if (!FileInfo.m_bDir)
        {
            FileInfo.m_uFileSize = fi.size();
        }

        FileInfo.m_strName = strFileName;

        FileInfo.m_tCreateTime = fi.created().toTime_t(); //.toString("yyyy-MM-dd hh:mm:ss");
        FileInfo.m_tModifyTime = fi.lastModified().toTime_t();

        cb(FileInfo);
    }

#else
	if (strDir.empty())
	{
        return false;
	}
	
	wstring strFind(strDir);
	if (!_checkFSSlant(strDir.back()))
	{
		strFind.append(1, __wcFSSlant);
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

        if (!cb(FileInfo))
        {
            break;
        }
	} while (::FindNextFileW(hFindFile, &FindData));

	(void)::FindClose(hFindFile);    
#endif

    return true;
}
