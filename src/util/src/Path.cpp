
#include "util.h"

CPath::CPath(const wstring& strDir)
{
	m_fi.bDir = true;

	m_fi.strName = wsutil::rtrim_r(strDir, __wcFSSlant);
}

void CPath::SetDir(const wstring& strDir)
{
	Clear();

    m_fi.bDir = true;

	m_fi.strName = wsutil::rtrim_r(strDir, __wcFSSlant);
}

wstring CPath::GetName() const
{
    if (NULL != m_fi.pParent)
	{
        return m_fi.strName;
	}
	else
	{
        return fsutil::GetFileName(m_fi.strName);
	}
}

wstring CPath::absPath() const
{
    if (m_fi.pParent)
	{
        return m_fi.pParent->absPath() + __wcFSSlant + m_fi.strName;
	}

    return m_fi.strName;
}

wstring CPath::oppPath() const
{
    if (m_fi.pParent)
    {
		WString strOppPath(m_fi.pParent->oppPath());
		strOppPath << __wcFSSlant << m_fi.strName;
		return strOppPath;
    }

    return L"";
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
	bool bRet = fsutil::findFile(this->absPath(), [&](tagFileInfo& FileInfo) {
		FileInfo.pParent = this;
		CPath *pSubPath = _newSubPath(FileInfo);
		if (NULL == pSubPath)
		{
			return;
		}

		if (FileInfo.bDir)
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
#if __android
	return wsutil::toQStr(lhs).compare(wsutil::toQStr(rhs), Qt::CaseSensitivity::CaseInsensitive);
	//.localeAwareCompare(wsutil::toQStr(rhs.GetName()));
#else
	return wsutil::collate(lhs, rhs);
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
    if (NULL != m_fi.pParent)
	{
        m_fi.pParent->Remove(this);
	}
}

void CPath::Remove(CPath *pSubPath)
{
	if (NULL == pSubPath)
	{
		return;
	}

    if (pSubPath->m_FileInfo.bDir)
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
