
#ifndef __ContainerType_H
#define __ContainerType_H

#include <sstream>

namespace NS_JSTL
{
	template <typename DATA>
	class __ContainerOperator
	{
	public:
		__ContainerOperator(DATA& data)
			: m_data(data)
		{
		}

	protected:
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

		template <typename CB>
		void forEach(const CB& cb)
		{
			if (!cb)
			{
				return;
			}

			size_t pos = 0;
			for (auto&data : m_data)
			{
				if (!cb(data, pos++))
				{
					break;
				}
			}
		}
	};


	template<typename __DataType__, typename __ContainerType__, typename __KeyType = __DataType__>
	class ContainerT
	{
	protected:
		using __DataType = __DataType__;
		using __ContainerType = __ContainerType__;

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

	protected:
		ContainerT()
			: m_ContainerOperator(m_data)
			, m_ContainerReader(m_data)
		{
		}

		template<typename T>
		explicit ContainerT(const T& container)
			: m_data(container.begin(), container.end())
			, m_ContainerOperator(m_data)
			, m_ContainerReader(m_data)
		{
		}

	protected:
		__ContainerType m_data;

		using __ContainerOperatorType = __ContainerOperator<__ContainerType>;
		__ContainerOperatorType m_ContainerOperator;
		using __ContainerReaderType = __ContainerOperator<const __ContainerType>;
		__ContainerReaderType m_ContainerReader;

		__ContainerOperatorType& getContainerOperator()
		{
			return m_ContainerOperator;
		}

		__ContainerReaderType& getContainerOperator() const
		{
			return (__ContainerReaderType&)m_ContainerReader;
		}

	protected:
		virtual TD_SizeType _add(__DataConstRef data) = 0;

		virtual TD_SizeType _del(__KeyConstRef key) = 0;

		virtual bool _includes(__KeyConstRef key) const = 0;

		virtual void _toString(stringstream& ss, __DataConstRef data) const
		{
			tagSSTryLMove(ss) << data;
		}

	protected:
		template<typename... args>
		TD_SizeType add(__DataConstRef data, const args&... others)
		{
			(void)extractDataTypeArgs([&](__DataConstRef data) {
				_add(data);
				return true;
			}, data, others...);

			return size();
		}

		template<typename T>
		decltype(checkContainer<T, TD_SizeType>()) add(const T& container)
		{
			if (checkIsSelf(container))
			{
				return size();
			}

			for (auto&data : container)
			{
				_add(data);
			}

			return size();
		}

		TD_SizeType add(__InitList initList)
		{
			return add<__InitList>(initList);
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

		template<typename T>
		bool checkIsSelf(const T& container) const
		{
			return ((void*)&container == (void*)this) || ((void*)&container == (void*)&m_data);
		}

	public:
		template<typename... args>
		ContainerT& assign(__DataConstRef data, const args&... others)
		{
			clear();

			add(data, others...);

			return *this;
		}

		template<typename T>
		ContainerT& assign(const T& container)
		{
			if (checkIsSelf(container))
			{
				return *this;
			}

			m_data = __ContainerType(container.begin(), container.end());

			return *this;
		}

		ContainerT& assign(__InitList initList)
		{
			return assign<__InitList>(initList);
		}

		ContainerT& swap(ContainerT& container)
		{
			if ((void*)this != (void*)&container)
			{
				m_data.swap(container.m_data);
			}

			return *this;
		}

	public:
		TD_SizeType size() const
		{
			return m_data.size();
		}

		TD_SizeType length() const
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
			return getContainerOperator().getFront(cb);
		}

		bool getFront(__CB_ConstRef_void cb) const
		{
			return getContainerOperator().getFront(cb);
		}

		bool getFront(__DataRef data) const
		{
			return getContainerOperator().getFront([&](__DataConstRef _data) {
				data = _data;
			});
		}

		void getBack(__CB_Ref_void cb)
		{
			getContainerOperator().getBack(cb);
		}

		void getBack(__CB_ConstRef_void cb) const
		{
			getContainerOperator().getBack(cb);
		}

		bool getBack(__DataRef data) const
		{
			return getContainerOperator().getBack([&](__DataConstRef _data) {
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

		template<typename T>
		decltype(checkContainer<T, bool>()) includes(const T& container) const
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

		template <typename T>
		T erase(T& itr)
		{
			return itr = m_data.erase(itr);
		}

		template<typename... args>
		TD_SizeType del(__KeyConstRef key, const args&... others)
		{
			TD_SizeType uRet = 0;

			(void)extractKeyTypeArgs([&](__KeyConstRef key) {
				uRet += _del(key);
				return true;
			}, key, others...);

			return uRet;
		}

		template <typename T>
		decltype(checkContainer<T, TD_SizeType>()) del(const T& container)
		{
			if (checkIsSelf(container))
			{
				TD_SizeType uRet = this->size();
				this->clear();
				return uRet;
			}

			TD_SizeType uRet = 0;

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

		TD_SizeType del(__InitList_Key keys)
		{
			return del<__InitList_Key>(keys);
		}

		TD_SizeType del(__CB_ConstRef_bool cb)
		{
			if (!cb)
			{
				return 0;
			}

			TD_SizeType uRet = 0;

			for (auto itr = m_data.begin(); itr != m_data.end();)
			{
				if (cb(*itr))
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

		void clear()
		{
			m_data.clear();
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

	public:
		void forEach(__CB_Ref_Pos cb)
		{
			getContainerOperator().forEach(cb);
		}

		void forEach(__CB_ConstRef_Pos cb) const
		{
			getContainerOperator().forEach(cb);
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
	};
}

#endif //__ContainerType_H
