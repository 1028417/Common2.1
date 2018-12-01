
#ifndef __JSArray_H
#define __JSArray_H

#include "Container.h"

#include <algorithm>

#include "JSMap.h"

namespace NS_JSTL
{
	template <typename DATA>
	class __ArrayOperator
	{
	public:
		__ArrayOperator(DATA& data)
			: m_data(data)
		{
		}

	protected:
		DATA& m_data;

	public:
		template <typename CB>
		void forEach(const CB& cb, size_t startPos, size_t count)
		{
			if (!cb)
			{
				return;
			}

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
		__UsingSuperType(__CB_Ref_Pos);

		__UsingSuperType(__CB_ConstRef_void);
		__UsingSuperType(__CB_ConstRef_bool);
		__UsingSuperType(__CB_ConstRef_Pos);
#endif

	protected:
		using __ArrayOperatorType = __ArrayOperator<__ContainerType>;
		__ArrayOperatorType m_ArrayOperator;
		using __ContainerReaderType = __ArrayOperator<const __ContainerType>;
		__ContainerReaderType m_ArrayReader;

		__ArrayOperatorType& getArrayOperator()
		{
			return m_ArrayOperator;
		}

		__ContainerReaderType& getArrayOperator() const
		{
			return (__ContainerReaderType&)m_ArrayReader;
		}

		__ContainerType& _data()
		{
			return __Super::m_data;
		}

		const __ContainerType& _data() const
		{
			return __Super::m_data;
		}

	public:
		static JSArrayT init(TD_SizeType size, __DataConstRef data)
		{
			JSArrayT arr;
			arr.m_data.assign(size, data);
			return arr;
		}

		JSArrayT()
			: m_ArrayOperator(_data())
			, m_ArrayReader(_data())
		{
		}

		template<typename... args>
		explicit JSArrayT(__DataConstRef data, const args&... others)
			: m_ArrayOperator(_data())
			, m_ArrayReader(_data())
		{
			__Super::add(data, others...);
		}

		explicit JSArrayT(const JSArrayT& arr)
			: __Super(arr)
			, m_ArrayOperator(_data())
			, m_ArrayReader(_data())
		{
		}

		JSArrayT(JSArrayT&& arr)
			: m_ArrayOperator(_data())
			, m_ArrayReader(_data())
		{
			__Super::swap(arr);
		}

		JSArrayT(__InitList initList)
			: __Super(initList)
			, m_ArrayOperator(_data())
			, m_ArrayReader(_data())
		{
		}

		template<typename T, typename _ITR = decltype(declval<T>().begin())>
		explicit JSArrayT(const T& container)
			: __Super(container)
			, m_ArrayOperator(_data())
			, m_ArrayReader(_data())
		{
		}

		JSArrayT& operator=(const JSArrayT& arr)
		{
			__Super::assign(arr);
			return *this;
		}

		JSArrayT& operator=(JSArrayT&& arr)
		{
			__Super::swap(arr);
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

	protected:
		TD_SizeType _add(__DataConstRef data) override
		{
			_data().push_back(data);
			return _data().size();
		}

		TD_SizeType _del(__DataConstRef data) override
		{
			TD_SizeType uRet = 0;

			auto itr = _data().begin();
			while (itr != _data().end())
			{
				if (tagTryCompare<__DataType>().compare(*itr, data))
				{
					itr = _data().erase(itr);
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
			for (auto& data : lhs)
			{
				if (rhs.includes(data))
				{
					arr.push(data);
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
		
		template<typename... args>
		TD_SizeType push(__DataConstRef data, const args&... others)
		{
			return __Super::add(data, others...);
		}

		template<typename T>
		TD_SizeType push(const T& container)
		{
			return __Super::add(container);
		}

		TD_SizeType push(__InitList initList)
		{
			return __Super::add(initList);
		}

		bool get(TD_PosType pos, __CB_Ref_void cb)
		{
			if (pos >= _data().size())
			{
				return false;
			}

			if (cb)
			{
				cb(_data()[pos]);
			}

			return true;
		}

		bool get(TD_PosType pos, __CB_ConstRef_void cb) const
		{
			if (pos >= _data().size())
			{
				return false;
			}

			if (cb)
			{
				cb(_data()[pos]);
			}

			return true;
		}

		bool get(TD_PosType pos, __DataRef data)
		{
			return get([&](__DataRef _data) {
				data = _data;
			});
		}

		bool set(TD_PosType pos, __DataConstRef& data)
		{
			return get([&](__DataRef _data) {
				_data = data;
			});
		}

		int indexOf(__DataConstRef data) const
		{
			int uIdx = 0;
			for (auto& item : _data())
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
			if (_data().empty())
			{
				return false;
			}

			if (cb)
			{
				cb(_data().back());
			}

			_data().pop_back();

			return true;
		}

		bool pop(__DataRef data)
		{
			if (_data().empty())
			{
				return false;
			}

			data = _data().back();
			_data().pop_back();

			return true;
		}

		bool shift(__CB_ConstRef_void cb = NULL)
		{
			if (_data().empty())
			{
				return false;
			}

			if (cb)
			{
				cb(_data().front());
			}

			_data().pop_front();

			return true;
		}

		bool shift(__DataRef data)
		{
			if (_data().empty())
			{
				return false;
			}

			data = _data().front();
			_data().pop_front();

			return true;
		}

		template<typename... args>
		TD_SizeType unshift(__DataConstRef data, const args&... others)
		{
			return tagDynamicArgsExtractor<const __DataType>::extractReverse([&](__DataConstRef data) {
				_data().insert(_data().begin(), data);
				return true;
			}, data, others...);

			return __Super::size();
		}

		template<typename T>
		TD_SizeType unshift(const T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				_data().insert(_data().begin(), container.begin(), container.end());
			}

			return _data().size();
		}

		TD_SizeType unshift(__InitList initList)
		{
			return unshift<__InitList>(initList);
		}

		int lastIndexOf(__DataConstRef data) const
		{
			int uIdx = 1;
			for (auto& item : _data())
			{
				if (tagTryCompare<__DataType>().compare(item, data))
				{
					return _data().size()-uIdx;
				}
				uIdx++;
			}

			return -1;
		}

		void forEach(__CB_Ref_Pos cb, TD_PosType startPos = 0, TD_SizeType count = 0)
		{
			getArrayOperator().forEach(cb, startPos, count);
		}

		void forEach(__CB_ConstRef_Pos cb, TD_PosType startPos = 0, TD_SizeType count = 0) const
		{
			getArrayOperator().forEach(cb, startPos, count);
		}

		void forEach(__CB_Ref_bool cb, TD_PosType startPos = 0, TD_SizeType count = 0)
		{
			if (!cb)
			{
				return;
			}

			forEach([&](__DataRef data, TD_PosType pos) {
				return cb(data);
			}, startPos, count);
		}

		void forEach(__CB_ConstRef_bool cb, TD_PosType startPos = 0, TD_SizeType count = 0) const
		{
			if (!cb)
			{
				return;
			}

			forEach([&](__DataConstRef data, TD_PosType pos) {
				return cb(data);
			}, startPos, count);
		}

		int find(__CB_ConstRef_Pos cb, TD_PosType stratPos = 0) const
		{
			if (!cb)
			{
				return -1;
			}

			int iRet = -1;
			forEach([&](__DataConstRef data, TD_PosType pos) {
				if (cb(data, pos))
				{
					iRet = pos;
					return false;
				}

				return true;
			});

			return iRet;
		}

		template<typename... args>
		JSArrayT concat(__DataConstRef data, const args&... others) const
		{
			JSArrayT arr(*this);
			arr.push(data, others...);
			return arr;
		}

		template<typename T>
		JSArrayT concat(const T& container) const
		{
			JSArrayT arr(*this);
			arr.push(container);
			return arr;
		}

		JSArrayT concat(__InitList initList) const
		{
			JSArrayT arr(*this);
			arr.push(initList);
			return arr;
		}

		JSArrayT slice(int startPos) const
		{
			JSArrayT arr;

			startPos = _checkPos(startPos);
			if (startPos >= 0)
			{
				forEach([&](__DataConstRef data) {
					arr.push(data);
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
					arr.push(data);
					return true;
				}, (TD_PosType)startPos, TD_SizeType(endPos - startPos + 1));
			}

			return arr;
		}

		template<typename... args>
		JSArrayT& splice(TD_PosType pos, TD_SizeType nRemove, __DataConstRef data, const args&... others)
		{
			vector<__DataType> vecData;
			extractDataTypeArgs(vecData, data, others...);
			return splice(pos, nRemove, vecData);
		}

		template<typename T>
		JSArrayT& splice(TD_PosType pos, TD_SizeType nRemove = 0, const T& container = {})
		{
			if (__Super::checkIsSelf(container))
			{
				return *this;
			}

			auto itr = _data().end();
			if (pos < _data().size())
			{
				itr = _data().begin() + pos;
			}

			while (itr != _data().end() && nRemove)
			{
				itr = _data().erase(itr);
				nRemove--;
			}

			_data().insert(itr, container.begin(), container.end());

			return *this;
		}

		JSArrayT& splice(TD_PosType pos, TD_SizeType nRemove, __InitList initList)
		{
			return splice(pos, nRemove, initList);
		}

		JSArrayT& sort(__CB_Sort_T<__DataType> cb = NULL)
		{
			std::sort(_data().begin(), _data().end(), tagTrySort<__DataType>(cb));

			return *this;
		}

		JSArrayT& reverse()
		{
			std::reverse(_data().begin(), _data().end());

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
				for (auto&data : _data())
				{
					arr.push(cb(data));
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
				for (auto&data : _data())
				{
					if (cb(data))
					{
						arr.push(data);
					}
				}
			}

			return arr;
		}

		JSMap<__DataType, TD_SizeType> itemSum() const
		{
			JSMap<__DataType, TD_SizeType> mapItemSum;

			for (auto&data : _data())
			{
				mapItemSum[data]++;
			}

			return mapItemSum;
		}

		void sum(JSMap<__DataType, TD_SizeType>& mapItemSum, JSMap<TD_SizeType, JSArrayT>& mapSumItem) const
		{
			mapItemSum = itemSum();

			for (auto& pr : mapItemSum)
			{
				mapSumItem[pr.second].push(pr.first);
			}
		}

		JSMap<TD_SizeType, JSArrayT> sumItem() const
		{
			JSMap<__DataType, TD_SizeType> mapItemSum;
			JSMap<TD_SizeType, JSArrayT> mapSumItem;
			sum(mapItemSum, mapSumItem);
			return mapSumItem;
		}
	};
}

#endif // __JSArray_H
