
#include "util.h"

CPath::CPath(const wstring& strName, bool bDir)
{
	if (bDir)
	{
		SetDir(strName);
	}
	else
	{
		m_strName = strName;
	}
}

void CPath::SetDir(const wstring& strDir, bool bFindFile)
{
	Clear();

	m_bDir = true;

    m_strName = wsutil::rtrim_r(strDir, __wcFSSlant);

    if (bFindFile)
    {
        _findFile();
    }
}

wstring CPath::GetName() const
{
	if (NULL != m_pParentDir)
	{
		return m_strName;
	}
	else
	{
		return fsutil::GetFileName(m_strName);
	}
}

wstring CPath::GetPath() const
{
	if (NULL != m_pParentDir)
	{
        return m_pParentDir->GetPath() + __wcFSSlant + m_strName;
	}

	return m_strName;
}

wstring CPath::GetParentDir() const
{
	if (NULL != m_pParentDir)
	{
		return m_pParentDir->GetPath();
	}

	return L"";
}

void CPath::_findFile()
{
	if (!m_bFinded)
	{
		m_bFinded = true;

		m_bDirExists = false;

		_onFindFile(m_lstSubPath);
	}
}

void CPath::_onFindFile(TD_PathList& lstSubPath, bool bSort)
{
	m_bDirExists = fsutil::findFile(this->GetPath(), [&](const tagFileInfo& FileInfo) {
		CPath *pSubPath = NewSubPath(FileInfo);
		if (pSubPath)
		{
			lstSubPath.add(pSubPath);
		}
	});

	if (bSort)
	{
		m_lstSubPath.qsort([&](const CPath& lhs, const CPath& rhs) {
			return _sortCompare(lhs, rhs) < 0;
		});
	}
}

int CPath::_sortCompare(const CPath& lhs, const CPath& rhs) const
{
    if (lhs.m_bDir && !rhs.m_bDir)
    {
        return -1;
    }

    if (lhs.m_bDir == rhs.m_bDir)
    {
#if __android
        return wsutil::toQStr(lhs.GetName()).compare(wsutil::toQStr(rhs.GetName()), Qt::CaseSensitivity::CaseInsensitive);
                //.localeAwareCompare(wsutil::toQStr(rhs.GetName()));
#else
        return wsutil::collate(lhs.GetName(), rhs.GetName());
#endif
    }

    return 1;
}

void CPath::_GetSubPath(TD_PathList *plstSubDir, TD_PathList *plstSubFile)
{
	GetSubPath()([&](CPath& SubPath) {
		if (SubPath.m_bDir)
		{
			if (plstSubDir)
			{
				plstSubDir->add(SubPath);
			}
		}
		else
		{
			if (plstSubFile)
			{
				plstSubFile->add(SubPath);
			}
		}
	});
}

bool CPath::hasSubDir()
{
	return GetSubPath().any([&](CPath& subPath) {
		return subPath.IsDir();
	});
}

bool CPath::hasSubFile()
{
	return GetSubPath().any([&](CPath& subPath) {
		return !subPath.IsDir();
	});
}

CPath *CPath::FindSubPath(wstring strSubPath, bool bDir)
{
	__EnsureReturn(m_bDir, NULL);

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
		
		pPath->GetSubPath()([&](CPath& SubPath) {
			if (lstSubName.empty())
			{
				if (SubPath.m_bDir != bDir)
				{
					return true;
				}
			}
			else
			{
				if (!SubPath.m_bDir)
				{
					return true;
				}
			}

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

void CPath::RemoveSubPath(CPath *pSubPath)
{
	if (0 != m_lstSubPath.del(pSubPath))
	{
		delete pSubPath;
	}
}

void CPath::RemoveSelf()
{
	if (NULL != m_pParentDir)
	{
		m_pParentDir->RemoveSubPath(this);
	}
}

void CPath::Clear()
{
	m_bDirExists = false;

	m_lstSubPath([](CPath& SubPath) {
		delete &SubPath;
	});
	m_lstSubPath.clear();

	m_bFinded = false;
}

bool CPath::enumSubFile(const function<bool(CPath& dir, TD_PathList& lstSubFile)>& cb)
{
	TD_PathList lstSubFile;
	GetSubFile(lstSubFile);
	if (lstSubFile)
	{
		if (!cb(*this, lstSubFile))
		{
			return false;
		}
	}

	TD_PathList lstSubDir;
	GetSubDir(lstSubDir);
	for (auto pSubDir : lstSubDir)
	{
		if (!pSubDir->enumSubFile(cb))
		{
			return false;
		}
	}

	return true;
}
