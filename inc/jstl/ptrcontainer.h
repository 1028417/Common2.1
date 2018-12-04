
#ifndef __ptrcontainer_H
#define __ptrcontainer_H

#include "_define.h"

#include <list>

namespace NS_JSTL
{
	template <template<typename...> typename __BaseType, class __PtrType>
	class ptrcontainerT : public __BaseType<__PtrType>
	{
	private:
		using __Super = __BaseType<__PtrType>;

	protected:
		using __Type = typename remove_reference<decltype(*(__PtrType)NULL)>::type;
		using __RefType = __Type&;
		
		using __ConstRef = const __Type&;
		using __ConstPtr = const __Type*;

		using __InitList = InitList_T<__PtrType>;

	public:
		ptrcontainerT()
		{
		}

		template<class _Iter, typename = checkIter_t<_Iter>>
		explicit ptrcontainerT(_Iter _First, _Iter _Last)
		{
			for (auto itr = _First; itr != _Last; itr++)
			{
				_add(*itr);
			}
		}

		explicit ptrcontainerT(__Super&& container)
		{
			__Super::swap(container);
		}

		ptrcontainerT(ptrcontainerT&& container)
		{
			__Super::swap(container);
		}

		ptrcontainerT(const ptrcontainerT& container)
			: __Super(container.begin(), container.end())
		{
		}

		explicit ptrcontainerT(__InitList initLst)
		{
			_addContainer(initLst);
		}

		template <typename T, typename = checkContainer_t<T>>
		explicit ptrcontainerT(const T& container)
		{
			_addContainer(container);
		}

		template <typename T, typename = checkContainer_t<T>>
		explicit ptrcontainerT(T& container)
		{
			_addContainer(container);
		}
		explicit ptrcontainerT(__PtrType ptr)
		{
			_add(ptr);
		}

		explicit ptrcontainerT(__RefType ref)
		{
			_add(ref);
		}

		template <typename T>
		explicit ptrcontainerT(T* ptr)
		{
			_add(ptr);
		}

		ptrcontainerT& operator = (const ptrcontainerT& container)
		{
			__Super::assign(container.begin(), container.end());
			return *this;
		}

	private:
		inline bool _add(__RefType ref)
		{
			__Super::push_back(&ref);
			return true;
		}

		inline bool _add(__PtrType ptr)
		{
			if (NULL != ptr)
			{
				__Super::push_back(ptr);
				return true;
			}
			
			return false;
		}

		/*template <typename = checkNotConstType_t<__Type>>
		inline void _add(typename remove_const<__Type>::type * ptr, ...)
		{
			if (NULL != ptr)
			{
				__Super::push_back(ptr);
			}
		}*/

		template <typename T, typename = checkClass_t<T, __Type>>
		inline bool _add(T* ptr, ...)
		{
			return _add(dynamic_cast<__PtrType>(ptr));
		}

		template <typename T, typename = checkClass_t<T, __Type>>
		inline bool _add(T& ref, ...)
		{
			return _add(dynamic_cast<__PtrType>(&ref));
		}

		template <typename T>
		size_t _addContainer(T& container)
		{
			size_t uRet = 0;

			for (auto&data : container)
			{
				if (_add(data))
				{
					uRet++;
				}
			}

			return uRet;
		}

		inline size_t _del(__ConstPtr ptr)
		{
			size_t uRet = 0;

			auto itr = __Super::begin();
			while (true)
			{
				itr = find(itr, __Super::end(), ptr);
				if (itr != __Super::end())
				{
					itr = __Super::erase(itr);
				
					uRet++;
				}
				else
				{
					break;
				}
			}

			return uRet;
		}

		inline size_t _del(__ConstRef ref)
		{
			return _del(&ref);
		}

		template <typename T>
		size_t _del(T* ptr)
		{
			return _del((__ConstPtr)ptr);
		}

		template <typename T>
		size_t _delContainer(T& container)
		{
			size_t uRet = 0;

			for (auto&data : container)
			{
				if (_del(data))
				{
					uRet++;
				}
			}

			return uRet;
		}

	public:
		ptrcontainerT& add(__RefType ref)
		{
			__Super::push_back(&ref);
			return *this;
		}

		bool add(__PtrType ptr)
		{
			return _add(ptr);
		}

		size_t add(__InitList initLst)
		{
			return _addContainer(initLst);
		}

		template <typename T, typename = checkContainer_t<T>>
		size_t add(const T& container)
		{
			return _addContainer(container);
		}

		template <typename T, typename = checkContainer_t<T>>
		size_t add(T& container)
		{
			return _addContainer(container);
		}

	public:
		//template <typename = checkNotConstType_t<__Type>>
		size_t del(__ConstRef ref)
		{
			return _del(&ref);
		}

		size_t del(__ConstPtr ptr)
		{
			return _del(ptr);
		}

		template <typename T>
		size_t del(T* ptr)
		{
			return del((__ConstPtr)ptr);
		}

		size_t del(__InitList initLst)
		{
			return _delContainer(initLst);
		}

		template <typename T, typename = checkContainer_t<T>>
		size_t del(const T& container)
		{
			return _delContainer(container);
		}

		template <typename T, typename = checkContainer_t<T>>
		size_t del(T& container)
		{
			return _delContainer(container);
		}
	};

	template <class __PtrType>
	using ptrvectorT = ptrcontainerT<vector, __PtrType>;

	template <class __Type>
	using ptrvector = ptrvectorT<__Type*>;

	template <class __PtrType>
	using ptrlistT = ptrcontainerT<list, __PtrType>;

	template <class __Type>
	using ptrlist = ptrlistT<__Type*>;
};

#endif // __ptrcontainer_H
