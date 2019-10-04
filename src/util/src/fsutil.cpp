
#include "util.h"

void fsutil::trimPathTail(wstring& strPath)
{
	if (!strPath.empty())
	{
		if (_checkPathTail(strPath.back()))
		{
			strPath.pop_back();
		}
	}
}

wstring fsutil::trimPathTail_r(const wstring& strPath)
{
	if (!strPath.empty())
	{
		if (_checkPathTail(strPath.back()))
		{
			return strPath.substr(0, strPath.size() - 1);
		}
	}

	return strPath;
}

bool fsutil::loadBinary(const wstring& strFile, vector<char>& vecData, UINT uReadSize)
{
	ibstream fs(strFile);
    if (!fs)
	{
		return false;
	}

	if (0 != uReadSize)
	{
		vecData.resize(uReadSize);
        size_t uCount = fs.read(&vecData.front(), uReadSize);
        if (uCount < uReadSize)
		{
            vecData.resize(uCount);
		}
	}
	else
	{
		while (!fs.eof())
		{
			char lpBuff[256] = { 0 };
            size_t uCount = fs.read(lpBuff, sizeof(lpBuff));
            if (uCount > 0)
			{
				size_t prevSize = vecData.size();
                vecData.resize(prevSize + uCount);

                memcpy(&vecData[prevSize], lpBuff, uCount);
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

bool fsutil::copyFileEx(const wstring& strSrcFile, const wstring& strDstFile, const CB_CopyFile& cb)
{
	ibstream srcStream(strSrcFile);
	__EnsureReturn(srcStream, false);

	obstream dstStream(strDstFile, true);
	__EnsureReturn(dstStream, false);

	char lpBuffer[1024]{ 0 };
	while (!srcStream.eof())
	{
        size_t uCount = srcStream.read(lpBuffer, sizeof lpBuffer);
        if (uCount > 0)
		{
			if (cb)
			{
                cb(lpBuffer, uCount);
			}

            if (!dstStream.write(lpBuffer, uCount))
			{
				return false;
			}
		}
	}

	return true;
}

bool fsutil::fileStat(FILE *lpFile, tagFileStat& stat)
{
#if __windows
    return 0 == _fstat(_fileno(lpFile), &stat);
#else
    return 0 == ::fstat(_fileno(lpFile), &stat);
#endif
}

bool fsutil::fileStat(const wstring& strFile, tagFileStat& stat)
{
#if __windows
    return 0 == _wstat(strFile.c_str(), &stat);
#else
    return 0 == ::stat(wsutil::toStr(strFile).c_str(), &stat);
#endif
}

bool fsutil::fileStat32(FILE *lpFile, tagFileStat32& stat)
{
#if __windows
    return 0 == _fstat32(_fileno(lpFile), &stat);
#else
    return fileStat(lpFile, stat);
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

bool fsutil::fileStat32_64(FILE *lpFile, tagFileStat32_64& stat)
{
#if __windows
    return 0 == _fstat32i64(_fileno(lpFile), &stat);
#else
    return fileStat(lpFile, stat);
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

bool fsutil::fileStat64(FILE *lpFile, tagFileStat64& stat)
{
#if __windows
    return 0 == _fstat64(_fileno(lpFile), &stat);
#else
    return fileStat(lpFile, stat);
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

bool fsutil::fileStat64_32(FILE *lpFile, tagFileStat64_32& stat)
{
#if __windows
    return 0 == _fstat64i32(_fileno(lpFile), &stat);
#else
    return fileStat(lpFile, stat);
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

void fsutil::SplitPath(const wstring& strPath, wstring *pstrDir, wstring *pstrFile)
{
	int size = strPath.size();
	for (int pos = size - 1; pos>=0; pos--)
	{
		if (_checkPathTail(strPath[pos]))
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
		if (_checkPathTail(strPath[pos]))
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

	__EnsureReturn(_checkPathTail(*strDir.rbegin()) || _checkPathTail(strSubPath[size]), false);

#if __windows
    return 0 == _wcsnicmp(strDir.c_str(), strSubPath.c_str(), size);
#else
	cauto& _strDir = wsutil::toStr(strDir);
    return 0 == strncasecmp(_strDir.c_str(), wsutil::toStr(strSubPath).c_str(), _strDir.size());
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
    QFileInfo fi(wsutil::toQStr(strPath));
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
    if (!QDir().mkpath(wsutil::toQStr(strDir)))
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
    QDir dir(wsutil::toQStr(strDir));
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
    if (!QFile::remove(wsutil::toQStr(strFile)))
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
		if (!wsutil::matchIgnoreCase(t_strSrcFile, t_strDstFile))
		{
			if (!removeFile(strDstFile))
			{
				return false;
			}
		}
    }

    if (!QFile::rename(wsutil::toQStr(strSrcFile), wsutil::toQStr(strDstFile)))
    {
        return false;
    }
#endif

    return true;
}

int64_t fsutil::seekFile(FILE *lpFile, int64_t offset, E_SeekFileFlag eFlag)
{
#if __windows
    (void)_fseeki64(lpFile, offset, (int)eFlag);
    return _ftelli64(lpFile);

#elif __android
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
    (void)fseek(lpFile, (long)offset, (int)eFlag);
    return ftell(lpFile);
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

#if __windows
wstring fsutil::getModuleDir(wchar_t *pszModuleName)
{
	wchar_t pszPath[MAX_PATH];
	memset(pszPath, 0, sizeof pszPath);
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
    for (cauto& strDriver : lstDrivers)
    {
        tagFileInfo FileInfo;
        FileInfo.bDir = true;
        FileInfo.strName = strDriver;
        cb(FileInfo);
    }
*/

bool fsutil::findFile(const wstring& strDir, CB_FindFile cb, E_FindFindFilter eFilter, const wchar_t *pstrFilter)
{
	if (strDir.empty())
	{
		return false;
	}

#if __windows
    if (strDir.empty())
    {
        return false;
    }

    wstring strFind(strDir);
    if (!_checkPathTail(strDir.back()))
    {
        strFind.append(1, __wcFSSlant);
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
        if(g_wsDot == strFileName || g_wsDotDot == strFileName)
        {
            continue;
        }

        tagFileInfo FileInfo;
        FileInfo.bDir = FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        FileInfo.strName = strFileName;
        FileInfo.uFileSize = FindData.nFileSizeLow;
        FileInfo.tCreateTime = winfsutil::transFileTime(FindData.ftCreationTime);
        FileInfo.tModifyTime = winfsutil::transFileTime(FindData.ftLastWriteTime);

        cb(FileInfo);
    } while (::FindNextFileW(hFindFile, &FindData));

    (void)::FindClose(hFindFile);

#else
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

		cauto& strFileName = fi.fileName().toStdWString();
		if (g_wsDot == strFileName || g_wsDotDot == strFileName)
		{
			continue;
		}

		if (pstrFilter)
		{
			if (E_FindFindFilter::FFP_ByPrefix == eFilter)
			{
				QString qsFilter = wsutil::toQStr(pstrFilter);
				if (0 != fi.fileName().left(qsFilter.size()).compare(qsFilter, Qt::CaseSensitivity::CaseInsensitive))
				{
					continue;
				}
			}
			else if (!fi.isDir())
			{
				if (E_FindFindFilter::FFP_ByExt == eFilter)
				{
					if (!wsutil::matchIgnoreCase(fi.suffix().toStdWString(), pstrFilter))
					{
						continue;
					}
				}
			}
        }

        tagFileInfo FileInfo;
        FileInfo.bDir = fi.isDir();
        if (!FileInfo.bDir)
        {
            FileInfo.uFileSize = fi.size();
        }

        FileInfo.strName = strFileName;

        FileInfo.tCreateTime = fi.created().toTime_t(); //.toString("yyyy-MM-dd hh:mm:ss");
        FileInfo.tModifyTime = fi.lastModified().toTime_t();

        cb(FileInfo);
    }
#endif

    return true;
}

static long _xcompressFile(const wstring& strSrcFile, const wstring& strDstFile
                     , const function<unsigned long(const vector<char>&, vector<char>&)>& cb)
{
    vector<char> vecData;
    if (!fsutil::loadBinary(strSrcFile, vecData))
    {
        return -1;
    }
    if (vecData.empty())
    {
        return 0;
    }

    vector<char> vecOutput;
    unsigned long uRet = cb(vecData, vecOutput);
    if (0 == uRet)
    {
        return 0;
    }

    obstream stream(strDstFile, true);
    __EnsureReturn(stream, false);
    if (!stream.write(&vecOutput.front(), uRet))
    {
        return -1;
    }

    return uRet;
}

#include "../../../zlib1.2.3-win32lib/include/zlib.h"

long fsutil::zcompressFile(const wstring& strSrcFile, const wstring& strDstFile)
{
    return _xcompressFile(strSrcFile, strDstFile, [&](const vector<char>&vecData, vector<char>&vecOutput){
        auto sourceLen = vecData.size();

        uLongf destLen = sourceLen;
        vecOutput.resize(destLen);

        int nRet = compress2((Bytef*)&vecOutput.front(), &destLen, (const Bytef*)&vecData.front(), sourceLen, Z_BEST_COMPRESSION);
        if (nRet != Z_OK)
        {
            return 0ul;
        }

        return destLen;
    });
}

long fsutil::zuncompressFile(const wstring& strSrcFile, const wstring& strDstFile)
{
    return _xcompressFile(strSrcFile, strDstFile, [&](const vector<char>&vecData, vector<char>&vecOutput){
        vecOutput.resize(vecData.size()*2);

        uLongf destLen = 0;
        int nRet = uncompress((Bytef*)&vecOutput.front(), &destLen, (const Bytef*)&vecData.front(), vecData.size());
        if (nRet != 0)
        {
            return 0ul;
        }

        return destLen;
    });
}

#if !__winvc
long fsutil::qcompressFile(const wstring& strSrcFile, const wstring& strDstFile, int nCompressLecvel)
{
    vector<char> vecData;
    if (!fsutil::loadBinary(strSrcFile, vecData))
    {
        return -1;
    }
    if (vecData.empty())
    {
        return 0;
    }

    cauto& baOutput = qCompress((const uchar*)&vecData.front(), vecData.size(), nCompressLecvel);

    obstream stream(strDstFile, true);
    __EnsureReturn(stream, false);
    if (!stream.write(baOutput.data(), baOutput.size()))
    {
        return -1;
    }

    return baOutput.size();
}

long fsutil::quncompressFile(const wstring& strSrcFile, const wstring& strDstFile)
{
    vector<char> vecData;
    if (!fsutil::loadBinary(strSrcFile, vecData))
    {
        return -1;
    }
    if (vecData.empty())
    {
        return 0;
    }

    cauto& baOutput = qUncompress((const uchar*)&vecData.front(), vecData.size());

    obstream stream(strDstFile, true);
    __EnsureReturn(stream, false);
    if (!stream.write(baOutput.data(), baOutput.size()))
    {
        return -1;
    }

    return baOutput.size();
}
#endif
