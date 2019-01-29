
#include <util.h>

#include <fsutil.h>

int fsutil::GetFileSize(const wstring& strFilePath)
{
	struct _stat fileStat;
	if (0 == _wstat(strFilePath.c_str(), &fileStat))
	{
		return fileStat.st_size;
	}
	
	return -1;
}

void fsutil::SplitPath(const wstring& strPath, wstring *pstrDir, wstring *pstrFile)
{
	int iPos = -1;
	auto pos = strPath.find_last_of(fsutil::slant);
	if (wstring::npos != pos)
	{
		iPos = pos;
	}
	pos = strPath.find_last_of(fsutil::backSlant);
	if (wstring::npos != pos && (int)pos > iPos)
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
	if (fsutil::backSlant == strNewPath.back() || fsutil::slant == strNewPath.back())
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

bool fsutil_win::CopyFile(const wstring& strSrcFile, const wstring& strSnkFile)
{
#define MAX_BUFFER 1024
	char lpBuffer[MAX_BUFFER];

	ifstream srcStream;
	try
	{
		srcStream.open(strSrcFile.c_str(), ios::in | ios::binary);
		//pfSrc = fopen(strSrcFile.c_str(), "r");
		//bResult = srcfile.Open(util::StrToWStr(strSrcFile), CFile::modeRead| CFile::shareDenyNone);
	}
	catch (...)
	{
	}	
	__EnsureReturn(srcStream, false);

	ofstream snkStream;
	try
	{
		snkStream.open(strSnkFile.c_str(), ios::out | ios::binary | ios::trunc);
		//bResult = snkfile.Open(util::StrToWStr(strSnkFile), CFile::modeCreate| CFile::modeWrite);
	}
	catch (...)
	{
	}
	if (!snkStream)
	{
		srcStream.close();
		//(void)fclose(pfSrc);
		//srcfile.Close();
		return false;
	}

	bool bResult = true;
	try
	{
		while (true)
		{
			//nReadedSize = fread(lpBuffer, 1, MAX_BUFFER, pfSrc)

			srcStream.read(lpBuffer, MAX_BUFFER);
			auto nReadedSize = srcStream.gcount();
			
			//fwrite(lpBuffer, nReadedSize, 1, pfSnk);
			//snkfile.Write(lpBuffer, nReadedSize);

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
	//fclose(pfSrc);
	//fclose(pfSnk);

	//srcfile.Close();
	//snkfile.Close();
	
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
