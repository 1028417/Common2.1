
#include "util.h"

#include <sys/utime.h>
#include <sys/stat.h>

#include <fstream>

#ifdef _MSC_VER
#include <Windows.h>
#endif

//#ifdef __ANDROID__
//#include <QAndroidJniObject>
//#include <QAndroidJniEnvironment>
//#include <QtAndroid>
//#include <QtAndroidExtras>
//#endif

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

static bool getFileStat(const wstring& strFile, FileStat& fileStat)
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

void fsutil::SplitPath(const wstring& strPath, wstring *pstrDir, wstring *pstrFile)
{
	int iPos = -1;
	auto pos = strPath.find_last_of(fsutil::wcBackSlant);
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

	auto pos = strName.find_last_of(fsutil::wcDot);
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
	if (fsutil::wcBackSlant == strNewPath.back())
	{
		strNewPath.pop_back();
	}

	int nPos = (int)strNewPath.rfind(fsutil::wcBackSlant);
	__EnsureReturn(0 <= nPos, L"");

	return strNewPath.substr(0, nPos);
}

bool fsutil::CheckSubPath(const wstring& strDir, const wstring& strSubPath)
{
	auto size = strDir.size();
	__EnsureReturn(size > 0, false);
	__EnsureReturn(size < strSubPath.size(), false);

	__EnsureReturn(fsutil::wcBackSlant == *strDir.rbegin() || fsutil::wcBackSlant == strSubPath[size], false);

#ifdef __ANDROID__
	const auto& _strDir = wsutil::toStr(strDir);
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

bool fsutil::existFile(const wstring& strFile)
{
	if (strFile.empty())
	{
		return false;
	}

#ifndef _MSC_VER
	QFileInfo fi(__QStr(strFile));
    if (!fi.isFile())
    {
        return false;
    }

#else
	DWORD dwFileAttr = ::GetFileAttributesW(strFile.c_str());
    if (INVALID_FILE_ATTRIBUTES == dwFileAttr || (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY))
    {
        return false;
    }
#endif

    return true;
}

bool fsutil::existDir(const wstring& strDir)
{
	if (strDir.empty())
	{
		return false;
	}

#ifndef _MSC_VER
	QFileInfo fi(__QStr(strDir));
    if (!fi.isDir())
    {
        return false;
    }

#else
	DWORD dwFileAttr = ::GetFileAttributesW(strDir.c_str());
    if (INVALID_FILE_ATTRIBUTES == dwFileAttr || 0 == (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY))
    {
        return false;
    }
#endif

    return true;
}

bool fsutil::createDir(const wstring& strDir)
{
#ifdef __ANDROID__
    if (!QDir().mkpath(__QStr(strDir)))
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
	QDir dir(__QStr(strDir));
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
    if (!QFile::remove(__QStr(strFile)))
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
        if (!removeFile(strDstFile))
        {
            return false;
        }
    }

    if (!QFile::rename(__QStr(strSrcFile), __QStr(strDstFile)))
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

wstring fsutil::startupDir()
{
#ifdef __ANDROID__
    return QDir::currentPath().toStdWString();
#else
    wchar_t pszPath[MAX_PATH];
    memset(pszPath, 0, sizeof pszPath);
    ::GetModuleFileNameW(NULL, pszPath, sizeof(pszPath));
    return GetParentDir(pszPath);
#endif
}

static const wstring g_wsDot(1, fsutil::wcDot);
static const wstring g_wsDotDot(2, fsutil::wcDot);

#ifdef __ANDROID__
bool fsutil::findFile(const wstring& strDir, CB_FindFile cb, E_FindFindFilter eFilter, const wstring& strFilter)
{
	if (strDir.empty())
	{
		return false;
	}

    QDir dir(__QStr(strDir));
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
        if (fi.isDir())
        {
            FileInfo.m_bDir = true;
            FileInfo.m_strName = strFileName;
            FileInfo.m_uFileSize = fi.size();
            FileInfo.m_tCreateTime = fi.created().toTime_t(); //.toString("yyyy-MM-dd hh:mm:ss");
            FileInfo.m_tModifyTime = fi.lastModified().toTime_t();
            // title = fi.completeBaseName()
            // parentdir = fi.path()
            // fullPath = fi.filePath()
            // ??? fi.absoluteFilePath()
        }

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
	bool bFlag = (strDir.back() != fsutil::wcBackSlant && strDir.back() != fsutil::wcSlant);

	wstring strFind(strDir);
	if (bFlag)
	{
		strFind.append(1, fsutil::wcBackSlant);
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

size_t CTxtWriter::_write(const void *pData, size_t size) const
{
	if (NULL == m_lpFile)
	{
		return 0;
	}

	if (NULL == pData || 0 == size)
	{
		return 0;
	}

	size_t ret = fwrite(pData, size, 1, m_lpFile);

#ifdef _DEBUG
	(void)fflush(m_lpFile);
#endif

	return ret;
}

bool CTxtWriter::open(const wstring& strFile, bool bTrunc)
{
	wstring strMode(bTrunc ? L"w" : L"a");
	if (m_bUTF8)
	{
		strMode.append(L"b,ccs=UTF-8");
	}
	else
	{
		strMode.append(L"b");
	}

#ifdef __ANDROID__
	m_lpFile = fopen(wsutil::toStr(strFile).c_str(), wsutil::toStr(strMode).c_str());
#else
	__EnsureReturn(0 == _wfopen_s(&m_lpFile, strFile.c_str(), strMode.c_str()), false);
#endif
	__EnsureReturn(m_lpFile, false);

	return true;
}

size_t CTxtWriter::write(const wstring& strText) const
{
	if (m_bUTF8)
	{
		return _write(wsutil::toUTF8(strText));
	}
	else
	{
		return _write(strText);
	}
}

size_t CTxtWriter::writeln(const wstring& strText) const
{
	if (E_EOLFlag::eol_n == m_eEOLFlag)
	{
		return write(strText + L"\n");
	}
	else if (E_EOLFlag::eol_rn == m_eEOLFlag)
	{
		return write(strText + L"\r\n");
	}
	else
	{
		return write(strText + L"\r");
	}
}

size_t CTxtWriter::writeln(const string& strText) const
{
	if (E_EOLFlag::eol_n == m_eEOLFlag)
	{
		return write(strText + "\n");
	}
	else if (E_EOLFlag::eol_rn == m_eEOLFlag)
	{
		return write(strText + "\r\n");
	}
	else
	{
		return write(strText + "\r");
	}
}

bool CTxtWriter::close()
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

bool CUnicodeTxtWriter::open(const wstring& strFile, bool bTrunc)
{
	bool bExists = fsutil::existFile(strFile);

	if (!CTxtWriter::open(strFile, bTrunc))
	{
		return false;
	}

	if (!bExists || bTrunc)
	{
		(void)CTxtWriter::_write(
#ifdef __ANDROID__
			__UnicodeHead_Big
#else
			__UnicodeHead_Lit
#endif
		);
	}

	return true;
}

bool CUTF8TxtWriter::open(const wstring& strFile, bool bTrunc)
{
	bool bExists = fsutil::existFile(strFile);

	if (!CTxtWriter::open(strFile, bTrunc))
	{
		return false;
	}

	if (m_bWithBom)
	{
		if (!bExists || bTrunc)
		{
			(void)CTxtWriter::write(__UTF8Bom);
		}
	}

	return true;
}
