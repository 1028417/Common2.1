
#ifndef __ptrcontainer_H
#define __ptrcontainer_H

#include "_define.h"

namespace NS_SSTL
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
		ptrcontainerT() = default;

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
		bool _add(__PtrType ptr)
		{
			if (NULL == ptr)
			{
				return false;
			}

			__Super::push_back(ptr);
			return true;
		}

		bool _add(__RefType ref)
		{
			__Super::push_back(&ref);
			return true;
		}

		template <typename T, typename = checkClass_t<T, __Type>>
		bool _add(T* ptr, ...)
		{
			return _add(dynamic_cast<__PtrType>(ptr));
		}

		template <typename T, typename = checkClass_t<T, __Type>>
		bool _add(T& ref, ...)
		{
			return _add(&ref);
		}

		template <typename T>
		size_t _addContainer(T& container)
		{
			size_t uRet = 0;

			CItrVisitor<T> Visitor(container);
			for (auto&data : Visitor)
			{
				if (_add(data))
				{
					uRet++;
				}
			}

			return uRet;
		}

		bool _set(TD_PosType pos, __PtrType ptr)
		{
			if (NULL == ptr)
			{
				return false;
			}

			if (pos >= __Super::size())
			{
				return false;
			}

			__Super::at(pos) = ptr;

			return true;
		}

		bool _set(TD_PosType pos, __RefType ref)
		{
			return _set(pos, &ref);
		}

		template <typename T, typename = checkClass_t<T, __Type>>
		bool _set(TD_PosType pos, T* ptr, ...)
		{
			return _set(pos, dynamic_cast<__PtrType>(ptr));
		}

		template <typename T, typename = checkClass_t<T, __Type>>
		bool _set(TD_PosType pos, T& ref, ...)
		{
			return _set(pos, &ref);
		}

		size_t _del(__ConstPtr ptr)
		{
			if (NULL == ptr)
			{
				return 0;
			}

			size_t uRet = 0;

			for (auto itr = __Super::begin(); itr != __Super::end(); )
			{
				if (*itr == ptr)
				{
					uRet++;

					itr = __Super::erase(itr);
				}
				else
				{
					itr++;
				}
			}

			return uRet;
		}

		size_t _del(__ConstRef ref)
		{
			return _del(&ref);
		}

		template <typename T, typename = checkClass_t<T, __Type>>
		size_t _del(T* ptr, ...)
		{
			return _del((__ConstPtr)dynamic_cast<__ConstPtr>(ptr));
		}

		template <typename T, typename = checkClass_t<T, __Type>>
		size_t _del(T& ref, ...)
		{
			return _del(&ref);
		}

		template <typename T>
		size_t _delContainer(T& container)
		{
			size_t uRet = 0;

			CItrVisitor<T> Visitor(container);
			for (auto&data : Visitor)
			{
				uRet += _del(data);
			}

			return uRet;
		}

		int _indexOf(__ConstPtr ptr) const
		{
			if (NULL == ptr)
			{
				return false;
			}

			auto itr = __Super::begin();
			for (int pos = 0; itr != __Super::end(); itr++, pos++)
			{
				if (*itr == ptr)
				{
					return pos;
				}
			}

			return -1;
		}

		int _indexOf(__ConstRef ref) const
		{
			return _indexOf(&ref);
		}

		template <typename T, typename = checkClass_t<T, __Type>>
		int _indexOf(T* ptr, ...) const
		{
			return _indexOf((__ConstPtr)dynamic_cast<__ConstPtr>(ptr));
		}

		template <typename T, typename = checkClass_t<T, __Type>>
		int _indexOf(T& ref, ...) const
		{
			return _indexOf(&ref);
		}

	public:
		bool add(__PtrType ptr)
		{
			return _add(ptr);
		}

		bool add(__RefType ref)
		{
			return _add(ref);
		}

		template<typename T>
		bool add(T* ptr)
		{
			return _add(ptr);
		}

		template<typename T, typename = checkNotContainer_t<T>>
		bool add(T& ref)
		{
			return _add(ref);
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
		
		bool set(TD_PosType pos, __PtrType ptr)
		{
			return _set(pos, ptr);
		}

		bool set(TD_PosType pos, __RefType ref)
		{
			return _set(pos, ref);
		}

		template<typename T>
		bool set(TD_PosType pos, T* ptr)
		{
			return _set(pos, ptr);
		}

		template<typename T>
		bool set(TD_PosType pos, T& ref)
		{
			return _set(pos, ref);
		}

		size_t del(__ConstPtr ptr)
		{
			return _del(ptr);
		}

		size_t del(__ConstRef ref)
		{
			return _del(ref);
		}

		template <typename T>
		size_t del(T* ptr)
		{
			return _del(ptr);
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

		int indexOf(__ConstPtr ptr) const
		{
			return _indexOf(ptr);
		}

		int indexOf(__ConstRef ref) const
		{
			return _indexOf(ref);
		}

		template <typename T>
		int indexOf(T* ptr) const
		{
			return _indexOf(ptr);
		}

		template <typename T>
		int indexOf(T& ref) const
		{
			return _indexOf(ref);
		}
	};
};

#endif // __ptrcontainer_H
