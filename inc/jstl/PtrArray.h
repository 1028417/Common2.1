
#ifndef __PtrArray_H
#define __PtrArray_H

#include "JSArray.h"

#include "ptrcontainer.h"

namespace NS_JSTL
{
#define __PtrArraySuper JSArrayT<__Type*, __BaseType>

	template<typename __Type, template<typename...> class __BaseType = ptrvectorT>
	class PtrArray : public __PtrArraySuper
	{
	private:
		using __Super = __PtrArraySuper;

	protected:
		using __PtrType = __Type*;

		using __InitList_Ptr = InitList_T<__PtrType>;

		using __RefType = __Type&;
		using __ConstPtrRef = const __PtrType&;

		using __CB_RefType_void = CB_T_void<__RefType>;
		using __CB_RefType_bool = CB_T_bool<__RefType>;

		using __CB_RefType_Pos = CB_T_Pos<__RefType>;
		
	public:
		PtrArray()
		{
		}

		template<typename... args>
		explicit PtrArray(__ConstPtrRef ptr, const args&... others)
			: __Super(ptr, others...)
		{
		}

		template<typename... args>
		explicit PtrArray(__RefType ref, args&... others)
		{
			assign(ref, others...);
		}

		explicit PtrArray(const PtrArray& arr)
			: __Super(arr)
		{
		}

		PtrArray(PtrArray&& arr)
		{
			__Super::swap(arr);
		}

		PtrArray(__InitList_Ptr initList)
			: __Super(initList)
		{
		}

		template<typename T, typename _ITR = decltype(declval<T>().begin())>
		explicit PtrArray(T& container)
		{
			assign(container);
		}

		template<typename T, typename _ITR = decltype(declval<T>().begin())>
		explicit PtrArray(const T& container)
		{
			assign(container);
		}

		PtrArray& operator=(const PtrArray& arr)
		{
			__Super::assign(arr);
			return *this;
		}

		PtrArray& operator=(PtrArray&& arr)
		{
			__Super::swap(arr);
			return *this;
		}

		PtrArray& operator=(__InitList_Ptr initList)
		{
			assign(initList);
			return *this;
		}

		template <typename T>
		PtrArray& operator=(const T&t)
		{
			assign(t);
			return *this;
		}

		template <typename T>
		PtrArray& operator=(T&t)
		{
			assign(t);
			return *this;
		}

	protected:
		size_t _add(__PtrType ptr)
		{
			return __Super::_add(ptr);
		}

		size_t _add(__RefType ref)
		{
			return __Super::_add(&ref);
		}

		void _unshift(__ConstPtrRef ptr)
		{
			__Super::unshift(ptr);
		}

		void _unshift(__RefType ref)
		{
			__Super::unshift(&ref);
		}

	public:
		PtrArray& operator+= (__RefType rhs)
		{
			this->push(rhs);

			return *this;
		}

		PtrArray& operator+= (__ConstPtrRef rhs)
		{
			this->push(rhs);

			return *this;
		}

		template <typename T>
		PtrArray& operator+= (T& rhs)
		{
			this->push(rhs);

			return *this;
		}
		
		template <typename T>
		PtrArray& operator+= (const T& rhs)
		{
			this->push(rhs);

			return *this;
		}

		PtrArray& operator+= (__InitList_Ptr rhs)
		{
			this->push(rhs);
			
			return *this;
		}
		
		template <typename T>
		PtrArray& operator-= (const T& rhs)
		{
			__Super::del(rhs);
			return *this;
		}

		PtrArray& operator-= (__InitList_Ptr rhs)
		{
			__Super::del(rhs);
			return *this;
		}

		bool get(TD_PosType pos, __CB_RefType_void cb) const
		{
			__PtrType ptr = NULL;
			if (!__Super::get(pos, ptr))
			{
				return false;
			}

			if (NULL == ptr)
			{
				return false;
			}

			if (cb)
			{
				cb(*ptr);
			}

			return true;
		}

		bool set(TD_PosType pos, __ConstPtrRef ptr)
		{
			return __Super::set(pos, ptr);
		}

		bool set(TD_PosType pos, __RefType data)
		{
			return __Super::set(pos, &data);
		}

		template<typename... args>
		PtrArray& assign(__ConstPtrRef ptr, const args&... others)
		{
			__Super::assign(ptr, others...);
			return *this;
		}

		template<typename... args>
		PtrArray& assign(__RefType ref, args&... others)
		{
			__Super::clear();

			push(ref, others...);

			return *this;
		}

		template <typename T>
		PtrArray& assign(T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				__Super::clear();

				push(container);
			}

			return *this;
		}

		template <typename T>
		PtrArray& assign(const T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				__Super::clear();

				push(container);
			}

			return *this;
		}

		PtrArray& assign(__InitList_Ptr initList)
		{
			__Super::assign(initList);
			return *this;
		}

		template<typename... args>
		size_t push(__ConstPtrRef ptr, const args&... others)
		{
			return __Super::push(ptr, others...);
		}

		template<typename... args>
		size_t push(__RefType ref, args&... others)
		{
			(void)tagDynamicArgsExtractor<__RefType>::extract([&](__RefType ref) {
				_add(ref);
				return true;
			}, ref, others...);

			return __Super::size();
		}


		template<typename T>
		decltype(checkContainer<T, size_t>()) push(T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				for (auto&data : container)
				{
					_add(data);
				}
			}

			return __Super::size();
		}

		template<typename T>
		decltype(checkContainer<T, size_t>()) push(const T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				for (auto&data : container)
				{
					_add(data);
				}
			}

			return __Super::size();
		}

		size_t push(__InitList_Ptr initList)
		{
			return __Super::push(initList);
		}

		template<typename... args>
		PtrArray concat(__ConstPtrRef ptr, const args&... others) const
		{
			PtrArray arr(*this);
			arr.push(ptr, others...);
			return arr;
		}

		template<typename... args>
		PtrArray concat(__RefType ref, args&... others) const
		{
			PtrArray arr(*this);
			arr.push(ref, others...);
			return arr;
		}

		template<typename T>
		PtrArray concat(T& container) const
		{
			PtrArray arr(*this);
			arr.push(container);
			return arr;
		}

		template<typename T>
		PtrArray concat(const T& container) const
		{
			PtrArray arr(*this);
			arr.push(container);
			return arr;
		}

		PtrArray concat(__InitList_Ptr initList) const
		{
			PtrArray arr(*this);
			arr.push(initList);
			return arr;
		}

		template<typename... args>
		size_t unshift(__ConstPtrRef ptr, const args&... others)
		{
			return __Super::unshift(ptr, others...);
		}

		template<typename... args>
		size_t unshift(__RefType ref, args&... others)
		{
			(void)tagDynamicArgsExtractor<__RefType>::extract([&](__RefType ref) {
				_unshift(ref);
				return true;
			}, ref, others...);

			return __Super::size();
		}

		template<typename T>
		decltype(checkContainer<T, size_t>()) unshift(T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				for (auto&data : container)
				{
					_unshift(data);
				}
			}

			return __Super::size();
		}

		template<typename T>
		decltype(checkContainer<T, size_t>()) unshift(const T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				for (auto&data : container)
				{
					_unshift(data);
				}
			}

			return __Super::size();
		}

		size_t unshift(__InitList_Ptr initList)
		{
			return __Super::unshift(initList);
		}

		PtrArray slice(int startPos) const
		{
			PtrArray arr;

			startPos = __Super::_checkPos(startPos);
			if (startPos >= 0)
			{
				forEach([&](__RefType data) {
					arr.push(&data);
				}, (TD_PosType)startPos);
			}

			return arr;
		}

		PtrArray slice(int startPos, int endPos) const
		{
			PtrArray arr;

			startPos = __Super::_checkPos(startPos);
			endPos = __Super::_checkPos(endPos);

			if (startPos >= 0 && endPos >= 0 && startPos <= endPos)
			{
				forEach([&](__RefType data) {
					arr.push(&data);
					return true;
				}, (TD_PosType)startPos, size_t(endPos - startPos + 1));
			}

			return arr;
		}

		template<typename... args>
		PtrArray& splice(TD_PosType pos, size_t nRemove, __ConstPtrRef v, const args&... others)
		{
			__Super::splice(pos, nRemove, v, others...);

			return *this;
		}

		template<typename... args>
		PtrArray& splice(TD_PosType pos, size_t nRemove, __RefType ref, args&... others)
		{
			vector<__PtrType> vec;
			(void)tagDynamicArgsExtractor<__RefType>::extract([&](__RefType ref) {
				vec.push_back(&ref);
				return true;
			}, ref, others...);

			__Super::splice(pos, nRemove, vec);

			return *this;
		}

		template<typename T>
		PtrArray& splice(TD_PosType pos, size_t nRemove, T& container)
		{
			__Super::splice(pos, nRemove, container);

			return *this;
		}

		template<typename T>
		PtrArray& splice(TD_PosType pos, size_t nRemove, const T& container)
		{
			__Super::splice(pos, nRemove, container);

			return *this;
		}

		PtrArray& splice(TD_PosType pos, size_t nRemove, __InitList_Ptr initList)
		{
			__Super::splice(pos, nRemove, initList);

			return *this;
		}

		PtrArray& splice(TD_PosType pos, size_t nRemove)
		{
			__Super::splice(pos, nRemove, ((__InitList_Ptr) {}));

			return *this;
		}

	public:
		void forEach(__CB_RefType_Pos cb, TD_PosType startPos = 0, size_t count = 0) const
		{
			if (!cb)
			{
				return;
			}

			for (TD_PosType pos = startPos; pos < __Super::size(); pos++)
			{
				auto ptr = __Super::at(pos);
				if (NULL == ptr)
				{
					continue;
				}

				if (!cb(*ptr, pos))
				{
					break;
				}

				if (0 < count)
				{
					count--;
					if (0 == count)
					{
						break;
					}
				}
			}
		}

		void forEach(__CB_RefType_bool cb, TD_PosType startPos = 0, size_t count = 0) const
		{
			if (!cb)
			{
				return;
			}

			forEach([&](__RefType data, TD_PosType pos) {
				return cb(data);
			}, (TD_PosType)startPos);
		}

		int find(__CB_RefType_Pos cb, TD_PosType stratPos = 0) const
		{
			if (!cb)
			{
				return -1;
			}

			int iRet = -1;
			
			__Super::forEach([&](__ConstPtrRef ptr, TD_PosType pos) {
				if (NULL != ptr)
				{
					if (cb(*ptr, pos))
					{
						iRet = pos;
					}
				}

				return true;
			});

			return iRet;
		}

		bool getFront(__CB_RefType_void cb = NULL) const
		{
			return __Super::getFront([&](__ConstPtrRef ptr) {
				if (NULL != ptr)
				{
					if (cb)
					{
						cb(ptr);
					}
				}
			});
		}

		bool getBack(__CB_RefType_void cb = NULL) const
		{
			return __Super::getBack([&](__ConstPtrRef ptr) {
				if (NULL != ptr)
				{
					if (cb)
					{
						cb(ptr);
					}
				}
			});
		}

		bool pop(__CB_RefType_void cb = NULL)
		{
			return __Super::pop([&](__ConstPtrRef ptr) {
				if (NULL != ptr)
				{
					if (cb)
					{
						cb(ptr);
					}
				}
			});
		}

		bool shift(__CB_RefType_void cb = NULL)
		{
			return __Super::shift([&](__ConstPtrRef ptr) {
				if (NULL != ptr)
				{
					if (cb)
					{
						cb(ptr);
					}
				}
			});
		}

		PtrArray& sort(__CB_Sort_T<__Type> cb)
		{
            if (cb)
            {
                __Super::sort([&](__ConstPtrRef lhs, __ConstPtrRef rhs) {
                    if (NULL != lhs && NULL != rhs)
                    {
                        return cb(*lhs, *rhs);
                    }

                    return false;
                });
            }

			return *this;
		}

	public:
		template <typename T>
		JSArray<T> map(CB_T_Ret<__RefType, T> cb) const
		{
			JSArray<T> arr;

			if (cb)
			{
				forEach([&](__RefType ref) {
					arr.push(cb(ref));
					return true;
				});
			}

			return arr;
		}

		template <typename CB, typename RET = decltype(declval<CB>()(declval<__RefType>()))>
		JSArray<RET> map(const CB& cb) const
		{
			return map<RET>(cb);
		}

		PtrArray filter(__CB_RefType_bool cb) const
		{
			PtrArray arr;
			
			if (cb)
			{
				arr = __Super::filter([&](__ConstPtrRef ptr) {
					if (NULL == ptr)
					{
						return false;
					}

					return cb(*ptr);
				});
			}

			return arr;
		}

		bool every(__CB_RefType_bool cb) const
		{
			if (!cb)
			{
				return false;
			}

			return __Super::every([&](__ConstPtrRef ptr) {
				if (NULL == ptr)
				{
					return false;
				}

				return cb(*ptr);
			});
		}

		bool some(__CB_RefType_bool cb) const
		{
			if (!cb)
			{
				return false;
			}

			return __Super::some([&](__ConstPtrRef ptr) {
				if (NULL == ptr)
				{
					return false;
				}

				return cb(*ptr);
			});
		}
	};

	template<typename __Type>
	using ConstPtrArray = PtrArray<const __Type>;
}

#endif // __PtrArray_H
