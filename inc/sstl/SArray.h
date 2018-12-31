
#ifndef __SArray_H
#define __SArray_H

#include <algorithm>

namespace NS_SSTL
{
	template<typename __DataType, template<typename...> class __BaseType>
	class SArrayT : public __SuperT
	{
	protected:
		__UsingSuper(__SuperT);

		typedef decltype(declval<__ContainerType&>().rbegin()) __RItrType;
		typedef decltype(declval<const __ContainerType&>().rbegin()) __CRItrType;

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

		public:
			template <typename CB>
			bool get(TD_PosType pos, CB cb)
			{
				if (pos >= m_data.size())
				{
					return false;
				}

				cb(m_data[pos]);

				return true;
			}

			template <typename CB, typename = checkCBBool_t<CB, __RefType, TD_PosType>>
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

			template <typename CB, typename = checkCBVoid_t<CB, __RefType, TD_PosType>, typename = void>
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
		SArrayT() = default;

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

		template<typename T, typename = checkContainerData_t<T, __DataType>>
		explicit SArrayT(const T& container)
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

	public:
		__RItrType rbegin()
		{
			return m_data.rbegin();
		}
		__CRItrType rbegin() const
		{
			return m_data.rbegin();
		}

		__RItrType rend()
		{
			return m_data.rend();
		}
		__CRItrType rend() const
		{
			return m_data.rend();
		}

		bool get(TD_PosType pos, __CB_Ref_void cb)
		{
			return _getOperator().get(pos, cb);
		}
		bool get(TD_PosType pos, __CB_ConstRef_void cb) const
		{
			return _getOperator().get(pos, cb);
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

			__Super::erase(m_data.begin() + pos);

			return true;
		}

		bool del_pos(list<TD_PosType> lstPos)
		{
			for (int iIdx = m_data.size() - 1; iIdx >= 0; iIdx--)
			{
				auto itr = std::find(lstPos.begin(), lstPos.end(), (TD_PosType)iIdx);
				if (itr != lstPos.end())
				{
					__Super::erase(m_data.begin() + iIdx);

					(void)lstPos.erase(itr);

					if (lstPos.empty())
					{
						break;
					}
				}
			}

			return true;
		}

		int indexOf(__DataConstRef data) const
		{
			int nIdx = 0;
			for (auto& item : m_data)
			{
				if (tagTryCompare<__DataType>::compare(item, data))
				{
					return nIdx;
				}
				nIdx++;
			}

			return -1;
		}

		int lastIndexOf(__DataConstRef data) const
		{
			int nIdx = m_data.size()-1;
			auto itrEnd = m_data.rend();
			for (auto itr = m_data.rbegin(); itr != itrEnd; ++itr, nIdx--)
			{
				if (tagTryCompare<const __DataType>::compare(*itr, data))
				{
					return nIdx;
				}
			}

			return -1;
		}

		int find(CB_T_Pos_RET<__DataConstRef, bool> cb, TD_PosType stratPos = 0) const
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
		SArrayT& addFront(const T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				m_data.insert(m_data.begin(), container.begin(), container.end());
			}

			return *this;
		}

		SArrayT& addFront(__InitList initList)
		{
			return addFront<__InitList>(initList);
		}

		template<typename... args>
		SArrayT& addFront(__DataConstRef data, const args&... others)
		{
			(void)tagDynamicArgsExtractor<const __DataType>::extractReverse([&](__DataConstRef data) {
				m_data.insert(m_data.begin(), data);
				return true;
			}, data, others...);

			return *this;
		}

		bool popBack(__CB_Ref_void cb = NULL)
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

	public:
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

				if (0 != nRemove)
				{
					itr = m_data.erase(itr, itr+(nRemove-1));
				}
			}

			m_data.insert(itr, container.begin(), container.end());

			return *this;
		}

		SArrayT& splice(TD_PosType pos, size_t nRemove, __InitList initList)
		{
			return splice(pos, nRemove, initList);
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

	private:
		void _add(__DataConstRef data) override
		{
			m_data.push_back(data);
		}

		size_t _del(__DataConstRef data, CB_Del cb) override
		{
			size_t uRet = 0;

			for (auto itr = m_data.begin(); itr != m_data.end(); )
			{
				if (!tagTryCompare<__DataConstRef>::compare(*itr, data))
				{
					++itr;
					continue;
				}

				E_DelConfirm eRet = cb(*itr);
				if (E_DelConfirm::DC_Abort == eRet)
				{
					break;
				}
				else if (E_DelConfirm::DC_No == eRet)
				{
					++itr;
					continue;
				}
				else
				{
					itr = __Super::erase(itr);
					uRet++;

					if (E_DelConfirm::DC_YesAbort == eRet)
					{
						break;
					}
				}
			}

			return uRet;
		}
		
		bool _includes(__DataConstRef data) const override
		{
			for (auto& t_data : m_data)
			{
				if (tagTryCompare<__DataConstRef>::compare(t_data, data))
				{
					return true;
				}
			}

			return false;
		}

	protected:
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
	};
}

#endif // __SArray_H