
#include "stdafx.h"

#include <Path.h>

#include <locale>
static const char *ZH_CN_LOCALE_STRING = "Chinese_china";
static const locale zh_CN_locale = locale(ZH_CN_LOCALE_STRING);
static const collate<wchar_t>& zh_CN_collate = use_facet<collate<wchar_t> >(zh_CN_locale);

struct tagPathSortor
{
	bool operator () (CPath *pPath1, CPath *pPath2)
	{
		if (pPath1->m_bDir && !pPath2->m_bDir)
		{
			return true;
		}

		if (pPath1->m_bDir == pPath2->m_bDir)
		{
			return zh_CN_collate.compare(pPath1->m_strName.c_str(), pPath1->m_strName.c_str() + pPath1->m_strName.size()
				, pPath2->m_strName.c_str(), pPath2->m_strName.c_str() + pPath2->m_strName.size()) < 0;
		}

		return false;
	}
};

wstring CPath::GetPath()
{
	if (m_pParentPath)
	{
		return m_pParentPath->GetPath() + __BackSlant + m_strName;
	}

	return m_strName;
}

bool CPath::GetSubPath(TD_PathList& lstSubPath)
{
	__EnsureReturn(FindFile() && m_plstSubPath, false);
	
	lstSubPath.Insert(*m_plstSubPath);

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

		strSubPath = fsutil::GetParentPath(strSubPath);
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

	CString cstrFind = this->GetPath().c_str();
	cstrFind.Append(L"\\*");
	CFileFind fileFind;
	bool bExists = fileFind.FindFile(cstrFind);
	if (!bExists)
	{
		return false;
	}
	
	m_plstSubPath = new TD_PathList();
	
	CPath *pSubPath = NULL;

	TD_PathList lstSubPath;
	while (bExists)
	{
		bExists = fileFind.FindNextFile();

		if (fileFind.IsDots() || fileFind.IsSystem())
		{
			continue;
		}

		pSubPath = NewSubPath(fileFind, this);
		if (pSubPath)
		{
			lstSubPath.push_back(pSubPath);
		}
	}

	lstSubPath.sort(tagPathSortor());
	
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
