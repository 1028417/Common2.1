
#ifndef __SList_H
#define __SList_H

namespace NS_SSTL
{
	template<typename __DataType, template<typename...> class __BaseType>
	class SListT : public __SuperT
	{
	private:
		__UsingSuper(__SuperT);

		typedef decltype(declval<__ContainerType&>().rbegin()) __RItrType;
		typedef decltype(declval<const __ContainerType&>().rbegin()) __CRItrType;

	private:
		template <class T = __ContainerType>
		class __ListOperator
		{
		public:
			__ListOperator(T& data)
				: m_data(data)
			{
			}

		private:
			T& m_data;

		public:
			template <typename CB, typename = checkCBBool_t<CB, __DataRef>>
			void forEach(const CB& cb)
			{
				for (auto& data : m_data)
				{
					if (!cb(data))
					{
						break;
					}
				}
			}

			template <typename CB, typename = checkCBVoid_t<CB, __DataRef>, typename = void>
			void forEach(const CB& cb)
			{
				for (auto& data : m_data)
				{
					cb(data);
				}
			}
		};

		__ListOperator<> m_ListOperator = __ListOperator<>(m_data);
		__ListOperator<>& _getOperator()
		{
			return m_ListOperator;
		}
		__ListOperator<const __ContainerType>& _getOperator() const
		{
			return (__ListOperator<const __ContainerType>&)m_ListOperator;
		}

	public:
		SListT() = default;

		template<typename... args>
		explicit SListT(__DataConstRef data, const args&... others)
			: __Super(data, others...)
		{
		}

		explicit SListT(__ContainerType&& container)
		{
			__Super::swap(container);
		}

		SListT(SListT&& lst)
		{
			__Super::swap(lst);
		}

		SListT(const SListT& lst)
			: __Super(lst)
		{
		}

		explicit SListT(__InitList initList)
			: __Super(initList)
		{
		}

		template<typename T, typename = checkContainerData_t<T, __DataType>>
		explicit SListT(const T& container)
			: __Super(container)
		{
		}

		SListT& operator=(__ContainerType&& container)
		{
			__Super::swap(container);
			return *this;
		}

		SListT& operator=(SListT&& lst)
		{
			__Super::swap(lst);
			return *this;
		}

		SListT& operator=(const SListT& lst)
		{
			__Super::assign(lst);
			return *this;
		}

		SListT& operator=(__InitList initList)
		{
			__Super::assign(initList);
			return *this;
		}

		template <typename T>
		SListT& operator=(const T&t)
		{
			__Super::assign(t);
			return *this;
		}

		template <typename T>
		SListT& operator=(T&t)
		{
			__Super::assign(t);
			return *this;
		}
		
		template<typename CB>
		void operator() (const CB& cb)
		{
			_getOperator().forEach(cb);
		}

		template<typename CB>
		void operator() (const CB& cb) const
		{
			_getOperator().forEach(cb);
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
			int nIdx = m_data.size() - 1;
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

		template<typename T>
		SListT& addFront(const T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				m_data.insert(m_data.begin(), container.begin(), container.end());
			}

			return *this;
		}

		SListT& addFront(__InitList initList)
		{
			addFront<__InitList>(initList);
		}

		template<typename... args>
		SListT& addFront(__DataConstRef data, const args&... others)
		{
			(void)tagDynamicArgsExtractor<const __DataType>::extractReverse([&](__DataConstRef data) {
				m_data.push_front(data);
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

		bool del_one(__DataConstRef data, __CB_Ref_void cb = NULL)
		{
			return 0 != _del(data, [&](__DataRef data) {
				if (cb)
				{
					cb(data);
				}

				return E_DelConfirm::DC_YesAbort;
			});
		}

	public:
		SListT& sort(__CB_Sort_T<__DataType> cb = NULL)
		{
			if (cb)
			{
				m_data.sort(tagTrySort<__DataType>(cb));
			}
			else
			{
				m_data.sort();
			}

			return *this;
		}

		SListT& Reverse()
		{
			reverse(m_data.begin(), m_data.end());

			return *this;
		}

	public:
		template <typename T>
		SListT<T, __BaseType> map(CB_T_Ret<__DataConstRef, T> cb) const
		{
			SListT<T, __BaseType> lst;

			for (auto&data : m_data)
			{
				lst.add(cb(data));
			}

			return lst;
		}

		template <typename CB, typename RET = decltype(declval<CB>()(__DataType()))>
		SListT<RET, __BaseType> map(const CB& cb) const
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

		void sum(SMap<__DataType, size_t>& mapItemSum, SMap<size_t, SListT>& mapSumItem) const
		{
			mapItemSum = itemSum();

			for (auto& pr : mapItemSum)
			{
				mapSumItem[pr.second].add(pr.first);
			}
		}

		SMap<size_t, SListT> sumItem() const
		{
			SMap<__DataType, size_t> mapItemSum;
			SMap<size_t, SListT> mapSumItem;
			sum(mapItemSum, mapSumItem);
			return mapSumItem;
		}

	protected:
		void _add(__DataConstRef data) override
		{
			m_data.push_back(data);
		}

		bool _popFront(__CB_Ref_void cb = NULL) override
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

		size_t _del(__DataConstRef data, CB_Del cb) override
		{
			return NS_SSTL::del(m_data, data, cb);
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
	};
}

#endif // __SList_H
