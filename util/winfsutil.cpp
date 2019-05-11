
#ifndef __ANDROID__

#include "util.h"

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

time64_t winfsutil::transFileTime(const FILETIME& ft)
{
	ULARGE_INTEGER ui;
	ui.LowPart = ft.dwLowDateTime;
	ui.HighPart = ft.dwHighDateTime;

	return (ui.QuadPart - 116444736000000000) / 10000000;
}

wstring winfsutil::formatFileTime(const FILETIME& fileTime, const wstring& strFormat)
{
	return util::formatTime(strFormat, transFileTime(fileTime));

	/*SYSTEMTIME sysTime;
	SYSTEMTIME localTime;
	if (!FileTimeToSystemTime(&fileTime, &sysTime)
	|| !SystemTimeToTzSpecificLocalTime(nullptr, &sysTime, &localTime))
	{
	return L"";
	}

	tm atm;
	atm.tm_year = localTime.wYear - 1900;     // tm_year is 1900 based
	atm.tm_mon = localTime.wMonth - 1;        // tm_mon is 0 based
	atm.tm_mday = localTime.wDay;
	atm.tm_hour = localTime.wHour;
	atm.tm_min = localTime.wMinute;
	atm.tm_sec = 0;
	atm.tm_isdst = -1;

	return _FormatTime(atm, strFormat);*/
}

bool winfsutil::ExistsFile(const wstring& strFile)
{
	if (strFile.empty())
	{
		return false;
	}

	DWORD dwFileAttr = ::GetFileAttributesW(strFile.c_str());
	if (INVALID_FILE_ATTRIBUTES == dwFileAttr)
	{
		return false;
	}

	return 0 == (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY);
}

bool winfsutil::ExistsDir(const wstring& strDir)
{
	if (strDir.empty())
	{
		return false;
	}

	DWORD dwFileAttr = ::GetFileAttributesW(strDir.c_str());
	if (INVALID_FILE_ATTRIBUTES == dwFileAttr)
	{
		return false;
	}

	return (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY);
}

bool winfsutil::FindFile(const wstring& strFindPath, CB_FindFile cb)
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
		if (fsutil::dot == FindData.cFileName[0])
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

//bool winfsutil::FindFile(const wstring& strFindPath, SArray<tagFindData>& arrFindData)
//{
//	return winfsutil::FindFile(strFindPath, [&](const tagFindData& FindData) {
//		arrFindData.add(FindData);
//		return true;
//	});
//}

void winfsutil::GetSysDrivers(list<wstring>& lstDrivers)
{
	#define MAX_DRIVE (_MAX_DRIVE + 1)

	TCHAR pszBuffer[256] = {0};

	UINT uCount = ::GetLogicalDriveStringsW(0, NULL);

	(void)GetLogicalDriveStringsW(uCount, pszBuffer);

	wstring strDriver;
	int nDriveType = 0;

	for(UINT uIndex = 0; uIndex < uCount/MAX_DRIVE; ++uIndex)
	{
		strDriver = pszBuffer + uIndex*MAX_DRIVE;

		nDriveType = ::GetDriveTypeW(strDriver.c_str());
		if (DRIVE_FIXED == nDriveType || DRIVE_REMOVABLE == nDriveType)
		{
			lstDrivers.push_back(strDriver.substr(0,2));
		}
	}
}

bool winfsutil::DeletePath(const wstring& strPath, HWND hwndParent, const wstring& strTitle)
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

void winfsutil::ExploreDir(const wstring& strDir, HWND hWnd)
{
	(void)::ShellExecute(NULL, L"open", L"explorer", (L"/root," + strDir).c_str(), NULL, SW_MAXIMIZE);
}

void winfsutil::ExploreFile(const wstring& strPath, HWND hWnd)
{
	ExploreFiles(list<wstring>({ strPath }));
}

void winfsutil::ExploreFiles(const list<wstring>& lstPath, HWND hWnd)
{
	wstring strExplore;
	for (auto& strPath : lstPath)
	{
		if (ExistsFile(strPath) || ExistsDir(strPath))
		{
			if (!strExplore.empty())
			{
				strExplore.append(L",");
			}

			strExplore.append(L'\"' + strPath + L'\"');
		}
	}
	if (strExplore.empty())
	{
		return;
	}

	(void)::ShellExecute(NULL, L"open", L"explorer", (L"/select," + strExplore).c_str(), NULL, SW_MAXIMIZE);
}

bool winfsutil::CreateDir(const wstring& strDir)
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
HICON winfsutil::getFolderIcon()
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
HICON winfsutil::getFileIcon(const wstring& extention)
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

#endif
