
#ifndef __ptrcontain_H
#define __ptrcontain_H

#include <list>
#include <vector>
using namespace std;

template <template<typename...> typename __BaseType, class _RefType>
class ptrcontainT : public __BaseType<_RefType*>
{
	using _PtrType = _RefType*;

public:
	ptrcontainT()
	{
	}

	ptrcontainT(_PtrType ptr)
	{
		add(ptr);
	}

	ptrcontainT(_RefType& ref)
	{
		add(ref);
	}

	ptrcontainT(const list<_PtrType>& container)
		: list(container.begin(), container.end())
	{
	}

	ptrcontainT(const vector<_PtrType>& container)
		: list(container.begin(), container.end())
	{
	}

	template <typename T>
	ptrcontainT(const list<T*>& container)
	{
		add(container);
	}

	template <typename T>
	ptrcontainT(list<T*>& container)
	{
		add(container);
	}

	template <typename T>
	ptrcontainT(list<T>& container)
	{
		add(container);
	}

	template <typename T>
	ptrcontainT(const list<T*>& container, bool bDynamicCastFlag)
	{
		_addDowncast(container, bDynamicCastFlag);
	}

	template <typename T>
	ptrcontainT(const vector<T*>& container)
	{
		add(container);
	}

	template <typename T>
	ptrcontainT(vector<T*>& container)
	{
		add(container);
	}

	template <typename T>
	ptrcontainT(vector<T>& container)
	{
		add(container);
	}

	template <typename T>
	ptrcontainT(const vector<T*>& container, bool bDynamicCastFlag)
	{
		_addDowncast(container, bDynamicCastFlag);
	}

public:
	void add(_PtrType ptr)
	{
		push_back(ptr);
	}

	void add(_RefType& ref)
	{
		push_back(&ref);
	}

	void del(_PtrType ptr)
	{
		auto itr = std::find(begin(), end(), ptr);
		if (itr != end())
		{
			erase(itr);
		}
	}

	void del(const _RefType& ref)
	{
		auto itr = std::find(begin(), end(), &ref);
		if (itr != end())
		{
			erase(itr);
		}
	}

private:
	template <typename T>
	void _addPtr(T& container)
	{
		for (auto ptr : container)
		{
			add(ptr);
		}
	}

	template <typename T>
	void _addDowncast(T& container, bool bDynamicCastFlag)
	{
		for (auto& ptr : container)
		{
			auto newPtr = dynamic_cast<_PtrType>(ptr);
			if (NULL != newPtr)
			{
				add(newPtr);
			}
			else
			{
				if (bDynamicCastFlag)
				{
					add(newPtr);
				}
			}
		}
	}

	template <typename T>
	void _addRef(T& container)
	{
		for (auto& ref : container)
		{
			add(ref);
		}
	}

public:
	template <typename T>
	void add(const list<T*>& container)
	{
		_addPtr(container);
	}

	template <typename T>
	void add(list<T*>& container)
	{
		_addPtr(container);
	}

	template <typename T>
	void add(list<T>& container)
	{
		_addRef(container);
	}

	template <typename T>
	void add(const vector<T*>& container)
	{
		_addPtr(container);
	}

	template <typename T>
	void add(vector<T*>& container)
	{
		_addPtr(container);
	}

	template <typename T>
	void add(vector<T>& container)
	{
		_addRef(container);
	}

	template <typename T>
	void add(const list<T*>& container, bool bDynamicCastFlag)
	{
		_addDowncast(container, bDynamicCastFlag);
	}

	template <typename T>
	void add(const vector<T*>& container, bool bDynamicCastFlag)
	{
		_addDowncast(container, bDynamicCastFlag);
	}
};

template <class _RefType>
using ptrlist = ptrcontainT<list, _RefType>;

template <class _RefType>
using ptrvector = ptrcontainT<vector, _RefType>;

#endif // __ptrcontain_H
