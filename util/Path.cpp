
#include <util.h>

#include <Path.h>

struct tagPathSortor
{
	bool operator () (CPath *lhs, CPath *rhs) const
	{
		if (lhs->m_bDir && !rhs->m_bDir)
		{
			return true;
		}

		if (lhs->m_bDir == rhs->m_bDir)
		{
			return util::StrCompareUseCNCollate(lhs->m_strName, rhs->m_strName) < 0;
		}

		return false;
	}
};

wstring CPath::GetName()
{
	if (NULL != m_pParentPath)
	{
		return m_strName;
	}
	else
	{
		return fsutil::GetFileName(m_strName);
	}
}

void CPath::SetName(const wstring& strNewName)
{
	m_strName = strNewName;
}

wstring CPath::GetPath() const
{
	if (NULL != m_pParentPath)
	{
		return m_pParentPath->GetPath() + __BackSlant + m_strName;
	}

	return m_strName;
}

UINT CPath::GetSubPathCount()
{
	if (NULL == m_plstSubPath)
	{
		return 0;
	}

	return m_plstSubPath->size();
}

bool CPath::GetSubPath(TD_PathList& lstSubPath)
{
	__EnsureReturn(FindFile() && m_plstSubPath, false);
	
	lstSubPath.add(*m_plstSubPath);

	return true;
}

bool CPath::GetSubPath(TD_PathList *plstSubDir, TD_PathList *plstSubFile)
{
	__EnsureReturn(FindFile() && m_plstSubPath, false);

	for (auto& pSubPath : *m_plstSubPath)
	{
		if (pSubPath->m_bDir)
		{
			if (plstSubDir)
			{
				plstSubDir->push_back(pSubPath);
			}
		}
		else
		{
			if (plstSubFile)
			{
				plstSubFile->push_back(pSubPath);
			}
		}
	}

	return true;
}

CPath *CPath::GetSubPath(wstring strSubPath, bool bDir)
{
	list<wstring> lstSubDirs;
	while (!strSubPath.empty())
	{
		wstring strName;
		wstring strExtName;
		fsutil::GetFileName(strSubPath, &strName, &strExtName);

		strName += strExtName;
		if (strName.empty())
		{
			break;
		}
		
		lstSubDirs.push_back(strName);

		strSubPath = fsutil::GetParentDir(strSubPath);
	}
	
	CPath *pPath = this;

	while (!lstSubDirs.empty() && NULL != pPath)
	{
		wstring strName = lstSubDirs.back();
		lstSubDirs.pop_back();

		TD_PathList lstSubPath;
		pPath->GetSubPath(lstSubPath);
		
		pPath = NULL;

		for (TD_PathList::iterator itSubPath = lstSubPath.begin()
			; itSubPath != lstSubPath.end(); ++itSubPath)
		{
			if (lstSubDirs.empty())
			{
				if ((*itSubPath)->m_bDir != bDir)
				{
					continue;
				}
			}
			else
			{
				if (!(*itSubPath)->m_bDir)
				{
					continue;
				}
			}

			if (util::StrCompareIgnoreCase((*itSubPath)->m_strName, strName))
			{
				pPath = *itSubPath;
				break;
			}
		}
	}

	return pPath;
}

void CPath::ClearSubPath()
{
	if (NULL != m_plstSubPath)
	{
		for (TD_PathList::iterator itSubPath = m_plstSubPath->begin()
			; itSubPath != m_plstSubPath->end(); ++itSubPath)
		{
			delete *itSubPath;
		}

		m_plstSubPath->clear();
		delete m_plstSubPath;
		m_plstSubPath = NULL;
	}
}

void CPath::RemoveSubPath(const TD_PathList& lstDeletePaths)
{
	__Ensure(m_plstSubPath);

	for (TD_PathList::iterator itSubPath = m_plstSubPath->begin()
		; itSubPath != m_plstSubPath->end(); )
	{
		if (util::ContainerFind(lstDeletePaths, *itSubPath))
		{
			delete *itSubPath;
			itSubPath = m_plstSubPath->erase(itSubPath);
			continue;
		}			

		itSubPath++;
	}
}

bool CPath::FindFile()
{
	if (!m_bDir || NULL != m_plstSubPath)
	{
		return true;
	}

	CPath *pSubPath = NULL;

	TD_PathList lstSubPath;
	fsutil::FindFile(this->GetPath() + L"\\*", [&](const tagFindData& findData) {
		pSubPath = NewSubPath(findData, this);
		if (pSubPath)
		{
			lstSubPath.push_back(pSubPath);
		}
		return true;
	});

	lstSubPath.sort(tagPathSortor());

	m_plstSubPath = new TD_PathList();
	m_plstSubPath->swap(lstSubPath);

	return true;
}

bool CPath::HasFile()
{
	__EnsureReturn(m_plstSubPath, FALSE);

	for (auto pSubPath : *m_plstSubPath)
	{
		if (!pSubPath->m_bDir)
		{
			return true;
		}
	}

	return false;
}
