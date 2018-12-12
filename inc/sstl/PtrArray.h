
#ifndef __PtrArray_H
#define __PtrArray_H

#include "SArray.h"

#include "ptrcontainer.h"

namespace NS_SSTL
{
#define __PtrArraySuper SArrayT<__Type*, __BaseType>

	template<typename __Type, template<typename...> class __BaseType>
	class PtrArrayT : public __PtrArraySuper
	{
		friend tagItrVisitor;

	private:
		using __Super = __PtrArraySuper;

		using __PtrType = __Type*;
		using __RefType = __Type&;

		using __ConstRef = const __Type&;
		using __ConstPtr = const __Type*;

#ifndef _MSC_VER
		__UsingSuperType(__ContainerType);
		__UsingSuperType(__ItrType);
		__UsingSuperType(__ItrConstType);

		__UsingSuperType(__InitList);
#endif

		using __CB_RefType_void = CB_T_void<__RefType>;
		using __CB_RefType_bool = CB_T_bool<__RefType>;

		using __CB_RefType_Pos_void = CB_T_Pos_RET<__RefType, void>;
		using __CB_RefType_Pos_bool = CB_T_Pos_RET<__RefType, bool>;

		__ContainerType& m_data = __Super::m_data;

	public:
		PtrArrayT()
		{
		}

		template <typename T, typename... args, typename = checkNotSameType_t<T, __Type>>
		explicit PtrArrayT(T* ptr, args... others)
		{
			add(ptr, others...);
		}
		
		template <typename T, typename... args, typename = checkNotSameType_t<T, __Type>
			, typename = checkNotSameType_t<T, __PtrType>, typename = checkNotContainer_t<T>>
		explicit PtrArrayT(T& ref, args&... others)
		{
			add(ref, others...);
		}

		template<typename... args>
		explicit PtrArrayT(__PtrType ptr, args... others)
			: __Super(ptr, others...)
		{
		}

		template<typename... args>
		explicit PtrArrayT(__RefType ref, args&... others)
		{
			assign(ref, others...);
		}

		explicit PtrArrayT(__ContainerType&& container)
		{
			__Super::swap(container);
		}

		PtrArrayT(PtrArrayT&& arr)
		{
			__Super::swap(arr);
		}

		PtrArrayT(const PtrArrayT& arr)
			: __Super(arr)
		{
		}

		explicit PtrArrayT(__InitList initList)
		{
			assign(initList);
		}

		template<typename T, typename = checkContainer_t<T>>
		explicit PtrArrayT(const T& container)
		{
			assign(container);
		}

		template<typename T, typename = checkContainer_t<T>>
		explicit PtrArrayT(T& container)
		{
			assign(container);
		}

		PtrArrayT& operator=(__ContainerType&& container)
		{
			__Super::swap(container);
			return *this;
		}

		PtrArrayT& operator=(PtrArrayT&& arr)
		{
			__Super::swap(arr);
			return *this;
		}

		PtrArrayT& operator=(const PtrArrayT& arr)
		{
			assign(arr);
			return *this;
		}

		PtrArrayT& operator=(__InitList initList)
		{
			assign(initList);
			return *this;
		}

		template <typename T>
		PtrArrayT& operator=(const T&t)
		{
			assign(t);
			return *this;
		}

		template <typename T>
		PtrArrayT& operator=(T&t)
		{
			assign(t);
			return *this;
		}

	private:
		void _add(const __PtrType& ptr) override
		{
			m_data.add((__PtrType)ptr);
		}

		size_t _del(const __PtrType& ptr) override
		{
			return m_data.del((__PtrType)ptr);
		}

		__ItrType begin()
		{
			return m_data.begin();
		}
		__ItrType end()
		{
			return m_data.end();
		}

		__ItrConstType begin() const
		{
			return m_data.cbegin();
		}
		__ItrConstType end() const
		{
			return m_data.cend();
		}

		//template <typename T, typename = checkIter_t<T>>
		//T erase(const T& itr)
		//{
		//	return m_data.erase(itr);
		//}

		void _addFront(__PtrType ptr)
		{
			__Super::addFront(ptr);
		}

		void _addFront(__RefType ref)
		{
			__Super::addFront(&ref);
		}

	public:
		PtrArrayT& operator+= (__RefType rhs)
		{
			this->add(rhs);
			return *this;
		}

		PtrArrayT& operator+= (__PtrType rhs)
		{
			this->add(rhs);
			return *this;
		}

		template <typename T>
		PtrArrayT& operator+= (T& rhs)
		{
			this->add(rhs);
			return *this;
		}

		template <typename T>
		PtrArrayT& operator+= (const T& rhs)
		{
			this->add(rhs);
			return *this;
		}

		PtrArrayT& operator+= (__InitList rhs)
		{
			this->add(rhs);
			return *this;
		}

		PtrArrayT& operator-= (__RefType rhs)
		{
			this->del(rhs);
			return *this;
		}

		PtrArrayT& operator-= (__PtrType rhs)
		{
			this->del(rhs);
			return *this;
		}

		template <typename T>
		PtrArrayT& operator-= (T& rhs)
		{
			this->del(rhs);
			return *this;
		}

		template <typename T>
		PtrArrayT& operator-= (const T& rhs)
		{
			this->del(rhs);
			return *this;
		}
		
		PtrArrayT& operator-= (__InitList rhs)
		{
			this->del(rhs);
			return *this;
		}

		friend PtrArrayT operator+ (const PtrArrayT& lhs, const PtrArrayT& rhs)
		{
			return PtrArrayT(lhs) += rhs;
		}

		friend PtrArrayT operator+ (const PtrArrayT& lhs, __InitList rhs)
		{
			return PtrArrayT(lhs) += rhs;
		}

		friend PtrArrayT operator+ (__InitList lhs, const PtrArrayT& rhs)
		{
			return PtrArrayT(lhs) += rhs;
		}

		template <typename T>
		friend PtrArrayT operator+ (const PtrArrayT& lhs, const T& rhs)
		{
			return PtrArrayT(lhs) += rhs;
		}

		template <typename T, typename = checkNotIsBase_t<PtrArrayT, T>>
		friend PtrArrayT operator+ (const T& lhs, const PtrArrayT& rhs)
		{
			return PtrArrayT(lhs) += rhs;
		}
		
		friend PtrArrayT operator- (const PtrArrayT& lhs, const PtrArrayT& rhs)
		{
			return PtrArrayT(lhs) -= rhs;
		}

		friend PtrArrayT operator- (const PtrArrayT& lhs, __InitList rhs)
		{
			return PtrArrayT(lhs) -= rhs;
		}

		friend PtrArrayT operator- (__InitList lhs, const PtrArrayT& rhs)
		{
			return PtrArrayT(lhs) -= rhs;
		}\

		template <typename T>
		friend PtrArrayT operator- (const PtrArrayT& lhs, const T& rhs)
		{
			return PtrArrayT(lhs) -= rhs;
		}

		template <typename T, typename = checkNotIsBase_t<PtrArrayT, T>>
		friend PtrArrayT operator- (const T& lhs, const PtrArrayT& rhs)
		{
			return PtrArrayT(lhs) -= rhs;
		}

		template <typename T>
		friend PtrArrayT operator& (const PtrArrayT& lhs, const T& rhs)
		{
			PtrArrayT arr;
			for (auto&data : rhs)
			{
				if (lhs.includes(data))
				{
					arr.add(data);
				}
			}

			return arr;
		}

		template <typename T>
		friend PtrArrayT operator& (const PtrArrayT& lhs, T& rhs)
		{
			PtrArrayT arr;
			for (auto&data : rhs)
			{
				if (lhs.includes(data))
				{
					arr.add(data);
				}
			}

			return arr;
		}

		friend PtrArrayT operator& (const PtrArrayT& lhs, __InitList rhs)
		{
			return lhs & PtrArrayT(rhs);
		}

	public:
		int indexOf(__ConstPtr ptr) const
		{
			return m_data.indexOf(ptr);
		}

		int indexOf(__ConstRef ref) const
		{
			return m_data.indexOf(ref);
		}

		template <typename T>
		int indexOf(T* ptr) const
		{
			return m_data.indexOf(ptr);
		}

		template <typename T>
		int indexOf(T& ref) const
		{
			return m_data.indexOf(ref);
		}

		bool includes(__ConstPtr ptr) const
		{
			return indexOf(ptr) >= 0;
		}

		bool includes(__ConstRef ref) const
		{
			return indexOf(ref) >= 0;
		}

		template <typename T>
		bool includes(T* ptr) const
		{
			return indexOf(ptr) >= 0;
		}

		template <typename T>
		bool includes(T& ref) const
		{
			return indexOf(ref) >= 0;
		}

		size_t del(__ConstRef ref)
		{
			return m_data.del(ref);
		}

		size_t del(__ConstPtr ptr)
		{
			return m_data.del(ptr);
		}
		
		template <typename T, typename... args, typename = checkNotSameType_t<T, __Type>>
		size_t del(T* ptr, args... others)
		{
			size_t uRet = 0;
			(void)tagDynamicArgsExtractor<T*>::extract([&](T* ptr) {
				if (m_data.del(ptr))
				{
					uRet++;
				}
				return true;
			}, ptr, others...);
			return uRet;
		}

		template <typename T, typename... args, typename = checkNotSameType_t<T, __Type>
			, typename = checkNotSameType_t<T, __PtrType>, typename = checkNotContainer_t<T>>
		size_t del(T& ref, args&... others)
		{
			size_t uRet = 0;
			(void)tagDynamicArgsExtractor<T>::extract([&](T& ref) {
				if (m_data.del(ref))
				{
					uRet++;
				}
				return true;
			}, ref, others...);
			return uRet;
		}

		template<typename... args>
		size_t del(__ConstPtr ptr, args... others)
		{
			size_t uRet = 0;
			(void)tagDynamicArgsExtractor<__ConstPtr>::extract([&](__ConstPtr ptr) {
				if (m_data.del(ptr))
				{
					uRet++;
				}
				return true;
			}, ptr, others...);
			return uRet;
		}

		template<typename... args>
		size_t del(__ConstRef ref, args&... others)
		{
			size_t uRet = 0;
			(void)tagDynamicArgsExtractor<__ConstRef>::extract([&](__ConstRef ref) {
				if (m_data.del(ref))
				{
					uRet++;
				}
				return true;
			}, ref, others...);
			return uRet;
		}

		template<typename T, typename = checkContainer_t<T>>
		size_t del(T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				return m_data.del(container);
			}

			return 0;
		}

		template<typename T, typename = checkContainer_t<T>>
		size_t del(const T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				return m_data.del(container);
			}

			return 0;
		}

		size_t del(__InitList initList)
		{
			return m_data.del(initList);
		}

		template <typename CB, typename = checkCBRet_t<CB, E_DelConfirm, __RefType>, typename = void>
		size_t del(const CB& cb)
		{
			return __Super::del([&](__PtrType ptr) {
				if (NULL != ptr)
				{
					return cb(*ptr);
				}

				return E_DelConfirm::DC_No;
			});
		}

		template <typename T, typename... args, typename = checkNotSameType_t<T, __Type>>
		size_t add(T* ptr, args... others)
		{
			size_t uRet = 0;

			(void)tagDynamicArgsExtractor<T*>::extract([&](T* ptr) {
				if (m_data.add(ptr))
				{
					uRet++;
				}

				return true;
			}, ptr, others...);

			return uRet;
		}
		
		template <typename T, typename... args, typename = checkNotSameType_t<T, __Type>
			, typename = checkNotSameType_t<T, __PtrType>, typename = checkNotContainer_t<T>>
		size_t add(T& ref, args&... others)
		{
			size_t uRet = 0;

			(void)tagDynamicArgsExtractor<T>::extract([&](T& ref) {
				if (m_data.add(ref))
				{
					uRet++;
				}

				return true;
			}, ref, others...);

			return uRet;
		}

		template<typename... args>
		size_t add(__PtrType ptr, args... others)
		{
			size_t uRet = 0;

			(void)tagDynamicArgsExtractor<__PtrType>::extract([&](__PtrType ptr) {
				if (m_data.add(ptr))
				{
					uRet++;
				}

				return true;
			}, ptr, others...);

			return uRet;
		}

		template<typename... args>
		void add(__RefType ref, args&... others)
		{
			(void)tagDynamicArgsExtractor<__RefType>::extract([&](__RefType ref) {
				m_data.add(ref);
				return true;
			}, ref, others...);
		}

		template<typename T, typename = checkContainer_t<T>>
		size_t add(T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				return m_data.add(container);
			}

			return 0;
		}

		template<typename T, typename = checkContainer_t<T>>
		size_t add(const T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				return m_data.add(container);
			}

			return 0;
		}

		size_t add(__InitList initList)
		{
			return m_data.add(initList);
		}

		template<typename... args>
		size_t addFront(__PtrType ptr, args... others)
		{
			return __Super::addFront(ptr, others...);
		}

		template<typename... args>
		size_t addFront(__RefType ref, args&... others)
		{
			(void)tagDynamicArgsExtractor<__RefType>::extract([&](__RefType ref) {
				_addFront(ref);
				return true;
			}, ref, others...);

			return __Super::size();
		}

		template <typename T, typename... args, typename = checkNotSameType_t<T, __Type>
			, typename = checkNotSameType_t<T, __PtrType>, typename = checkNotContainer_t<T>>
			size_t addFront(T& ref, args&... others)
		{
			(void)tagDynamicArgsExtractor<T>::extract([&](T& ref) {
				_addFront(ref);
				return true;
			}, ref, others...);

			return __Super::size();
		}

		template<typename T, typename = checkContainer_t<T>>
		size_t addFront(T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				CItrVisitor<T> Visitor(container);
				for (auto&data : Visitor)
				{
					_addFront(data);
				}
			}

			return __Super::size();
		}

		template<typename T, typename = checkContainer_t<T>>
		size_t addFront(const T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				for (auto&data : container)
				{
					_addFront(data);
				}
			}

			return __Super::size();
		}

		size_t addFront(__InitList initList)
		{
			return __Super::addFront(initList);
		}

		template<typename... args>
		PtrArrayT& assign(__PtrType ptr, args... others)
		{
			__Super::assign(ptr, others...);
			return *this;
		}

		template<typename... args>
		PtrArrayT& assign(__RefType ref, args&... others)
		{
			m_data.clear();

			add(ref, others...);

			return *this;
		}

		template <typename T, typename... args, typename = checkNotSameType_t<T, __Type>
			, typename = checkNotSameType_t<T, __PtrType>, typename = checkNotContainer_t<T>>
		PtrArrayT& assign(T& ref, args&... others)
		{
			m_data.clear();

			add(ref, others...);

			return *this;
		}
			
		PtrArrayT& assign(__ContainerType&& container)
		{
			__Super::swap(container);
			return *this;
		}

		PtrArrayT& assign(PtrArrayT&& arr)
		{
			__Super::swap(arr);
			return *this;
		}

		template <typename T>
		PtrArrayT& assign(T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				m_data.clear();

				add(container);
			}

			return *this;
		}

		template <typename T>
		PtrArrayT& assign(const T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				m_data.clear();

				add(container);
			}

			return *this;
		}

		PtrArrayT& assign(__InitList initList)
		{
			__Super::assign(initList);
			return *this;
		}

		bool get(TD_PosType pos, __CB_RefType_void cb) const
		{
			if (pos >= m_data.size())
			{
				return false;
			}

			__PtrType ptr = m_data[pos];
			if (NULL == ptr)
			{
				return false;
			}
			
			cb(*ptr);
			
			return true;
		}

		bool set(TD_PosType pos, __PtrType ptr)
		{
			return m_data.set(pos, ptr);
		}

		bool set(TD_PosType pos, __RefType ref)
		{
			return m_data.set(pos, &ref);
		}

		template<typename... args>
		PtrArrayT concat(__PtrType ptr, args... others) const
		{
			PtrArrayT arr(*this);
			arr.add(ptr, others...);
			return arr;
		}

		template<typename... args>
		PtrArrayT concat(__RefType ref, args&... others) const
		{
			PtrArrayT arr(*this);
			arr.add(ref, others...);
			return arr;
		}

		template <typename T, typename... args, typename = checkNotSameType_t<T, __Type>
			, typename = checkNotSameType_t<T, __PtrType>, typename = checkNotContainer_t<T>>
		PtrArrayT concat(T& ref, args&... others) const
		{
			PtrArrayT arr(*this);
			arr.add(ref, others...);
			return arr;
		}

		template<typename T>
		PtrArrayT concat(T& container) const
		{
			PtrArrayT arr(*this);
			arr.add(container);
			return arr;
		}

		template<typename T>
		PtrArrayT concat(const T& container) const
		{
			PtrArrayT arr(*this);
			arr.add(container);
			return arr;
		}

		PtrArrayT concat(__InitList initList) const
		{
			PtrArrayT arr(*this);
			arr.add(initList);
			return arr;
		}

		PtrArrayT slice(int startPos) const
		{
			PtrArrayT arr;

			startPos = __Super::_checkPos(startPos);
			if (startPos >= 0)
			{
				forEach([&](__RefType ref) {
					arr.add(&ref);
				}, (TD_PosType)startPos);
			}

			return arr;
		}

		PtrArrayT slice(int startPos, int endPos) const
		{
			PtrArrayT arr;

			startPos = __Super::_checkPos(startPos);
			endPos = __Super::_checkPos(endPos);

			if (startPos >= 0 && endPos >= 0 && startPos <= endPos)
			{
				forEach([&](__RefType ref) {
					arr.add(&ref);
				}, (TD_PosType)startPos, size_t(endPos - startPos + 1));
			}

			return arr;
		}

		template<typename... args>
		PtrArrayT& splice(TD_PosType pos, size_t nRemove, __PtrType v, args... others)
		{
			__Super::splice(pos, nRemove, v, others...);

			return *this;
		}

		template<typename... args>
		PtrArrayT& splice(TD_PosType pos, size_t nRemove, __RefType ref, args&... others)
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
		PtrArrayT& splice(TD_PosType pos, size_t nRemove, T& container)
		{
			__Super::splice(pos, nRemove, container);

			return *this;
		}

		template<typename T>
		PtrArrayT& splice(TD_PosType pos, size_t nRemove, const T& container)
		{
			__Super::splice(pos, nRemove, container);

			return *this;
		}

		PtrArrayT& splice(TD_PosType pos, size_t nRemove, __InitList initList)
		{
			__Super::splice(pos, nRemove, initList);

			return *this;
		}

		PtrArrayT& splice(TD_PosType pos, size_t nRemove)
		{
			__Super::splice(pos, nRemove, ((__InitList) {}));

			return *this;
		}

	public:
		void operator() (__CB_RefType_Pos_bool cb, TD_PosType startPos = 0, size_t count = 0) const
		{
			__Super::operator() ([&](__PtrType ptr, TD_PosType pos) {
				if (NULL != ptr)
				{
					return cb(*ptr, pos);
				}

				return true;
			}, startPos, count);
		}
		
		template <typename CB, typename = checkSameType_t<decltype(declval<CB>()(declval<__RefType>(), 0)), void>>
		void operator() (const CB& cb, TD_PosType startPos = 0, size_t count = 0) const
		{
			__Super::operator() ([&](__PtrType ptr, TD_PosType pos) {
				if (NULL != ptr)
				{
					cb(*ptr, pos);
				}
			}, startPos, count);
		}

		template <typename CB, typename = checkSameType_t<decltype(declval<CB>()(declval<__RefType>())), void>, typename=void>
		void operator() (const CB& cb, TD_PosType startPos = 0, size_t count = 0) const
		{
			__Super::operator() ([&](__PtrType ptr, TD_PosType pos) {
				if (NULL != ptr)
				{
					cb(*ptr);
				}
			}, startPos, count);
		}

		void operator() (__CB_RefType_bool cb, TD_PosType startPos = 0, size_t count = 0) const
		{
			__Super::operator() ([&](__PtrType ptr) {
				if (NULL != ptr)
				{
					return cb(*ptr);
				}

				return true;
			}, startPos, count);
		}

		int find(__CB_RefType_Pos_bool cb, TD_PosType stratPos = 0) const
		{
			int iRetPos = -1;
			
			(*this)([&](__RefType ref, TD_PosType pos) {
				if (cb(ref, pos))
				{
					iRetPos = pos;
					return false;
				}

				return true;
			});

			return iRetPos;
		}

		bool getFront(__CB_RefType_void cb = NULL) const
		{
			return __Super::getFront([&](__PtrType ptr) {
				if (NULL != ptr && cb)
				{
					cb(*ptr);
				}
			});
		}

		bool getBack(__CB_RefType_void cb = NULL) const
		{
			return __Super::getBack([&](__PtrType ptr) {
				if (NULL != ptr && cb)
				{
					cb(*ptr);
				}
			});
		}

		bool popFront(__CB_RefType_void cb = NULL)
		{
			return __Super::shift([&](__PtrType ptr) {
				if (NULL != ptr && cb)
				{
					cb(*ptr);
				}
			});
		}

		bool popBack(__CB_RefType_void cb = NULL)
		{
			return __Super::pop([&](__PtrType ptr) {
				if (NULL != ptr && cb)
				{
					cb(*ptr);
				}
			});
		}

		PtrArrayT& qsort(__CB_Sort_T<__Type> cb)
		{
			__Super::qsort([&](__PtrType lhs, __PtrType rhs) {
                if (NULL != lhs && NULL != rhs)
                {
                    return cb(*lhs, *rhs);
                }

                return false;
            });

			return *this;
		}

	public:
		template <typename T>
		SArray<T> map(CB_T_Ret<__RefType, T> cb) const
		{
			SArray<T> arr;

			(*this)([&](__RefType ref) {
				arr.add(cb(ref));
			});
			
			return arr;
		}

		template <typename CB, typename RET = decltype(declval<CB>()(declval<__RefType>()))>
		SArray<RET> map(const CB& cb) const
		{
			return map<RET>(cb);
		}

		PtrArrayT filter(__CB_RefType_bool cb) const
		{
			PtrArrayT arr;

			for (auto& ptr : m_data)
			{
				if (NULL != ptr && cb(*ptr))
				{
					arr.add(ptr);
				}
			}

			return arr;
		}

		bool every(__CB_RefType_bool cb) const
		{
			return __Super::every([&](__PtrType ptr) {
				if (NULL == ptr)
				{
					return false;
				}

				return cb(*ptr);
			});
		}

		bool some(__CB_RefType_bool cb) const
		{
			return __Super::some([&](__PtrType ptr) {
				if (NULL == ptr)
				{
					return false;
				}

				return cb(*ptr);
			});
		}
	};
}

#endif // __PtrArray_H
