
#include "stdafx.h"

#include <fsutil.h>

// fsutil

void fsutil::SplitPath(const wstring& strPath, wstring *pstrDir, wstring *pstrFile)
{
	int iPos = -1;
	auto pos = strPath.find_last_of('/');
	if (wstring::npos != pos)
	{
		iPos = pos;
	}
	pos = strPath.find_last_of('\\');
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

	auto pos = strName.find_last_of(L'.');
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

wstring fsutil::GetFileTitle(const wstring& strPath)
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

wstring fsutil::GetParentPath(const wstring& strPath)
{
	__AssertReturn(!strPath.empty(), L"");

	wstring strNewPath = strPath;
	if ('\\' == strNewPath.back() || '/' == strNewPath.back())
	{
		strNewPath.pop_back();
	}

	int nPos = (int)strNewPath.rfind('\\');
	__EnsureReturn(0 <= nPos, L"");

	return strNewPath.substr(0, nPos);
}

bool fsutil::CheckSubPath(const wstring& strPath, const wstring& strSubPath)
{
	auto size = strPath.size();
	__EnsureReturn(size > 0, FALSE);
	__EnsureReturn(size < strSubPath.size(), FALSE);

	__EnsureReturn(0 == _wcsnicmp(strPath.c_str(), strSubPath.c_str(), size), FALSE);

	__EnsureReturn(__BackSlant == *strPath.rbegin() || __BackSlant == strSubPath[size], FALSE);

	return TRUE;
}

wstring fsutil::GetOppPath(const wstring& strPath, const wstring strBasePath)
{
	if (strBasePath.empty())
	{
		return strPath;
	}
	
	if (!CheckSubPath(strBasePath, strPath))
	{
		return L"";
	}

	return strPath.substr(strBasePath.size());
}

void fsutil::GetSysDrivers(list<wstring>& lstDrivers)
{
	#define MAX_DRIVE (_MAX_DRIVE + 1)

	TCHAR pszBuffer[256] = {0};

	UINT nCount = ::GetLogicalDriveStrings(0, NULL);

	(void)GetLogicalDriveStrings(nCount, pszBuffer);

	wstring strDriver;
	int nDriveType = 0;

	for(UINT i=0; i<nCount/MAX_DRIVE; ++i)
	{
		strDriver = pszBuffer + i*MAX_DRIVE;

		nDriveType = ::GetDriveType(strDriver.c_str());
		if (DRIVE_FIXED == nDriveType || DRIVE_REMOVABLE == nDriveType)
		{
			lstDrivers.push_back(strDriver.substr(0,2));
		}
	}
}

bool fsutil::DeletePath(const wstring& strPath, CWnd *pwndParent, const wstring& strTitle)
{
	SHFILEOPSTRUCT FileOp;
	ZeroMemory(&FileOp, sizeof(FileOp));
		
	FileOp.fFlags = FOF_NOCONFIRMATION;

	FileOp.hwnd = pwndParent->GetSafeHwnd();

	if (!strTitle.empty())
	{
		FileOp.lpszProgressTitle = strTitle.c_str();
	}

	FileOp.pFrom = strPath.c_str();

	FileOp.wFunc = FO_DELETE;

	int nResult = SHFileOperation(&FileOp);
	if (ERROR_SUCCESS == nResult)
	{
		return TRUE;
	}

	if (ERROR_FILE_NOT_FOUND == nResult || ERROR_FILE_NOT_FOUND == ::GetLastError())
	{
		return TRUE;
	}

	return FALSE;
}

bool fsutil::CopyFile(const wstring& strSrcFile, const wstring& strSnkFile)
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
	__EnsureReturn(srcStream, FALSE);

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
		return FALSE;
	}

	bool bResult = TRUE;
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
		bResult = FALSE;
	}

	srcStream.close();
	snkStream.close();
	//fclose(pfSrc);
	//fclose(pfSnk);

	//srcfile.Close();
	//snkfile.Close();
	
	return bResult;
}

void fsutil::ExplorePath(const list<wstring>& lstPath)
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

bool fsutil::CreateDir(const wstring& strDir)
{
	if (::CreateDirectory(strDir.c_str(), NULL) || ERROR_ALREADY_EXISTS == ::GetLastError())
	{
		return TRUE;
	}
	
	if (!CreateDir(GetParentPath(strDir)))
	{
		return FALSE;
	}

	return CreateDir(strDir);
}

void fsutil::FindFile(const wstring& strPath, map<wstring, wstring>& mapFiles)
{
	CFileFind fileFind;
	bool bExists = fileFind.FindFile(strPath.c_str());
	while (bExists)
	{
		bExists = fileFind.FindNextFile();

		if (fileFind.IsDots() || fileFind.IsSystem())
		{
			continue;
		}
		
		mapFiles[(LPCTSTR)fileFind.GetFileName()] = (LPCTSTR)fileFind.GetFilePath();
	}
}

bool fsutil::ExistsFile(const wstring& strFile)
{
	WIN32_FIND_DATA ffd;
	return (INVALID_HANDLE_VALUE != FindFirstFile(strFile.c_str(), &ffd));
}

bool fsutil::ExistsPath(const wstring& strDir)
{
	return (-1 != ::GetFileAttributes(strDir.c_str()));
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
HICON fsutil::getFolderIcon()
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
HICON fsutil::getFileIcon(const wstring& extention)
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
