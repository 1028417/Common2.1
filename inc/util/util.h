
#pragma once

#pragma warning(disable: 4251)

#include <Windows.h>

#ifdef __UtilPrj
#define __UtilExt __declspec(dllexport)
#else
#define __UtilExt __declspec(dllimport)
#endif

#define	__BackSlant L'\\'

#define __Ensure(x) \
	if (!(x)) \
	{ \
		return; \
	}

#define __EnsureReturn(x, y) \
	if (!(x)) \
	{ \
		return y; \
	}

#define __EnsureContinue(x) \
	if (!(x)) \
	{ \
		continue; \
	}

#define __EnsureBreak(x) \
	if (!(x)) \
	{ \
		break; \
	}

using namespace std;

#include <string>
#include <string>
#include <sstream>

#include <list>
#include <vector>
#include <set>
#include <map>

#include <algorithm>

#include <functional>
using fn_voidvoid = function<void()>;

#include <thread>
#include <future>

#include <fstream>

#include <jstl/jstl.h>
using namespace NS_JSTL;

class __UtilExt util
{
public:
	static void getCurrentTime(int& nHour, int& nMinute);
	static void getCurrentTime(tm& atm);

	static wstring FormatTime(const FILETIME& fileTime, const wstring& strFormat);
	static wstring FormatTime(time_t time, const wstring& strFormat);
	static wstring FormatTime(const tm& atm, const wstring& strFormat);

	static wstring trim(const wstring& strText, char chr=' ');

	static void SplitString(const wstring& strText, char cSplitor, vector<wstring>& vecRet, bool bTrim=false);

	static int StrCompareUseCNCollate(const wstring& lhs, const wstring& rhs);
	static bool StrCompareIgnoreCase(const wstring& str1, const wstring& str2);

	static int StrFindIgnoreCase(const wstring& str, const wstring& strToFind);

	static wstring StrLowerCase(const wstring& str);
	static wstring StrUpperCase(const wstring& str);

	static string WStrToStr(const wstring&str, UINT CodePage = CP_ACP);
	static wstring StrToWStr(const string&str, UINT CodePage = CP_ACP);
	
	static bool IsUTF8Str(const string& strText);

	template <class _C, class _V>
	static bool ContainerFind(_C& container, _V value)
	{
		return std::find(container.begin(), container.end(), value) != container.end();
	}

	template <class _C, class _V>
	static auto ContainerFindRef(_C& container, _V& value)
	{
		auto itr = container.begin();
		for (; itr != container.end(); itr++)
		{
			if ((void*)&*itr == &value)
			{
				break;
			}
		}
	
		return itr;
	}

	template <class _C>
	static wstring ContainerToStr(const _C& container, const wstring& strSplitor)
	{
		wstringstream ssResult;
		for (_C::const_iterator it = container.begin(); ; )
		{
			ssResult << *it;
			
			it++;
			__EnsureBreak(it != container.end());

			ssResult << strSplitor;
		}

		return ssResult.str();
	}
};

struct __UtilExt tagCNSortor
{
	bool operator()(const wstring& lhs, const wstring& rhs) const
	{
		return util::StrCompareUseCNCollate(lhs, rhs)<0;
	}
};

#include "SQLiteDB.h"

#include "fsutil.h"

#include "ProFile.h"

#include "WorkThread.h"

#include "fsdlg.h"

#include "Path.h"

class __UtilExt CListObject
{
public:
	virtual void GenListItem(bool bReport, vector<wstring>& vecText, int& iImage)
	{
	}

	virtual wstring GetRenameText()
	{
		return L"";
	}

	virtual bool OnListItemRename(const wstring& strNewName)
	{
		return false;
	}
};

typedef ptrlist<CListObject> TD_ListObjectList;

class CTreeObject;

typedef ptrlist<CTreeObject> TD_TreeObjectList;

class __UtilExt CTreeObject
{
public:
	CTreeObject()
	{
		m_hTreeItem = NULL;
	}

public:
	void *m_hTreeItem;

	virtual wstring GetTreeText()
	{
		return L"";
	};

	virtual int GetTreeImage()
	{
		return 0;
	}

	virtual void GetTreeChilds(TD_TreeObjectList& lstChilds)
	{
		//do nothing
	}
};

class CPathObject;
typedef ptrlist<CPathObject> TD_PathObjectList;

class __UtilExt CPathObject : public CPath, public CListObject
{
public:
	CPathObject()
	{
	}

	CPathObject(const wstring& strDir)
		: CPath(strDir)
	{
	}

	CPathObject(const wstring& strDir, const TD_PathObjectList& lstSubPathObjects)
		: CPath(strDir, TD_PathList(lstSubPathObjects))
	{
	}

	CPathObject(const tagFindData& findData, CPath *pParentPath)
		: CPath(findData, pParentPath)
	{
	}

	virtual ~CPathObject()
	{
	}

protected:
	virtual CPath *NewSubPath(const tagFindData& findData, CPath *pParentPath)
	{
		return new CPathObject(findData, pParentPath);
	}

public:
	void GenListItem(bool bReport, vector<wstring>& vecText, int& iImage) override
	{
		vecText.push_back(m_strName);

		vecText.push_back(to_wstring(m_uFileSize));

		//vecText.push_back((LPCTSTR)GetFileModifyTime());
	}

	//wstring GetFileModifyTime()
	//{
	//	if (m_bDir)
	//	{
	//		return L"";
	//	}

	//	return CTime(m_modifyTime).Format(_T("%y-%m-%d %H:%M"));
	//}
};

class CDirObject;

typedef ptrlist<CDirObject> TD_DirObjectList;

class __UtilExt CDirObject : public CPathObject, public CTreeObject
{
public:
	CDirObject(const wstring& strDir = L"")
		: CPathObject(strDir)
	{
	}

	CDirObject(const wstring& strName, const TD_DirObjectList& lstSubDirObjects)
		: CPathObject(strName, TD_PathObjectList(lstSubDirObjects))
	{
	}

	CDirObject(const tagFindData& findData, CPath *pParentPath)
		: CPathObject(findData, pParentPath)
	{
	}

	virtual ~CDirObject()
	{
	}

protected:
	virtual CPath *NewSubPath(const tagFindData& findData, CPath *pParentPath)
	{
		__EnsureReturn(findData.isDir(), NULL);

		return new CDirObject(findData, pParentPath);
	}

public:
	wstring GetTreeText() override
	{
		return m_strName;
	}

	void GetTreeChilds(TD_TreeObjectList& lstChilds)
	{
		TD_PathList lstSubPaths;
		this->GetSubPath(lstSubPaths);

		TD_DirObjectList lstDirObjects(lstSubPaths, false);
		lstChilds.add(lstDirObjects);
	}
};
