
#ifndef __JSArray_H
#define __JSArray_H

#include "Container.h"

#include "JSMap.h"

#include <deque>

namespace NS_JSTL
{
#define __JSArraySuper ContainerT<__DataType, __BaseType<__DataType>>

	template<typename __DataType, template<typename...> class __BaseType>
	class JSArrayT : public __JSArraySuper
	{
	private:
		using __Super = __JSArraySuper;

#ifndef _MSC_VER
	protected:
		__UsingSuperType(__ContainerType);

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
		template <typename DATA> class __ArrayOperatorT
		{
		public:
			__ArrayOperatorT(DATA& data)
				: m_data(data)
			{
			}

		private:
			DATA& m_data;

		public:
			template <typename CB>
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
		};

		using __ArrayOperator = __ArrayOperatorT<__ContainerType>;
		__ArrayOperator m_ArrayOperator = __ArrayOperator(m_data);
		__ArrayOperator& _getOperator()
		{
			return m_ArrayOperator;
		}

		using __ArrayReader = __ArrayOperatorT<const __ContainerType>;
		__ArrayReader m_ArrayReader = __ArrayReader(m_data);
		__ArrayReader& _getOperator() const
		{
			return (__ArrayReader&)m_ArrayReader;
		}

	public:
		JSArrayT& init(size_t size, __DataConstRef data)
		{
			m_data.assign(size, data);
			return *this;
		}

		JSArrayT()
		{
		}

		template<typename... args>
		explicit JSArrayT(__DataConstRef data, const args&... others)
			: __Super(data, others...)
		{
		}

		explicit JSArrayT(__ContainerType&& container)
		{
			__Super::swap(container);
		}

		JSArrayT(JSArrayT&& arr)
		{
			__Super::swap(arr);
		}

		JSArrayT(const JSArrayT& arr)
			: __Super(arr)
		{
		}

		explicit JSArrayT(__InitList initList)
			: __Super(initList)
		{
		}

		template<typename T, typename = checkContainer_t<T>>
		explicit JSArrayT(const T& container)
			: __Super(container)
		{
		}

		template<typename T, typename = checkContainer_t<T>>
		explicit JSArrayT(T& container)
			: __Super(container)
		{
		}

		JSArrayT& operator=(__ContainerType&& container)
		{
			__Super::swap(container);
			return *this;
		}

		JSArrayT& operator=(JSArrayT&& arr)
		{
			__Super::swap(arr);
			return *this;
		}

		JSArrayT& operator=(JSArrayT& arr)
		{
			__Super::assign(arr);
			return *this;
		}

		JSArrayT& operator=(__InitList initList)
		{
			__Super::assign(initList);
			return *this;
		}

		template <typename T>
		JSArrayT& operator=(const T&t)
		{
			__Super::assign(t);
			return *this;
		}

		template <typename T>
		JSArrayT& operator=(T&t)
		{
			__Super::assign(t);
			return *this;
		}

	protected:
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

		bool _includes(__DataConstRef data) const override
		{
			return indexOf(data) >= 0;
		}

	private:
		int _checkPos(int pos) const
		{
			auto size = __Super::size();
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
		JSArrayT& operator+= (const T& rhs)
		{
			__Super::add(rhs);
			return *this;
		}

		JSArrayT& operator+= (__InitList rhs)
		{
			__Super::add(rhs);
			return *this;
		}

		template <typename T>
		JSArrayT& operator-= (const T& rhs)
		{
			__Super::del(rhs);
			return *this;
		}

		JSArrayT& operator-= (__InitList rhs)
		{
			__Super::del(rhs);
			return *this;
		}

		friend JSArrayT operator& (const JSArrayT& lhs, const JSArrayT& rhs)
		{
			JSArrayT arr;
			for (auto&data : lhs)
			{
				if (rhs.includes(data))
				{
					arr.add(data);
				}
			}

			return arr;
		}

		template <typename T>
		friend JSArrayT operator& (const T& lhs, const JSArrayT& rhs)
		{
			return JSArrayT(lhs) & rhs;
		}
		
		template <typename T>
		friend JSArrayT operator& (const JSArrayT& lhs, const T& rhs)
		{
			return lhs & JSArrayT(rhs);
		}

		friend JSArrayT operator+ (const JSArrayT& lhs, const JSArrayT& rhs)
		{
			JSArrayT arr(lhs);
			arr += rhs;
			return arr;
		}

		template <typename T>
		friend JSArrayT operator+ (const JSArrayT& lhs, const T& rhs)
		{
			return lhs + JSArrayT(rhs);
		}

		template <typename T>
		friend JSArrayT operator+ (const T& lhs, const JSArrayT& rhs)
		{
			return JSArrayT(lhs) + rhs;
		}

		friend JSArrayT operator- (const JSArrayT& lhs, const JSArrayT& rhs)
		{
			JSArrayT arr(lhs);
			arr -= rhs;
			return arr;
		}

		template <typename T>
		friend JSArrayT operator- (const JSArrayT& lhs, const T& rhs)
		{
			return lhs - JSArrayT(rhs);
		}

		template <typename T>
		friend JSArrayT operator- (const T& lhs, const JSArrayT& rhs)
		{
			return JSArrayT(lhs) - rhs;
		}

		bool get(TD_PosType pos, __CB_Ref_void cb)
		{
			if (pos >= m_data.size())
			{
				return false;
			}

			if (cb)
			{
				cb(m_data[pos]);
			}

			return true;
		}

		bool get(TD_PosType pos, __CB_ConstRef_void cb) const
		{
			if (pos >= m_data.size())
			{
				return false;
			}

			if (cb)
			{
				cb(m_data[pos]);
			}

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
			int uIdx = 0;
			for (auto& item : m_data)
			{
				if (tagTryCompare<__DataType>().compare(item, data))
				{
					return uIdx;
				}
				uIdx++;
			}

			return -1;
		}

		bool pop(__CB_ConstRef_void cb = NULL)
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

		bool pop(__DataRef data)
		{
			if (m_data.empty())
			{
				return false;
			}

			data = m_data.back();
			m_data.pop_back();

			return true;
		}

		bool shift(__CB_ConstRef_void cb = NULL)
		{
			if (m_data.empty())
			{
				return false;
			}

			if (cb)
			{
				cb(m_data.front());
			}

			m_data.pop_front();

			return true;
		}

		bool shift(__DataRef data)
		{
			if (m_data.empty())
			{
				return false;
			}

			data = m_data.front();
			m_data.pop_front();

			return true;
		}

		template<typename... args>
		size_t unshift(__DataConstRef data, const args&... others)
		{
			return tagDynamicArgsExtractor<const __DataType>::extractReverse([&](__DataConstRef data) {
				m_data.insert(m_data.begin(), data);
				return true;
			}, data, others...);

			return __Super::size();
		}

		template<typename T>
		size_t unshift(const T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				m_data.insert(m_data.begin(), container.begin(), container.end());
			}

			return m_data.size();
		}

		size_t unshift(__InitList initList)
		{
			return unshift<__InitList>(initList);
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

		void forEach_if(__CB_Ref_Pos_bool cb, TD_PosType startPos = 0, size_t count = 0)
		{
			_getOperator().forEach(cb, startPos, count);
		}

		void forEach_if(__CB_ConstRef_Pos_bool cb, TD_PosType startPos = 0, size_t count = 0) const
		{
			_getOperator().forEach(cb, startPos, count);
		}

		void forEach(__CB_Ref_Pos_void cb, TD_PosType startPos = 0, size_t count = 0)
		{
			_getOperator().forEach([&](__DataRef data, TD_PosType pos) {
				cb(data, pos);
				return true;
			}, startPos, count);
		}

		void forEach(__CB_ConstRef_Pos_void cb, TD_PosType startPos = 0, size_t count = 0) const
		{
			_getOperator().forEach([&](__DataConstRef data, TD_PosType pos) {
				cb(data, pos);
				return true;
			}, startPos, count);
		}

		void forEach_if(__CB_Ref_bool cb, TD_PosType startPos = 0, size_t count = 0)
		{
			_getOperator().forEach([&](__DataRef data, TD_PosType pos) {
				return cb(data);
			}, startPos, count);
		}

		void forEach_if(__CB_ConstRef_bool cb, TD_PosType startPos = 0, size_t count = 0) const
		{
			_getOperator().forEach([&](__DataConstRef data, TD_PosType pos) {
				return cb(data);
			}, startPos, count);
		}
		
		void forEach(__CB_Ref_void cb, TD_PosType startPos = 0, size_t count = 0)
		{
			_getOperator().forEach([&](__DataRef data, TD_PosType pos) {
				cb(data);
				return true;
			}, startPos, count);
		}

		void forEach(__CB_ConstRef_void cb, TD_PosType startPos = 0, size_t count = 0) const
		{
			_getOperator().forEach([&](__DataConstRef data, TD_PosType pos) {
				cb(data);
				return true;
			}, startPos, count);
		}

		int find(__CB_ConstRef_Pos_bool cb, TD_PosType stratPos = 0) const
		{
			int iRetPos = -1;

			forEach_if([&](__DataConstRef data, TD_PosType pos) {
				if (cb(data, pos))
				{
					iRetPos = pos;
					return false;
				}

				return true;
			});

			return iRetPos;
		}

		template<typename... args>
		JSArrayT concat(__DataConstRef data, const args&... others) const
		{
			JSArrayT arr(*this);
			arr.add(data, others...);
			return arr;
		}

		template<typename T>
		JSArrayT concat(const T& container) const
		{
			JSArrayT arr(*this);
			arr.add(container);
			return arr;
		}

		JSArrayT concat(__InitList initList) const
		{
			JSArrayT arr(*this);
			arr.add(initList);
			return arr;
		}

		JSArrayT slice(int startPos) const
		{
			JSArrayT arr;

			startPos = _checkPos(startPos);
			if (startPos >= 0)
			{
				forEach([&](__DataConstRef data) {
					arr.add(data);
				}, (TD_PosType)startPos);
			}

			return arr;
		}

		JSArrayT slice(int startPos, int endPos) const
		{
			JSArrayT arr;

			startPos = _checkPos(startPos);
			endPos = _checkPos(endPos);

			if (startPos >= 0 && endPos >= 0 && startPos <= endPos)
			{
				forEach([&](__DataConstRef data) {
					arr.add(data);
				}, (TD_PosType)startPos, size_t(endPos - startPos + 1));
			}

			return arr;
		}

		template<typename... args>
		JSArrayT& splice(TD_PosType pos, size_t nRemove, __DataConstRef data, const args&... others)
		{
			vector<__DataType> vecData;
			extractDataTypeArgs(vecData, data, others...);
			return splice(pos, nRemove, vecData);
		}

		template<typename T>
		JSArrayT& splice(TD_PosType pos, size_t nRemove = 0, const T& container = {})
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

		JSArrayT& splice(TD_PosType pos, size_t nRemove, __InitList initList)
		{
			return splice(pos, nRemove, initList);
		}

		JSArrayT& qsort(__CB_Sort_T<__DataType> cb = NULL)
		{
			size_t size = m_data.size();
			if (size > 1)
			{
				NS_JSTL::qsort<__DataType>(&m_data.front(), size, cb);
			}

			return *this;
		}

		JSArrayT& reverse()
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
		JSArrayT<T, __BaseType> map(CB_T_Ret<__DataConstRef, T> cb) const
		{
			JSArrayT<T, __BaseType> arr;

			if (cb)
			{
				for (auto&data : m_data)
				{
					arr.add(cb(data));
				}
			}

			return arr;
		}

		template <typename CB, typename RET = decltype(declval<CB>()(__DataType()))>
		JSArrayT<RET, __BaseType> map(const CB& cb) const
		{
			return map<RET>(cb);
		}

		JSArrayT filter(__CB_ConstRef_bool cb) const
		{
			JSArrayT arr;

			if (cb)
			{
				for (auto&data : m_data)
				{
					if (cb(data))
					{
						arr.add(data);
					}
				}
			}

			return arr;
		}

		JSMap<__DataType, size_t> itemSum() const
		{
			JSMap<__DataType, size_t> mapItemSum;

			for (auto&data : m_data)
			{
				mapItemSum[data]++;
			}

			return mapItemSum;
		}

		void sum(JSMap<__DataType, size_t>& mapItemSum, JSMap<size_t, JSArrayT>& mapSumItem) const
		{
			mapItemSum = itemSum();

			for (auto& pr : mapItemSum)
			{
				mapSumItem[pr.second].add(pr.first);
			}
		}

		JSMap<size_t, JSArrayT> sumItem() const
		{
			JSMap<__DataType, size_t> mapItemSum;
			JSMap<size_t, JSArrayT> mapSumItem;
			sum(mapItemSum, mapSumItem);
			return mapSumItem;
		}
	};

	template <typename __DataType>
	using Vector = JSArrayT<__DataType, vector>;

	template <typename __DataType>
	using Deque = JSArrayT<__DataType, deque>;
}

#endif // __JSArray_H
