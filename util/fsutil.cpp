
#include <util.h>

#include <fsutil.h>

#include <sys/utime.h>

bool fsutil::saveFile(const wstring& strFile
	, const function<void(const function<void(const wstring&)>& fnWrite)>& cb, bool bTrunc, bool bToUTF8)
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

bool fsutil::saveFile(const wstring& strFile, const wstring& strData, bool bTrunc, bool bToUTF8)
{
	return saveFile(strFile, [&](auto& cb) {
		cb(strData);
	}, bTrunc, bToUTF8);
}

bool fsutil::loadFile(const string& strFile, string& strData)
{
	std::ifstream fs;
	fs.open(strFile);
	if (!fs.is_open())
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

bool fsutil::loadFile(const string& strFile, const function<bool(const string&)>& cb, char cdelimiter)
{
	string strData;
	if (!loadFile(strFile, strData))
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

bool fsutil::loadFile(const string& strFile, SVector<string>& vecLineData, char cdelimiter)
{
	return loadFile(strFile, [&](const string& strData) {
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

__time64_t fsutil::GetFileModifyTime(const wstring& strFilePath)
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

bool fsutil_win::ExistsFile(const wstring& strFile)
{
	WIN32_FIND_DATA ffd;
	return (INVALID_HANDLE_VALUE != FindFirstFileW(strFile.c_str(), &ffd));
}

bool fsutil_win::ExistsPath(const wstring& strDir)
{
	return (-1 != ::GetFileAttributesW(strDir.c_str()));
}

bool fsutil_win::FindFile(const wstring& strFindPath, const function<bool(const tagFindData&)>& cb)
{
	tagFindData FindData;
	WIN32_FIND_DATAW& FindFileData = FindData.data;
	auto hFindFile = ::FindFirstFileW(strFindPath.c_str(), &FindFileData);
	if (INVALID_HANDLE_VALUE == hFindFile)
	{
		return false;
	}

	do
	{
		if (fsutil::dot == FindFileData.cFileName[0])
		{
			continue;
		}

		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
		{
			continue;
		}

		if (!cb(FindData))
		{
			break;
		}
	} while (::FindNextFileW(hFindFile, &FindFileData));

	return true;
}

bool fsutil_win::FindFile(const wstring& strFindPath, SArray<tagFindData>& arrFindData)
{
	return fsutil_win::FindFile(strFindPath, [&](auto& FindData) {
		arrFindData.add(FindData);
		return true;
	});
}

void fsutil_win::GetSysDrivers(list<wstring>& lstDrivers)
{
	#define MAX_DRIVE (_MAX_DRIVE + 1)

	TCHAR pszBuffer[256] = {0};

	UINT uCount = ::GetLogicalDriveStrings(0, NULL);

	(void)GetLogicalDriveStrings(uCount, pszBuffer);

	wstring strDriver;
	int nDriveType = 0;

	for(UINT uIndex = 0; uIndex < uCount/MAX_DRIVE; ++uIndex)
	{
		strDriver = pszBuffer + uIndex*MAX_DRIVE;

		nDriveType = ::GetDriveType(strDriver.c_str());
		if (DRIVE_FIXED == nDriveType || DRIVE_REMOVABLE == nDriveType)
		{
			lstDrivers.push_back(strDriver.substr(0,2));
		}
	}
}

bool fsutil_win::DeletePath(const wstring& strPath, HWND hwndParent, const wstring& strTitle)
{
	SHFILEOPSTRUCT FileOp;
	ZeroMemory(&FileOp, sizeof(FileOp));
		
	FileOp.fFlags = FOF_NOCONFIRMATION;

	FileOp.hwnd = hwndParent;

	if (!strTitle.empty())
	{
		FileOp.lpszProgressTitle = strTitle.c_str();
	}

	FileOp.pFrom = strPath.c_str();

	FileOp.wFunc = FO_DELETE;

	int nResult = SHFileOperation(&FileOp);
	if (ERROR_SUCCESS == nResult)
	{
		return true;
	}

	if (ERROR_FILE_NOT_FOUND == nResult || ERROR_FILE_NOT_FOUND == ::GetLastError())
	{
		return true;
	}

	return false;
}

bool fsutil_win::copyFile(const wstring& strSrcFile, const wstring& strSnkFile, bool bSyncModifyTime)
{
#define MAX_BUFFER 1024
	char lpBuffer[MAX_BUFFER];

	ifstream srcStream;
	try
	{
		srcStream.open(strSrcFile.c_str(), ios::binary);
	}
	catch (...)
	{
	}	
	__EnsureReturn(srcStream, false);

	ofstream snkStream;
	try
	{
		snkStream.open(strSnkFile.c_str(), ios::binary | ios::trunc);
	}
	catch (...)
	{
	}
	if (!snkStream)
	{
		srcStream.close();

		return false;
	}

	bool bResult = true;
	try
	{
		while (true)
		{
			srcStream.read(lpBuffer, MAX_BUFFER);
			auto nReadedSize = srcStream.gcount();
			
			if (0 < nReadedSize)
			{
				snkStream.write(lpBuffer, nReadedSize);
			}
			if (nReadedSize < MAX_BUFFER)
			{
				break;
			}
		}
	}
	catch (...)
	{
		bResult = false;
	}

	srcStream.close();
	snkStream.close();

	if (bResult)
	{
		struct _stat fileStat;
		if (0 == _wstat(strSrcFile.c_str(), &fileStat))
		{
			struct _utimbuf timbuf { fileStat.st_atime, fileStat.st_mtime };
			(void)_wutime(strSnkFile.c_str(), &timbuf);
		}
	}

	return bResult;
}

void fsutil_win::ExplorePath(const wstring& strPath)
{
	ExplorePath(list<wstring>({ strPath }));
}

void fsutil_win::ExplorePath(const list<wstring>& lstPath)
{
	wstring strExplore;
	for (auto& strPath : lstPath)
	{
		if (!ExistsPath(strPath))
		{
			continue;
		}

		if (!strExplore.empty())
		{
			strExplore.append(L",");
		}

		strExplore.append(L'\"' + strPath + L'\"');
	}
	if (strExplore.empty())
	{
		return;
	}

	(void)::ShellExecute(NULL, L"open", L"explorer.exe", (L"/select," + strExplore).c_str(), NULL, SW_MAXIMIZE);
}

bool fsutil_win::CreateDir(const wstring& strDir)
{
	if (::CreateDirectory(strDir.c_str(), NULL) || ERROR_ALREADY_EXISTS == ::GetLastError())
	{
		return true;
	}
	
	if (!CreateDir(fsutil::GetParentDir(strDir)))
	{
		return false;
	}

	return CreateDir(strDir);
}

// 获取文件夹类型
static wstring getFolderType()
{
	SHFILEINFO info;
	if (SHGetFileInfo(L"folder",
		FILE_ATTRIBUTE_DIRECTORY,
		&info,
		sizeof(info),
		SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES))
	{
		return info.szTypeName;
	}

	return L"";
}

// 获取文件类型
static wstring getFileType(const wstring& extention)
{
	if (!extention.empty())
	{
		SHFILEINFO info;
		if (SHGetFileInfo(extention.c_str(),
			FILE_ATTRIBUTE_NORMAL,
			&info,
			sizeof(info),
			SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES))
		{
			return info.szTypeName;
		}
	}

	return L"";
}

// 获取文件夹图标
HICON fsutil_win::getFolderIcon()
{
	SHFILEINFO info;
	if (SHGetFileInfo(L"folder",
		FILE_ATTRIBUTE_DIRECTORY,
		&info,
		sizeof(info),
		SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))
	{
		return info.hIcon;
	}

	return NULL;
}

// 获取文件图标
HICON fsutil_win::getFileIcon(const wstring& extention)
{
	if (!extention.empty())
	{
		SHFILEINFO info;
		if (SHGetFileInfo(extention.c_str(),
			FILE_ATTRIBUTE_NORMAL,
			&info,
			sizeof(info),
			SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_USEFILEATTRIBUTES))
		{
			return info.hIcon;
		}
	}

	return NULL;
}
