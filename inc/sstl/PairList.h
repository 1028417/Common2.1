
#ifndef __PairList_H
#define __PairList_H

namespace NS_SSTL
{
	template<typename __FirstType, typename __SecondType, typename __DataType = pair<__FirstType, __SecondType>>
	class PairListT : public ArrListT<__DataType>
	{
	private:
		__UsingSuper(ArrListT<__DataType>)
		
		using __PairType = pair<__FirstType, __SecondType>;
		using __PairRef = __PairType&;
		using __PairConstRef = const __PairType&;

		using __FirstRef = __FirstType&;
		using __FirstConstRef = const __FirstType&;

		using __SecondRef = __SecondType&;
		using __SecondConstRef = const __SecondType&;

	public:
		PairListT() = default;
		
		explicit PairListT(__FirstConstRef first, __SecondConstRef second)
		{
			_add({ first, second });
		}
		
		template<typename... args>
		explicit PairListT(__DataConstRef data, const args&... others)
			: __Super(data, others...)
		{
		}

		explicit PairListT(__ContainerType&& container)
		{
			__Super::swap(container);
		}

		PairListT(PairListT&& lst)
		{
			__Super::swap(lst);
		}

		PairListT(const PairListT& lst)
			: __Super(lst)
		{
		}

		explicit PairListT(__InitList initList)
			: __Super(initList)
		{
		}

		template<typename T, typename = checkContainerData_t<T, __PairType>>
		explicit PairListT(const T& container)
			: __Super(container)
		{
		}

		PairListT& operator=(__ContainerType&& container)
		{
			__Super::swap(container);
			return *this;
		}

		PairListT& operator=(PairListT&& lst)
		{
			__Super::swap(lst);
			return *this;
		}

		PairListT& operator=(const PairListT& lst)
		{
			__Super::assign(lst);
			return *this;
		}

		PairListT& operator=(__InitList initList)
		{
			__Super::assign(initList);
			return *this;
		}

		template <typename CB>
		void operator() (int startPos, int endPos, const CB& cb)
		{
			adaptor().forEach(cb, startPos, endPos);
		}

		template <typename CB>
		void operator() (int startPos, int endPos, const CB& cb) const
		{
			adaptor().forEach(cb, startPos, endPos);
		}

		template <typename CB>
		void operator() (int startPos, const CB& cb)
		{
			adaptor().forEach(cb, startPos);
		}

		template <typename CB>
		void operator() (int startPos, const CB& cb) const
		{
			adaptor().forEach(cb, startPos);
		}

		template <typename CB>
		void operator() (const CB& cb)
		{
			adaptor().forEach(cb);
		}

		template <typename CB>
		void operator() (const CB& cb) const
		{
			adaptor().forEach(cb);
		}

	public:
		template <typename CB>
		void forFirst(const CB& cb, int startPos = 0, int endPos = -1)
		{
			adaptor().forFirst(cb, startPos, endPos);
		}

		template <typename CB>
		void forFirst(const CB& cb, int startPos = 0, int endPos = -1) const
		{
			adaptor().forFirst(cb, startPos, endPos);
		}

		template <typename CB>
		void forSecond(const CB& cb, int startPos = 0, int endPos = -1)
		{
			adaptor().forSecond(cb, startPos, endPos);
		}

		template <typename CB>
		void forSecond(const CB& cb, int startPos = 0, int endPos = -1) const
		{
			adaptor().forSecond(cb, startPos, endPos);
		}

		inline PairListT& addPair(__FirstConstRef first, __SecondConstRef second)
		{
			__Super::_add({ first, second });
		}

		bool set(size_t pos, __FirstConstRef first, __SecondConstRef second)
		{
			return __Super::get([&](__DataRef m_data) {
				m_data = { first, second };
			});
		}

	public:
		template <typename CB>
		SArray<__FirstType> firsts(const CB& cb = NULL) const
		{
			return adaptor().firsts(cb);
		}

		SArray<__FirstType> firsts() const
		{
			return adaptor().firsts([&](__FirstConstRef) {
				return true;
			});
		}

		template <typename CB>
		SArray<__SecondType> seconds(const CB& cb) const
		{
			return adaptor().seconds(cb);
		}

		SArray<__SecondType> seconds() const
		{
			return adaptor().seconds([&](__SecondConstRef) {
				return true;
			});
		}

        template <typename CB, typename RET = SMap<cbRet_t<CB, __FirstConstRef>, __SecondType> >
		RET mapFirst(const CB& cb) const
		{
			RET lst;

			for (auto& pr : m_data)
			{
				lst.addPair(cb(pr.first), pr.second);
			}

			return lst;
		}

        template <typename CB, typename RET = SMap<__FirstType, cbRet_t<CB, __SecondConstRef>> >
		RET mapSecond(const CB& cb) const
		{
			RET lst;

			for (auto& pr : m_data)
			{
				lst.addPair(pr.first, cb(pr.second));
			}

			return lst;
		}

		template <typename CB, typename = checkCBBool_t<CB, __FirstConstRef, __SecondConstRef> >
		PairListT filter(const CB& cb) const
		{
			PairListT lst;

			for (auto& pr : m_data)
			{
				if (cb(pr.first, pr.second))
				{
					lst.addPair(pr.first, pr.second);
				}
			}

			return lst;
		}

		template <typename CB, typename = checkCBBool_t<CB, __FirstConstRef> >
		PairListT filterFirst(const CB& cb) const
		{
			PairListT lst;

			for (auto& pr : m_data)
			{
				if (cb(pr.first))
				{
					lst.addPair(pr.first, pr.second);
				}
			}

			return lst;
		}

		template <typename CB, typename = checkCBBool_t<CB, __SecondConstRef> >
		PairListT filterSecond(const CB& cb) const
		{
			PairListT lst;

			for (auto& pr : m_data)
			{
				if (cb(pr.second))
				{
					lst.addPair(pr.first, pr.second);
				}
			}

			return lst;
		}

		template <typename CB, typename = checkCBBool_t<CB, __FirstConstRef> >
		bool everyFirst(const CB& cb) const
		{
			for (auto& pr : m_data)
			{
				if (!cb(pr.first))
				{
					return false;
				}
			}

			return true;
		}

		template <typename CB, typename = checkCBBool_t<CB, __SecondConstRef> >
		bool everySecond(const CB& cb) const
		{
			for (auto& pr : m_data)
			{
				if (!cb(pr.second))
				{
					return false;
				}
			}

			return true;
		}

		template <typename CB, typename = checkCBBool_t<CB, __FirstConstRef> >
		bool anyFirst(const CB& cb) const
		{
			for (auto& pr : m_data)
			{
				if (cb(pr.first))
				{
					return true;
				}
			}

			return false;
		}

		template <typename CB, typename = checkCBBool_t<CB, __SecondConstRef> >
		bool anySecond(const CB& cb) const
		{
			for (auto& pr : m_data)
			{
				if (cb(pr.second))
				{
					return true;
				}
			}

			return false;
		}

	private:
		virtual void _toString(stringstream& ss, const __PairType& pr) const override
		{
			tagSSTryLMove(ss) << '<' << pr.first << ", " << pr.second << '>';
		}

	private:
		template <class T = __ContainerType>
		class CAdaptor
		{
		public:
			CAdaptor(T& data, const PtrArray<__PairType>& ptrArray)
				: m_data(data)
				, m_ptrArray(ptrArray)
			{
			}

		private:
			T& m_data;

			const PtrArray<__PairType>& m_ptrArray;

			using __PairRef = decltype(*m_data.begin())&;
			using __FirstRef = decltype(m_data.begin()->first)&;
			using __SecondRef = decltype(m_data.begin()->second)&;

		public:
			template <typename CB, typename = checkCBBool_t<CB, __FirstConstRef, __SecondConstRef> >
			SArray<__FirstType> firsts(const CB& cb) const
			{
				SArray<__FirstType> arr;
				for (auto& pr : m_data)
				{
					if (cb(pr.first, pr.second))
					{
						arr.add(pr.first);
					}
				}

				return arr;
			}

			template <typename CB, typename = checkCBBool_t<CB, __FirstConstRef>, typename = void>
			SArray<__FirstType> firsts(const CB& cb) const
			{
				return firsts([&](__FirstConstRef first, __SecondConstRef) {
					return cb(first);
				});
			}

			template <typename CB, typename = checkCBBool_t<CB, __FirstConstRef, __SecondConstRef> >
			SArray<__SecondType> seconds(const CB& cb) const
			{
				SArray<__SecondType> arr;
				for (auto& pr : m_data)
				{
					if (cb(pr.first, pr.second))
					{
						arr.add(pr.second);
					}
				}

				return arr;
			}

			template <typename CB, typename = checkCBBool_t<CB, __SecondConstRef>, typename = void>
			SArray<__SecondType> seconds(const CB& cb) const
			{
				return seconds([&](__FirstConstRef, __SecondConstRef second) {
					return cb(second);
				});
			}

			template <typename CB, typename = checkCBBool_t<CB, __FirstRef, __SecondRef>>
			void forEach(const CB& cb, int startPos = 0, int endPos = -1) const
			{
				m_ptrArray(startPos, endPos, [&](__PairRef pr) {
					return cb(pr.first, pr.second);
				});
			}

			template <typename CB, typename = checkCBVoid_t<CB, __FirstRef, __SecondRef>, typename = void>
			void forEach(const CB& cb, int startPos = 0, int endPos = -1) const
			{
				m_ptrArray(startPos, endPos, [&](__PairRef pr) {
					cb(pr.first, pr.second);

					return true;
				});
			}

			template <typename CB, typename = checkCBBool_t<CB, __FirstRef>>
			void forFirst(const CB& cb, int startPos = 0, int endPos = 0) const
			{
				m_ptrArray([&](__PairRef pr) {
					return cb(pr.first);
				});
			}

			template <typename CB, typename = checkCBVoid_t<CB, __FirstRef>, typename = void>
			void forFirst(const CB& cb, int startPos = 0, int endPos = 0) const
			{
				m_ptrArray([&](__PairRef pr) {
					cb(pr.first);

					return true;
				});
			}

			template <typename CB, typename = checkCBBool_t<CB, __SecondRef>>
			void forSecond(const CB& cb, int startPos = 0, int endPos = 0) const
			{
				m_ptrArray([&](__PairRef pr) {
					return cb(pr.second);
				});
			}

			template <typename CB, typename = checkCBVoid_t<CB, __SecondRef>, typename = void>
			void forSecond(const CB& cb, int startPos = 0, int endPos = 0) const
			{
				m_ptrArray([&](__PairRef pr) {
					cb(pr.second);

					return true;
				});
			}
		};

		CAdaptor<> m_adaptor = CAdaptor<>(m_data, __Super::m_ptrArray);
		inline CAdaptor<>& adaptor()
		{
			return m_adaptor;
		}
		inline CAdaptor<const __ContainerType>& adaptor() const
		{
			return (CAdaptor<const __ContainerType>&)m_adaptor;
		}
	};
}

#endif // __PairList_H
