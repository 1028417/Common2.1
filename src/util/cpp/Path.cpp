
#include "util.h"

wstring XFile::GetName() const
{
    if (fileinfo.pParent)
	{
        return fileinfo.strName;
	}
	else
	{
        return fsutil::GetFileName(fileinfo.strName);
	}
}

wstring XFile::absPath() const
{
    if (fileinfo.pParent)
	{
        WString strAbsPath = fileinfo.pParent->absPath();
        if (!strAbsPath.empty())
        {
            strAbsPath << __wcFSSlant;
        }
        strAbsPath << fileinfo.strName;
        return std::move(strAbsPath);
	}

    return fileinfo.strName;
}

wstring XFile::oppPath() const
{
    if (NULL == fileinfo.pParent)
    {
        return L"";
    }

    WString strOppPath(fileinfo.pParent->oppPath());
    strOppPath << __wcFSSlant << fileinfo.strName;
    return std::move(strOppPath);
}

void XFile::Remove()
{
	if (NULL != fileinfo.pParent)
	{
		fileinfo.pParent->RemoveSubObject(this);
	}
}

void CPath::_findFile()
{
	if (E_FindFileStatus::FFS_None == m_eFindFileStatus)
	{
		(void)_onFindFile(m_paSubDir, m_paSubFile);
    }
}

void CPath::_onFindFile(TD_PathList& paSubDir, TD_XFileList& paSubFile)
{
    bool bRet = fsutil::findFile(this->absPath(), [&](tagFileInfo& fileInfo) {
        fileInfo.pParent = this;

        if (fileInfo.bDir)
		{
			XFile *pSubDir = _newSubDir(fileInfo);
			if (pSubDir)
			{
				paSubDir.add(pSubDir);
			}
		}
		else
		{
			XFile *pSubFile = _newSubFile(fileInfo);
			if (pSubFile)
			{
				paSubFile.add(pSubFile);
			}
		}
	});

	m_eFindFileStatus = bRet ? E_FindFileStatus::FFS_Exists : E_FindFileStatus::FFS_NotExists;

	paSubDir.qsort([&](const CPath& lhs, const CPath& rhs) {
		return _sort(lhs, rhs) < 0;
	});

	paSubDir.qsort([&](const XFile& lhs, const XFile& rhs) {
		return _sort(lhs, rhs) < 0;
	});
}

inline static int _sort(const wstring& lhs, const wstring& rhs)
{
#if __windows
    return strutil::collate(lhs, rhs);
#else
    return strutil::wstrToQStr(lhs).compare(strutil::wstrToQStr(rhs), Qt::CaseSensitivity::CaseInsensitive);
    //.localeAwareCompare(strutil::wstrToQStr(rhs.GetName()));
#endif

	return 1;
}

int CPath::_sort(const XFile& lhs, const XFile& rhs) const
{
	return ::_sort(lhs.GetName(), rhs.GetName());
}

void CPath::scan(const CB_PathScan& cb)
{
	(void)_scan(cb);
}

bool CPath::_scan(const CB_PathScan& cb)
{
    mtutil::usleep(1);

	_findFile();
	if (!cb(*this, m_paSubFile))
	{
		return false;
	}

	for (auto pSubDir : m_paSubDir)
	{
		if (!pSubDir->_scan(cb))
		{
			return false;
		}
	}

	return true;
}

XFile *CPath::FindSubPath(wstring strSubPath, bool bDir)
{
    __EnsureReturn(fileinfo.bDir, NULL);

	list<wstring> lstSubName;
	while (!strSubPath.empty())
	{
		wstring strSubName = fsutil::GetFileName(strSubPath);
		if (strSubName.empty())
		{
			break;
		}
		
		lstSubName.push_front(strSubName);

		strSubPath = fsutil::GetParentDir(strSubPath);
	}
	
	CPath *pSubDir = this;
	while (!lstSubName.empty())
	{
		wstring strSubName = lstSubName.front();
		lstSubName.pop_front();

		if (lstSubName.empty() && !bDir)
		{
			XFile *pSubFile = NULL;
			pSubDir->files()([&](XFile& file) {
				if (strutil::matchIgnoreCase(file.GetName(), strSubName))
				{
					pSubFile = &file;
					return false;
				}

				return true;
			});
			return pSubFile;
		}

		if (!pSubDir->dirs().any([&](CPath& dir) {
			if (strutil::matchIgnoreCase(dir.GetName(), strSubName))
			{
				pSubDir = &dir;
				return true;
			}

			return false;
		}))
		{
			return NULL;
		}
	}
	
	return pSubDir;
}

void CPath::RemoveSubObject(XFile *pSubPath)
{
	if (pSubPath)
	{
		if (pSubPath->fileInfo().bDir)
		{
			if (0 != m_paSubDir.del((CPath*)pSubPath))
			{
				delete pSubPath;
			}
		}
		else
		{
			if (0 != m_paSubFile.del(pSubPath))
			{
				delete pSubPath;
			}
		}
	}
}

void CPath::Clear()
{
	for (auto p : m_paSubDir)
	{
		delete p;
	}
	m_paSubDir.clear();

	for (auto p : m_paSubDir)
	{
		delete p;
	}
	m_paSubFile.clear();

	m_eFindFileStatus = E_FindFileStatus::FFS_None;
}
