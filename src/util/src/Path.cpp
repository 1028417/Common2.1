
#include "util.h"

CPath::CPath(const wstring& strName, bool bDir)
{
	if (bDir)
	{
		SetDir(strName);
	}
	else
	{
		m_FileInfo.strName = strName;
	}
}

void CPath::SetDir(const wstring& strDir)
{
	Clear();

	m_FileInfo.bDir = true;

	m_FileInfo.strName = wsutil::rtrim_r(strDir, __wcFSSlant);
}

wstring CPath::GetName() const
{
	if (NULL != m_FileInfo.pParent)
	{
		return m_FileInfo.strName;
	}
	else
	{
		return fsutil::GetFileName(m_FileInfo.strName);
	}
}

wstring CPath::GetPath() const
{
	if (NULL != m_FileInfo.pParent)
	{
        return m_FileInfo.pParent->GetPath() + __wcFSSlant + m_FileInfo.strName;
	}

	return m_FileInfo.strName;
}

wstring CPath::GetParentDir() const
{
	if (NULL != m_FileInfo.pParent)
	{
		return m_FileInfo.pParent->GetPath();
	}

	return L"";
}

void CPath::findFile(bool bScanAll)
{
	if (bScanAll || !m_bFinded)
	{
		m_bFinded = true;

		if (bScanAll)
		{
			Clear();
		}

		m_bDirExists = fsutil::findFile(this->GetPath(), [&](const tagFileInfo& FileInfo) {
			CPath *pSubPath = NewSubPath(FileInfo);
			if (NULL == pSubPath)
			{
				return;
			}
			
			if (bScanAll && FileInfo.bDir)
			{
				pSubPath->findFile(true);
				if (pSubPath->size() == 0)
				{
					delete pSubPath;
					return;
				}
			}

			m_lstSubPath.add(pSubPath);
		});

		_sort(m_lstSubPath);
	}
}

void CPath::_sort(TD_PathList& lstSubPath)
{
	lstSubPath.qsort([&](const CPath& lhs, const CPath& rhs) {
		return _sortCompare(lhs, rhs) < 0;
	});
}

int CPath::_sortCompare(const CPath& lhs, const CPath& rhs) const
{
    if (lhs.IsDir() && !rhs.IsDir())
    {
        return -1;
    }

    if (lhs.IsDir() == rhs.IsDir())
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
		if (SubPath.IsDir())
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
	__EnsureReturn(m_FileInfo.bDir, NULL);

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
				if (SubPath.IsDir() != bDir)
				{
					return true;
				}
			}
			else
			{
				if (!SubPath.IsDir())
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
	if (NULL != m_FileInfo.pParent)
	{
		m_FileInfo.pParent->RemoveSubPath(this);
	}
}

void CPath::Clear()
{
	m_lstSubPath([](CPath& SubPath) {
		delete &SubPath;
	});
	m_lstSubPath.clear();

	m_bFinded = false;

	m_bDirExists = false;
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
