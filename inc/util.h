
#pragma once

class __CommonPrjExt util
{
public:
	static BOOL StrCompareIgnoreCase(const wstring& str1, const wstring& str2);

	static int StrFind(const wstring& str, const wstring& strToFind, bool bIgnoreCase=false);

	static wstring StrLowerCase(const wstring& str);
	static wstring StrUpperCase(const wstring& str);

	static string WStrToStr(const wstring&str, UINT CodePage = CP_ACP);
	static wstring StrToWStr(const string&str, UINT CodePage = CP_ACP);
	
	static bool IsUTF8Str(const string& strText);

	template <class _C, class _V>
	static BOOL ContainerFind(_C& container, _V value)
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
	ptrvector(const list<_RefType*>& container)
	{
		insert(container);
	}

	template <typename _RefType>
	ptrvector(const list<_RefType>& container)
	{
		insert(container);
	}

public:
	void insert(_PtrType ptr)
	{
		push_back(ptr);
	}

	template <typename _RefType>
	void insert(const list<_RefType*>& container)
	{
		for (list<_RefType*>::const_iterator it = container.begin(); it != container.end(); ++it)
		{
			push_back((_PtrType)*it);
		}
	}

	template <typename _RefType>
	void insert(const list<_RefType>& container)
	{
		for (list<_RefType>::const_iterator it = container.begin(); it != container.end(); ++it)
		{
			push_back((_PtrType)(_RefType*)&*it);
		}
	}
};
