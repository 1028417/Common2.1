
#include "util.h"

void winfsutil::getSysDrivers(list<wstring>& lstDrivers)
{
	#define MAX_DRIVE (_MAX_DRIVE + 1)

	TCHAR pszBuffer[256] {0};

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

bool winfsutil::removeDir(cwstr strPath, HWND hwndParent, cwstr strTitle)
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

static void _shellExplore(cwstr strSelFile, cwstr strDir=L"", bool bRoot=false, HWND hWnd = NULL)
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

void winfsutil::exploreDir(cwstr strDir, bool bAsRoot)
{
	_shellExplore(L"", strDir, bAsRoot);
}
void winfsutil::exploreFile(cwstr strFile)
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
static wstring getFileType(cwstr extention)
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
HICON winfsutil::getFileIcon(cwstr extention)
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
