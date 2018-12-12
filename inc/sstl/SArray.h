
#ifndef __SSArray_H
#define __SSArray_H

#include "SContainer.h"

#include "SMap.h"

namespace NS_SSTL
{
#define __SArraySuper SContainerT<__DataType, __BaseType<__DataType>>

	template<typename __DataType, template<typename...> class __BaseType>
	class SArrayT : public __SArraySuper
	{
	private:
		using __Super = __SArraySuper;

#ifndef _MSC_VER
	protected:
		__UsingSuperType(__ContainerType);
		__UsingSuperType(__ItrType);
		__UsingSuperType(__ItrConstType);

		__UsingSuperType(__InitList);

		__UsingSuperType(__DataRef);
		__UsingSuperType(__DataConstRef);

		__UsingSuperType(__CB_Ref_void);
		__UsingSuperType(__CB_Ref_bool);

		__UsingSuperType(__CB_ConstRef_void);
		__UsingSuperType(__CB_ConstRef_bool);
#endif

		using __CB_Ref_Pos_void = CB_T_Pos_RET<__DataRef, void>;
		using __CB_Ref_Pos_bool = CB_T_Pos_RET<__DataRef, bool>;
		using __CB_ConstRef_Pos_void = CB_T_Pos_RET<__DataConstRef, void>;
		using __CB_ConstRef_Pos_bool = CB_T_Pos_RET<__DataConstRef, bool>;

	protected:
		__ContainerType& m_data = __Super::m_data;

	private:
		template <class T= __ContainerType>
		class __ArrayOperator
		{
		public:
			__ArrayOperator(T& data)
				: m_data(data)
			{
			}

		private:
			T& m_data;

			using __RefType = decltype(m_data[0])&;

			//template <typename CB, typename Ret>
			//using checkCBRet_1 = checkCBRet_t<CB, Ret, __RefType>;

			//template <typename CB, typename Ret>
			//using checkCBRet_2 = checkCBRet_t<CB, Ret, __RefType, size_t>;

		public:
			template <typename CB, typename = checkCBBool_t<CB, __RefType, size_t>>
			void forEach(const CB& cb, size_t startPos, size_t count)
			{
				if (startPos >= m_data.size())
				{
					return;
				}

				for (auto pos = startPos; pos < m_data.size(); pos++)
				{
					if (!cb(m_data[pos], pos))
					{
						break;
					}

					if (count > 0)
					{
						count--;
						if (0 == count)
						{
							break;
						}
					}
				}
			}

			template <typename CB, typename = checkCBVoid_t<CB, __RefType, size_t>, typename = void>
			void forEach(const CB& cb, size_t startPos, size_t count)
			{
				forEach([&](__RefType data, size_t pos) {
					cb(data, pos);
					return true;
				}, startPos, count);
			}
			
			template <typename CB, typename = checkCBVoid_t<CB, __RefType>, typename = void, typename = void>
			void forEach(const CB& cb, size_t startPos, size_t count)
			{
				forEach([&](__RefType data, size_t pos) {
					cb(data);
					return true;
				}, startPos, count);
			}

			template <typename CB, typename = checkCBBool_t<CB, __RefType>
			, typename = void, typename = void, typename = void>
			void forEach(const CB& cb, size_t startPos, size_t count)
			{
				forEach([&](__RefType data, size_t pos) {
					return cb(data);
				}, startPos, count);
			}
		};

		__ArrayOperator<> m_ArrayOperator = __ArrayOperator<>(m_data);
		__ArrayOperator<>& _getOperator()
		{
			return m_ArrayOperator;
		}

		__ArrayOperator<const __ContainerType>& _getOperator() const
		{
			return (__ArrayOperator<const __ContainerType>&)m_ArrayOperator;
		}

	public:
		SArrayT& init(size_t size, __DataConstRef data)
		{
			m_data.assign(size, data);
			return *this;
		}

		SArrayT()
		{
		}

		template<typename... args>
		explicit SArrayT(__DataConstRef data, const args&... others)
			: __Super(data, others...)
		{
		}

		explicit SArrayT(__ContainerType&& container)
		{
			__Super::swap(container);
		}

		SArrayT(SArrayT&& arr)
		{
			__Super::swap(arr);
		}

		SArrayT(const SArrayT& arr)
			: __Super(arr)
		{
		}

		explicit SArrayT(__InitList initList)
			: __Super(initList)
		{
		}

		template<typename T, typename = checkContainer_t<T>>
		explicit SArrayT(const T& container)
			: __Super(container)
		{
		}

		template<typename T, typename = checkContainer_t<T>>
		explicit SArrayT(T& container)
			: __Super(container)
		{
		}

		SArrayT& operator=(__ContainerType&& container)
		{
			__Super::swap(container);
			return *this;
		}

		SArrayT& operator=(SArrayT&& arr)
		{
			__Super::swap(arr);
			return *this;
		}

		SArrayT& operator=(const SArrayT& arr)
		{
			__Super::assign(arr);
			return *this;
		}

		SArrayT& operator=(__InitList initList)
		{
			__Super::assign(initList);
			return *this;
		}

		template <typename T>
		SArrayT& operator=(const T&t)
		{
			__Super::assign(t);
			return *this;
		}

		template <typename T>
		SArrayT& operator=(T&t)
		{
			__Super::assign(t);
			return *this;
		}

	private:
		void _add(__DataConstRef data) override
		{
			m_data.push_back(data);
		}

		size_t _del(__DataConstRef data) override
		{
			size_t uRet = 0;
			
			tagTryCompare<__DataType> comparetor;

			auto itr = m_data.begin();
			while (itr != m_data.end())
			{
				if (comparetor.compare(*itr, data))
				{
					itr = m_data.erase(itr);
					uRet++;
				}
				else
				{
					itr++;
				}
			}

			return uRet;
		}

		__ItrConstType _find(__DataConstRef data) const override
		{
			auto itr = m_data.begin();
			for (; itr != m_data.end(); itr++)
			{
				if (tagTryCompare<__DataType>().compare(*itr, data))
				{
					break;
				}
			}

			return itr;
		}

	private:
		int _checkPos(int pos) const
		{
			auto size = m_data.size();
			if (0 == size)
			{
				return -1;
			}

			if (pos < 0)
			{
				return (int)size + pos;
			}
			else
			{
				if (pos >= (int)size)
				{
					return -1;
				}

				return pos;
			}
		}

	public:
		template <typename T>
		friend SArrayT operator& (const SArrayT& lhs, const T& rhs)
		{
			SArrayT arr;
			for (auto&data : rhs)
			{
				if (lhs.includes(data))
				{
					arr.add(data);
				}
			}

			return arr;
		}

		friend SArrayT operator& (const SArrayT& lhs, __InitList rhs)
		{
			return lhs & SArrayT(rhs);
		}

	public:
		bool get(TD_PosType pos, __CB_Ref_void cb)
		{
			if (pos >= m_data.size())
			{
				return false;
			}

			cb(m_data[pos]);
			
			return true;
		}

		bool get(TD_PosType pos, __CB_ConstRef_void cb) const
		{
			if (pos >= m_data.size())
			{
				return false;
			}

			cb(m_data[pos]);

			return true;
		}

		bool set(TD_PosType pos, __DataConstRef& data)
		{
			return get([&](__DataRef m_data) {
				m_data = data;
			});
		}

		bool del_pos(TD_PosType pos)
		{
			if (pos >= m_data.size())
			{
				return false;
			}

			m_data.erase(m_data.begin() + pos);
	
			return true;
		}

		int indexOf(__DataConstRef data) const
		{
			auto itr = _find(data);
			if (itr == m_data.end())
			{
				return -1;
			}

			return itr - m_data.begin();
		}

		int lastIndexOf(__DataConstRef data) const
		{
			int uIdx = 1;
			for (auto& item : m_data)
			{
				if (tagTryCompare<__DataType>().compare(item, data))
				{
					return m_data.size()-uIdx;
				}
				uIdx++;
			}

			return -1;
		}

		template<typename CB>
		void operator() (const CB& cb, TD_PosType startPos = 0, size_t count = 0)
		{
			_getOperator().forEach(cb, startPos, count);
		}

		template<typename CB>
		void operator() (const CB& cb, TD_PosType startPos = 0, size_t count = 0) const
		{
			_getOperator().forEach(cb, startPos, count);
		}

		int find(__CB_ConstRef_Pos_bool cb, TD_PosType stratPos = 0) const
		{
			int iRetPos = -1;

			(*this)([&](__DataConstRef data, TD_PosType pos) {
				if (cb(data, pos))
				{
					iRetPos = pos;
					return false;
				}

				return true;
			});

			return iRetPos;
		}

		template<typename T>
		size_t addFront(const T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				m_data.insert(m_data.begin(), container.begin(), container.end());
			}

			return m_data.size();
		}

		size_t addFront(__InitList initList)
		{
			return addFront<__InitList>(initList);
		}

		bool popFront(__CB_ConstRef_void cb = NULL)
		{
			if (m_data.empty())
			{
				return false;
			}

			auto itr = m_data.begin();
			if (cb)
			{
				cb(*itr);
			}

			m_data.erase(itr);

			return true;
		}

		bool popFront(__DataRef data)
		{
			if (m_data.empty())
			{
				return false;
			}

			auto itr = m_data.begin();
			data = *itr;
			m_data.erase(itr);

			return true;
		}

		bool popBack(__CB_ConstRef_void cb = NULL)
		{
			if (m_data.empty())
			{
				return false;
			}

			if (cb)
			{
				cb(m_data.back());
			}

			m_data.pop_back();

			return true;
		}

		bool popBack(__DataRef data)
		{
			if (m_data.empty())
			{
				return false;
			}

			data = m_data.back();
			m_data.pop_back();

			return true;
		}

		template<typename... args>
		size_t addFront(__DataConstRef data, const args&... others)
		{
			return tagDynamicArgsExtractor<const __DataType>::extractReverse([&](__DataConstRef data) {
				m_data.insert(m_data.begin(), data);
				return true;
			}, data, others...);

			return __Super::size();
		}
		
		SArrayT slice(int startPos) const
		{
			SArrayT arr;

			startPos = _checkPos(startPos);
			if (startPos >= 0)
			{
				forEach([&](__DataConstRef data) {
					arr.add(data);
				}, (TD_PosType)startPos);
			}

			return arr;
		}

		SArrayT slice(int startPos, int endPos) const
		{
			SArrayT arr;

			startPos = _checkPos(startPos);
			endPos = _checkPos(endPos);

			if (startPos >= 0 && endPos >= 0 && startPos <= endPos)
			{
				(*this)([&](__DataConstRef data) {
					arr.add(data);
				}, (TD_PosType)startPos, size_t(endPos - startPos + 1));
			}

			return arr;
		}

		template<typename... args>
		SArrayT& splice(TD_PosType pos, size_t nRemove, __DataConstRef data, const args&... others)
		{
			vector<__DataType> vecData;
			extractDataTypeArgs(vecData, data, others...);
			return splice(pos, nRemove, vecData);
		}

		template<typename T>
		SArrayT& splice(TD_PosType pos, size_t nRemove = 0, const T& container = {})
		{
			if (__Super::checkIsSelf(container))
			{
				return *this;
			}

			auto itr = m_data.end();
			if (pos < m_data.size())
			{
				itr = m_data.begin() + pos;
			}

			while (itr != m_data.end() && nRemove)
			{
				itr = m_data.erase(itr);
				nRemove--;
			}

			m_data.insert(itr, container.begin(), container.end());

			return *this;
		}

		SArrayT& splice(TD_PosType pos, size_t nRemove, __InitList initList)
		{
			return splice(pos, nRemove, initList);
		}

		SArrayT& qsort(__CB_Sort_T<__DataType> cb = NULL)
		{
			size_t size = m_data.size();
			if (size > 1)
			{
				NS_SSTL::qsort<__DataType>(&m_data.front(), size, cb);
			}

			return *this;
		}

		SArrayT& Reverse()
		{
			reverse(m_data.begin(), m_data.end());

			return *this;
		}

		string join(const string& strSplitor = ",") const
		{
			return __Super::toString(strSplitor);
		}

	public:
		template <typename T>
		SArrayT<T, __BaseType> map(CB_T_Ret<__DataConstRef, T> cb) const
		{
			SArrayT<T, __BaseType> arr;

			for (auto&data : m_data)
			{
				arr.add(cb(data));
			}
			
			return arr;
		}

		template <typename CB, typename RET = decltype(declval<CB>()(__DataType()))>
		SArrayT<RET, __BaseType> map(const CB& cb) const
		{
			return map<RET>(cb);
		}

		SArrayT filter(__CB_ConstRef_bool cb) const
		{
			SArrayT arr;

			for (auto&data : m_data)
			{
				if (cb(data))
				{
					arr.add(data);
				}
			}
			
			return arr;
		}

		SMap<__DataType, size_t> itemSum() const
		{
			SMap<__DataType, size_t> mapItemSum;

			for (auto&data : m_data)
			{
				mapItemSum[data]++;
			}

			return mapItemSum;
		}

		void sum(SMap<__DataType, size_t>& mapItemSum, SMap<size_t, SArrayT>& mapSumItem) const
		{
			mapItemSum = itemSum();

			for (auto& pr : mapItemSum)
			{
				mapSumItem[pr.second].add(pr.first);
			}
		}

		SMap<size_t, SArrayT> sumItem() const
		{
			SMap<__DataType, size_t> mapItemSum;
			SMap<size_t, SArrayT> mapSumItem;
			sum(mapItemSum, mapSumItem);
			return mapSumItem;
		}
	};
}

#endif // __SArray_H
