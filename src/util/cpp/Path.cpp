
#include "util.h"

wstring XFile::fileName() const
{
    if (m_fileInfo.pParent)
	{
        return m_fileInfo.strName;
	}
	else
	{
        return fsutil::GetFileName(m_fileInfo.strName);
	}
}

wstring XFile::parentDir() const
{
	if (m_fileInfo.pParent)
	{
		return m_fileInfo.pParent->path();
	}

	return fsutil::GetParentDir(m_fileInfo.strName);
}

wstring XFile::path() const
{
    if (m_fileInfo.pParent)
	{
        auto strAbsPath = m_fileInfo.pParent->path();
        if (!strAbsPath.empty())
        {
            strAbsPath.push_back(__wcPathSeparator);
            strAbsPath.append(m_fileInfo.strName);
			return strAbsPath;
		}
	}

    return m_fileInfo.strName;
}

const CPath* XFile::rootDir() const
{
    if (NULL == m_fileInfo.pParent)
	{
		return (const CPath*)this;
	}

    return m_fileInfo.pParent->rootDir();
}

void XFile::remove()
{
    if (NULL != m_fileInfo.pParent)
	{
        m_fileInfo.pParent->remove(this);
	}
}

void CPath::_findFile()
{
	if (E_FindFileStatus::FFS_None == m_eFindFileStatus)
	{
        m_eFindFileStatus = E_FindFileStatus::FFS_Exists;
		(void)_onFindFile(m_paSubDir, m_paSubFile);
    }
}

void CPath::_onFindFile(TD_PathList& paSubDir, TD_XFileList& paSubFile)
{
    bool bRet = fsutil::findFile(this->path(), [&](tagFileInfo& fileInfo) {
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
    if (bRet)
    {
        m_eFindFileStatus = E_FindFileStatus::FFS_Exists;
    }

	paSubDir.qsort([&](const CPath& lhs, const CPath& rhs) {
		return _sort(lhs, rhs) < 0;
	});

    paSubFile.qsort([&](const XFile& lhs, const XFile& rhs) {
		return _sort(lhs, rhs) < 0;
	});
}

inline static int _sort(cwstr lhs, cwstr rhs)
{
#if __windows
    return strutil::collate(lhs, rhs);
#else
    return __WS2Q(lhs).compare(__WS2Q(rhs), Qt::CaseSensitivity::CaseInsensitive);
    //.localeAwareCompare(__WS2Q(rhs.GetName()));
#endif

	return 1;
}

int CPath::_sort(const XFile& lhs, const XFile& rhs) const
{
    return ::_sort(lhs.fileName(), rhs.fileName());
}

void CPath::scan(const CB_PathScan& cb)
{
	(void)_scan(cb);
}

bool CPath::_scan(const CB_PathScan& cb)
{
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

        mtutil::usleep(1);
	}

	return true;
}

XFile *CPath::findSubPath(wstring strSubPath, bool bDir)
{
    __EnsureReturn(m_fi.bDir, NULL);

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
                if (strutil::matchIgnoreCase(file.fileName(), strSubName))
				{
					pSubFile = &file;
					return false;
				}

				return true;
			});
			return pSubFile;
		}

		if (!pSubDir->dirs().any([&](CPath& dir) {
            if (strutil::matchIgnoreCase(dir.fileName(), strSubName))
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

void CPath::remove(XFile *pSubPath)
{
	if (pSubPath)
	{
		if (pSubPath->isDir())
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

void CPath::clear()
{
    _onClear();

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
