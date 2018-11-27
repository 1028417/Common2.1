
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

#include <string>
#include <string>
#include <sstream>

#include <list>
#include <vector>
#include <set>
#include <map>

#include <algorithm>

#include <functional>

#include <thread>
#include <future>

#include <fstream>

using namespace std;

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
		wstringstream strmResult;

		for (_C::const_iterator it = container.begin(); ; )
		{
			strmResult << *it;
			
			it++;
			__EnsureBreak(it != container.end());

			strmResult << strSplitor;
		}

		return strmResult.str();
	}
};

template <class _PtrType>
class ptrlist : public list<_PtrType>
{
public:
	ptrlist()
	{
	}
	
	ptrlist(_PtrType ptr)
	{
		Insert(ptr);
	}

	template <typename _RefType>
	ptrlist(const list<_RefType*>& container)
	{
		Insert(container);
	}

	template <typename _RefType>
	ptrlist(const list<_RefType>& container)
	{
		Insert(container);
	}
	
public:
	void Insert(_PtrType ptr)
	{
		push_back(ptr);
	}

	template <typename _RefType>
	void Insert(const list<_RefType*>& container)
	{
		for (list<_RefType*>::const_iterator it = container.begin(); it != container.end(); ++it)
		{
			push_back((_PtrType)*it);
		}
	}

	template <typename _RefType>
	void Insert(const list<_RefType>& container)
	{
		for (list<_RefType>::const_iterator it = container.begin(); it != container.end(); ++it)
		{
			push_back((_PtrType)(_RefType*)&*it);
		}
	}

	template <typename _RefType>
	void Insert(const vector<_RefType*>& container)
	{
		for (vector<_RefType*>::const_iterator it = container.begin(); it != container.end(); ++it)
		{
			push_back((_PtrType)*it);
		}
	}

	template <typename _RefType>
	void Insert(const vector<_RefType>& container)
	{
		for (vector<_RefType>::const_iterator it = container.begin(); it != container.end(); ++it)
		{
			push_back((_PtrType)(_RefType*)&*it);
		}
	}
};


template <class _PtrType>
class ptrvector : public vector<_PtrType>
{
public:
	ptrvector()
	{
	}

	ptrvector(_PtrType ptr)
	{
		insert(ptr);
	}

	template <typename _RefType>
	ptrvector(const vector<_RefType*>& container)
	{
		insert(container);
	}

	template <typename _RefType>
	ptrvector(const vector<_RefType>& container)
	{
		Insert(container);
	}

	template <typename _RefType>
	ptrvector(const list<_RefType*>& container)
	{
		Insert(container);
	}

	template <typename _RefType>
	ptrvector(const list<_RefType>& container)
	{
		Insert(container);
	}

public:
	void Insert(_PtrType ptr)
	{
		push_back(ptr);
	}

	template <typename _RefType>
	void Insert(const vector<_RefType*>& container)
	{
		for (vector<_RefType*>::const_iterator it = container.begin(); it != container.end(); ++it)
		{
			push_back((_PtrType)*it);
		}
	}

	template <typename _RefType>
	void Insert(const vector<_RefType>& container)
	{
		for (vector<_RefType>::const_iterator it = container.begin(); it != container.end(); ++it)
		{
			push_back((_PtrType)(_RefType*)&*it);
		}
	}

	template <typename _RefType>
	void Insert(const list<_RefType*>& container)
	{
		for (list<_RefType*>::const_iterator it = container.begin(); it != container.end(); ++it)
		{
			push_back((_PtrType)*it);
		}
	}

	template <typename _RefType>
	void Insert(const list<_RefType>& container)
	{
		for (list<_RefType>::const_iterator it = container.begin(); it != container.end(); ++it)
		{
			push_back((_PtrType)(_RefType*)&*it);
		}
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

typedef ptrlist<CListObject*> TD_ListObjectList;

class CTreeObject;

typedef ptrlist<CTreeObject*> TD_TreeObjectList;

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
typedef ptrlist<CPathObject*> TD_PathObjectList;

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

typedef ptrlist<CDirObject*> TD_DirObjectList;

class __UtilExt CDirObject : public CPathObject, public CTreeObject
{
public:
	CDirObject(const wstring& strDir = L"")
		: CPathObject(strDir)
	{
	}

	CDirObject(const wstring& strName, const TD_DirObjectList& lstSubDirObjects)
		: CPathObject(strName, TD_PathList(lstSubDirObjects))
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

		TD_DirObjectList lstDirObjects(lstSubPaths);
		lstChilds.Insert(lstDirObjects);
	}
};
