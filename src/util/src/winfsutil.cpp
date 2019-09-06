
#include "util.h"

#if __windows

time64_t winfsutil::transFileTime(const FILETIME& ft)
{
	ULARGE_INTEGER ui;
	ui.LowPart = ft.dwLowDateTime;
	ui.HighPart = ft.dwHighDateTime;

	return (ui.QuadPart - 116444736000000000) / 10000000;
}

wstring winfsutil::formatFileTime(const FILETIME& fileTime, const wstring& strFormat)
{
	return tmutil::formatTime64(strFormat, transFileTime(fileTime));

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

void winfsutil::getSysDrivers(list<wstring>& lstDrivers)
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

bool winfsutil::removeDir(const wstring& strPath, HWND hwndParent, const wstring& strTitle)
{
	SHFILEOPSTRUCT FileOp;
	ZeroMemory(&FileOp, sizeof(FileOp));
	FileOp.hwnd = hwndParent;

	if (!strTitle.empty())
	{
		FileOp.lpszProgressTitle = strTitle.c_str();
	}

	FileOp.pFrom = strPath.c_str();
	FileOp.wFunc = FO_DELETE;
	FileOp.fFlags = FOF_NOCONFIRMATION;

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

static void _shellExplore(const wstring& strSelFile, const wstring& strDir=L"", bool bRoot=false, HWND hWnd = NULL)
{
	wstring strPara = L"/e";

	if (!strDir.empty())
	{
		if (bRoot)
		{
			strPara.append(L",/root");
		}
		strPara.append(L", \"" + strDir + L"\"");
	}
	else if (!strSelFile.empty())
	{
		strPara.append(L",/select, \"" + strSelFile + L"\"");
	}

    (void)::ShellExecuteW(hWnd, L"open", L"explorer",  strPara.c_str(), NULL, SW_MAXIMIZE);
}

void winfsutil::exploreDir(const wstring& strDir, bool bAsRoot)
{
	_shellExplore(L"", strDir, bAsRoot);
}
void winfsutil::exploreFile(const wstring& strFile)
{
	_shellExplore(strFile);
}

// 获取文件夹类型
/*static wstring getFolderType()
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
}*/

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
