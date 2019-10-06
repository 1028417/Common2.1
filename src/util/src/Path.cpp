
#include "util.h"

CPath::CPath(const wstring& strDir)
{
    m_fileInfo.bDir = true;

    m_fileInfo.strName = wsutil::rtrim_r(strDir, __wcFSSlant);
}

void CPath::SetDir(const wstring& strDir)
{
	Clear();

    m_fileInfo.bDir = true;

    m_fileInfo.strName = wsutil::rtrim_r(strDir, __wcFSSlant);
}

wstring CPath::GetName() const
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

wstring CPath::absPath() const
{
    if (m_fileInfo.pParent)
	{
        return m_fileInfo.pParent->absPath() + __wcFSSlant + m_fileInfo.strName;
	}

    return m_fileInfo.strName;
}

wstring CPath::oppPath() const
{
    if (NULL == m_fileInfo.pParent)
    {
        return L"";
    }

    WString strOppPath(m_fileInfo.pParent->oppPath());
    strOppPath << __wcFSSlant << m_fileInfo.strName;
    return strOppPath;
}

void CPath::_findFile()
{
	if (E_FindFileStatus::FFS_None == m_eFindFileStatus)
	{
		(void)_onFindFile(m_paSubDir, m_paSubFile);
    }
}

void CPath::_onFindFile(TD_PathList& paSubDir, TD_PathList& paSubFile)
{
    bool bRet = fsutil::findFile(this->absPath(), [&](tagFileInfo& fileInfo) {
        fileInfo.pParent = this;
        CPath *pSubPath = _newSubPath(fileInfo);
		if (NULL == pSubPath)
		{
			return;
		}

        if (fileInfo.bDir)
		{
			paSubDir.add(pSubPath);
		}
		else
		{
			paSubFile.add(pSubPath);
		}
	});

	m_eFindFileStatus = bRet ? E_FindFileStatus::FFS_Exists : E_FindFileStatus::FFS_NotExists;
		
	_sort(paSubDir);
	_sort(paSubFile);
}

inline static int _sort(const wstring& lhs, const wstring& rhs)
{
#if __windows
    return wsutil::collate(lhs, rhs);
#else
    return wsutil::toQStr(lhs).compare(wsutil::toQStr(rhs), Qt::CaseSensitivity::CaseInsensitive);
    //.localeAwareCompare(wsutil::toQStr(rhs.GetName()));
#endif

	return 1;
}

int CPath::_sort(const CPath& lhs, const CPath& rhs) const
{
	return ::_sort(lhs.GetName(), rhs.GetName());
}

void CPath::_sort(TD_PathList& paSubPath)
{
	paSubPath.qsort([&](const CPath& lhs, const CPath& rhs) {
		return _sort(lhs, rhs) < 0;
	});
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

CPath *CPath::FindSubPath(wstring strSubPath, bool bDir)
{
    __EnsureReturn(m_fileInfo.bDir, NULL);

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
	
	CPath *pPath = this;

	while (!lstSubName.empty() && NULL != pPath)
	{
		wstring strSubName = lstSubName.front();
		lstSubName.pop_front();
		
		cauto& paSubPath = (!lstSubName.empty() || bDir) ? pPath->dirs() : pPath->files();
		paSubPath([&](CPath& SubPath) {
			if (wsutil::matchIgnoreCase(SubPath.GetName(), strSubName))
			{
				pPath = &SubPath;
				return false;
			}

			return true;
		});
	}
	
	return pPath;
}

void CPath::RemoveSelf()
{
    if (NULL != m_fileInfo.pParent)
	{
        m_fileInfo.pParent->Remove(this);
	}
}

void CPath::Remove(CPath *pSubPath)
{
	if (NULL == pSubPath)
	{
		return;
	}

    if (pSubPath->m_fileInfo.bDir)
	{
		if (0 != m_paSubDir.del(pSubPath))
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
