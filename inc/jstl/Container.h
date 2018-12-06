
#ifndef __ContainerType_H
#define __ContainerType_H

#include <sstream>

#include <list>

namespace NS_JSTL
{
	enum class E_DelConfirm
	{
		DC_Yes
		, DC_No
		, DC_Abort
		, DC_YesAbort
	};

	template<typename __DataType__, typename __ContainerType__, typename __KeyType = __DataType__>
	class ContainerT
	{		
	protected:
		using __DataType = __DataType__;

		using __ContainerType = __ContainerType__;
		
		__ContainerType m_data;

		using __InitList = InitList_T<__DataType>;
		using __InitList_Key = InitList_T<__KeyType>;

		using __DataRef = __DataType&;
		using __DataConstRef = const __DataType&;
		using __KeyConstRef = const __KeyType&;

		using __CB_Ref_void = CB_T_void<__DataRef>;
		using __CB_Ref_bool = CB_T_bool<__DataRef>;
		using __CB_Ref_Pos = CB_T_Pos<__DataRef>;

		using __CB_ConstRef_void = CB_T_void<__DataConstRef>;
		using __CB_ConstRef_bool = CB_T_bool<__DataConstRef>;
		using __CB_ConstRef_Pos = CB_T_Pos<__DataConstRef>;

		using __CB_Ref_DelConfirm = CB_T_Ret<__DataRef, E_DelConfirm>;

	public:
		ContainerT()
		{
		}

		template<typename... args>
		explicit ContainerT(__DataConstRef data, const args&... others)
		{
			add(data, others...);
		}

		explicit ContainerT(__ContainerType&& container)
		{
			m_data.swap(container);
		}

		ContainerT(ContainerT&& container)
		{
			swap(container);
		}

		ContainerT(const ContainerT& container)
			: m_data(container.begin(), container.end())
		{
		}

		explicit ContainerT(__InitList initList)
			: m_data(initList.begin(), initList.end())
		{
		}

		template<typename T, typename = checkContainer_t<T>>
		explicit ContainerT(const T& container)
			: m_data(container.begin(), container.end())
		{
		}

		template<typename T, typename = checkContainer_t<T>>
		explicit ContainerT(T& container)
			: m_data(container.begin(), container.end())
		{
		}

		ContainerT& operator=(ContainerT&& container)
		{
			swap(container);
			return *this;
		}

		ContainerT& operator=(const ContainerT& container)
		{
			assign(container);
			return *this;
		}

		ContainerT& operator=(__InitList initList)
		{
			assign(initList);
			return *this;
		}

		template <typename T>
		ContainerT& operator=(const T&t)
		{
			assign(t);
			return *this;
		}

		template <typename T>
		ContainerT& operator=(T&t)
		{
			assign(t);
			return *this;
		}

	public:
		ContainerT& swap(ContainerT& container)
		{
			_swap(container.m_data);
			return *this;
		}

		ContainerT& swap(__ContainerType& container)
		{
			_swap(container);
			return *this;
		}

		template<typename... args>
		ContainerT& assign(__DataConstRef data, const args&... others)
		{
			clear();

			add(data, others...);

			return *this;
		}

		ContainerT assign(__ContainerType&& container)
		{
			m_data.swap(container);
		}

		ContainerT& assign(ContainerT&& container)
		{
			return swap(container);
		}

		ContainerT& assign(__InitList initList)
		{
			return assign<__InitList>(initList);
		}

		template<typename T, typename = checkContainer_t<T>>
		ContainerT& assign(const T& container)
		{
			if (checkIsSelf(container))
			{
				return *this;
			}

			//m_data = __ContainerType(container.begin(), container.end());

			//m_data.clear();
			//for (auto&data : container)
			//{
			//	_add(data);
			//}

			m_data.clear();
			new (&m_data) __ContainerType(container.begin(), container.end());

			return *this;
		}

		void add(__DataConstRef data)
		{
			_add(data);
		}

		template<typename... args>
		void add(__DataConstRef data, const args&... others)
		{
			(void)extractDataTypeArgs([&](__DataConstRef data) {
				_add(data);
				return true;
			}, data, others...);
		}

		void add(__InitList initList)
		{
			add<__InitList>(initList);
		}

		template<typename T, typename = checkContainer_t<T>>
		void add(const T& container)
		{
			if (checkIsSelf(container))
			{
				return;
			}

			for (auto&data : container)
			{
				_add(data);
			}
		}

	public:
		operator bool() const
		{
			return !m_data.empty();
		}

		template <typename T, typename = checkIter_t<T>>
		T erase(const T& itr)
		{
			return m_data.erase(itr);
		}

		void clear()
		{
			m_data.clear();
		}

		size_t size() const
		{
			return m_data.size();
		}

		decltype(m_data.begin()) begin()
		{
			return m_data.begin();
		}
		decltype(m_data.end()) end()
		{
			return m_data.end();
		}

		decltype(m_data.cbegin()) begin() const
		{
			return m_data.cbegin();
		}
		decltype(m_data.cbegin()) end() const
		{
			return m_data.cend();
		}

		bool getFront(__CB_Ref_void cb)
		{
			return _getOperator().getFront(cb);
		}

		bool getFront(__CB_ConstRef_void cb) const
		{
			return _getOperator().getFront(cb);
		}

		bool getFront(__DataRef data) const
		{
			return _getOperator().getFront([&](__DataConstRef _data) {
				data = _data;
			});
		}

		bool getBack(__CB_Ref_void cb)
		{
			return _getOperator().getBack(cb);
		}

		bool getBack(__CB_ConstRef_void cb) const
		{
			return _getOperator().getBack(cb);
		}

		bool getBack(__DataRef data) const
		{
			return _getOperator().getBack([&](__DataConstRef _data) {
				data = _data;
			});
		}

		bool popFront(__CB_ConstRef_void cb = NULL)
		{
			auto itr = m_data.begin();
			if (itr == m_data.end())
			{
				return false;
			}

			if (cb)
			{
				cb(*itr);
			}
			m_data.erase(itr);

			return true;
		}

		bool popFront(__DataRef data)
		{
			auto itr = m_data.begin();
			if (itr == m_data.end())
			{
				return false;
			}

			data = *itr;
			m_data.erase(itr);

			return true;
		}

		template<typename... args>
		bool includes(__DataConstRef data, const args&... others) const
		{
			bool bRet = true;

			(void)extractDataTypeArgs([&](__DataConstRef data) {
				return bRet = _includes(data);
			}, data, others...);

			return bRet;
		}

		template<typename T, typename = checkContainer_t<T>>
		bool includes(const T& container) const
		{
			if (checkIsSelf(container))
			{
				return true;
			}

			for (auto&data : container)
			{
				if (!_includes(data))
				{
					return false;
				}
			}

			return true;
		}

		bool includes(__InitList initList) const
		{
			return includes<__InitList>(initList);
		}

		template<typename... args>
		vector<__KeyType> getInner(__KeyConstRef key, const args&... others) const
		{
			vector<__KeyType> vec;
			extractKeyTypeArgs(vec, key, others...);
			return getInner(vec);
		}

		template <typename T>
		T getInner(const T& container) const
		{
			T ret(container);

			if (!m_data.empty())
			{
				for (auto itr = ret.begin(); itr != ret.end(); )
				{
					if (!_includes(*itr))
					{
						itr = ret.erase(itr);
					}
					else
					{
						itr++;
					}
				}
			}

			return ret;
		}

		vector<__KeyType> getInner(__InitList_Key keys) const
		{
			vector<__KeyType> vec(keys);
			return getInner(vec);
		}

		template<typename... args>
		vector<__KeyType> getOuter(__KeyConstRef key, const args&... others) const
		{
			vector<__KeyType> vec;
			extractKeyTypeArgs(vec, key, others...);
			return getOuter(vec);
		}

		template <typename T>
		T getOuter(const T& container) const
		{
			T ret(container);

			if (!m_data.empty())
			{
				for (auto itr = ret.begin(); itr != ret.end(); )
				{
					if (_includes(*itr))
					{
						itr = ret.erase(itr);
					}
					else
					{
						itr++;
					}
				}
			}

			return ret;
		}

		vector<__KeyType> getOuter(__InitList_Key keys) const
		{
			vector<__KeyType> vec(keys);
			return getOuter(vec);
		}

		template<typename... args>
		size_t del(__KeyConstRef key, const args&... others)
		{
			size_t uRet = 0;

			(void)extractKeyTypeArgs([&](__KeyConstRef key) {
				uRet += _del(key);
				return true;
			}, key, others...);

			return uRet;
		}

		template<typename T, typename = checkContainer_t<T>>
		size_t del(const T& container)
		{
			if (checkIsSelf(container))
			{
				size_t uRet = this->size();
				this->clear();
				return uRet;
			}

			size_t uRet = 0;

			for (auto&data : container)
			{
				if (m_data.empty())
				{
					break;
				}

				uRet += _del(data);
			}

			return uRet;
		}

		size_t del(__InitList_Key keys)
		{
			return del<__InitList_Key>(keys);
		}

		size_t del_if(__CB_Ref_DelConfirm cb)
		{
			if (!cb)
			{
				return 0;
			}

			size_t uRet = 0;

			for (auto itr = m_data.begin(); itr != m_data.end();)
			{
				auto eRet = cb(*itr);
				
				if (E_DelConfirm::DC_No == eRet)
				{
					itr++;
				}
				else
				{
					if (E_DelConfirm::DC_Yes == eRet || E_DelConfirm::DC_YesAbort == eRet)
					{
						itr = m_data.erase(itr);
						uRet++;
					}

					if (E_DelConfirm::DC_Abort == eRet || E_DelConfirm::DC_YesAbort == eRet)
					{
						break;
					}
				}
			}

			return uRet;
		}

		string toString(const string& strSplitor = ",") const
		{
			stringstream ss;
			ss << '[';
			for (auto itr = m_data.begin(); itr != m_data.end(); itr++)
			{
				if (itr != m_data.begin())
				{
					ss << strSplitor.c_str();
				}

				_toString(ss, *itr);
			}

			ss << ']';
			return ss.str();
		}

		bool every(__CB_ConstRef_bool cb) const
		{
			if (!cb)
			{
				return false;
			}

			for (auto&data : m_data)
			{
				if (!cb(data))
				{
					return false;
				}
			}

			return true;
		}

		bool some(__CB_ConstRef_bool cb) const
		{
			if (!cb)
			{
				return false;
			}

			for (auto&data : m_data)
			{
				if (cb(data))
				{
					return true;
				}
			}

			return false;
		}

		__DataType reduce(const function<__DataType(__DataConstRef, __DataConstRef)>& cb) const
		{
			return NS_JSTL::reduce<__DataType, __ContainerType >(m_data, cb);
		}

	protected:
		virtual void _add(__DataConstRef data)
		{
			m_data.insert(m_data.end(), data);
		}

		virtual size_t _del(__KeyConstRef key)
		{
			return 0;
		}

		virtual bool _includes(__KeyConstRef key) const
		{
			return false;
		}

		virtual void _toString(stringstream& ss, __DataConstRef data) const
		{
			tagSSTryLMove(ss) << data;
		}

		template<typename T>
		bool checkIsSelf(const T& container) const
		{
			if (is_same<T, __ContainerType>::value)
			{
				return (__ContainerType*)&container == &m_data;
			}

			return (ContainerT*)&container == this;
		}

		template<typename... args>
		static bool extractDataTypeArgs(__CB_Ref_bool cb, __DataRef data, args&... others)
		{
			return tagDynamicArgsExtractor<__DataType>::extract([&](__DataRef data) {
				return cb(data);
			}, data, others...);
		}

		template<typename... args>
		static bool extractDataTypeArgs(__CB_ConstRef_bool cb, __DataConstRef data, const args&... others)
		{
			return tagDynamicArgsExtractor<const __DataType>::extract([&](__DataConstRef data) {
				return cb(data);
			}, data, others...);
		}

		template<typename... args>
		static void extractDataTypeArgs(vector<__DataType>& vecArgs, __DataConstRef data, const args&... others)
		{
			extractDataTypeArgs([&](__DataConstRef data) {
				vecArgs.push_back(data);
				return true;
			}, data, others...);
		}

		template<typename... args>
		static bool extractKeyTypeArgs(CB_T_bool<__KeyConstRef> cb, __KeyConstRef key, const args&... others)
		{
			return tagDynamicArgsExtractor<const __KeyType>::extract([&](__KeyConstRef key) {
				return cb(key);
			}, key, others...);
		}

		template<typename... args>
		static void extractKeyTypeArgs(vector<__KeyType>& vecArgs, __KeyConstRef key, const args&... others)
		{
			extractKeyTypeArgs([&](__KeyConstRef key) {
				vecArgs.push_back(key);
				return true;
			}, key, others...);
		}

	private:
		void _swap(__ContainerType& container)
		{
			if (&container != &m_data)
			{
				m_data.swap(container);
			}
		}

	private:
		template <typename DATA> class __ContainerOperatorT
		{
		public:
			__ContainerOperatorT(DATA& data)
				: m_data(data)
			{
			}

		private:
			DATA& m_data;

		public:
			template <typename CB>
			bool getFront(const CB& cb)
			{
				auto itr = m_data.begin();
				if (itr == m_data.end())
				{
					return false;
				}

				if (cb)
				{
					cb(*itr);
				}

				return true;
			}

			template <typename CB>
			bool getBack(const CB& cb)
			{
				auto itr = m_data.rbegin();
				if (itr == m_data.rend())
				{
					return false;
				}

				if (cb)
				{
					cb(*itr);
				}

				return true;
			}
		};

		using __ContainerOperator = __ContainerOperatorT<__ContainerType>;
		__ContainerOperator m_ContainerOperator = __ContainerOperator(m_data);
		__ContainerOperator& _getOperator()
		{
			return m_ContainerOperator;
		}

		using __ContainerReader = __ContainerOperatorT<const __ContainerType>;
		__ContainerReader m_ContainerReader = __ContainerReader(m_data);
		__ContainerReader& _getOperator() const
		{
			return (__ContainerReader&)m_ContainerReader;
		}
	};

	template <template<typename...> typename __BaseType, class __DataType>
	using Container = ContainerT<__DataType, __BaseType<__DataType>>;

	template <class __DataType>
	using ListT = Container<list, __DataType>;
}

#endif //__ContainerType_H
