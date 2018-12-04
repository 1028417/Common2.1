
#ifndef __JSMap_H
#define __JSMap_H

#include "Container.h"

#include <unordered_map>
#include <map>

#include "JSArray.h"
 
namespace NS_JSTL
{
#define __JSMapSuper ContainerT<pair<__KeyType const, __ValueType>, __BaseType<__KeyType, __ValueType>, __KeyType>

	template<typename __KeyType, typename __ValueType, template<typename...> typename __BaseType>
	class JSMapT
		: public __JSMapSuper
	{
	private:
		using __Super = __JSMapSuper;

#ifndef _MSC_VER
	protected:
		__UsingSuperType(__ContainerType);

		__UsingSuperType(__DataType);

		__UsingSuperType(__InitList);
		__UsingSuperType(__InitList_Key);
		
		__UsingSuperType(__KeyConstRef);
#endif

	protected:
		using __ValueRef = __ValueType&;
		using __ValueConstRef = const __ValueType&;

		//template <typename T> using __CB_KeyCR_ValueR_T = const function<T(__KeyConstRef, __ValueRef)>&;
		//using __CB_KeyCR_ValueR_bool = __CB_KeyCR_ValueR_T<bool>;
		//using __CB_KeyCR_ValueR_void = __CB_KeyCR_ValueR_T<void>;

		template <typename T>
		using __CB_KeyCR_ValueCR_T = const function<T(__KeyConstRef, __ValueConstRef)>&;
		using __CB_KeyCR_ValueCR_bool = __CB_KeyCR_ValueCR_T<bool>;
		//using __CB_KeyCR_ValueCR_void = __CB_KeyCR_ValueCR_T<void>;

		using __CB_ValueR_void = const function<void(__ValueRef)>&;
		using __CB_ValueCR_void = const function<void(__ValueConstRef)>&;

		using __CB_ValueR_bool = const function<bool(__ValueRef)>&;
		using __CB_ValueCR_bool = const function<bool(__ValueConstRef)>&;

		using __CB_ValueR_DelConfirm = const function<E_DelConfirm(__ValueRef)>&;

	private:
		__ContainerType& m_data = __Super::m_data;

	public:
		JSMapT()
		{
		}

		template <typename T>
		explicit JSMapT(const T& keys, const function<__ValueType(__KeyType)>& cb)
		{
			set(keys, cb);
		}

		explicit JSMapT(__InitList_Key keys, const function<__ValueType(__KeyType)>& cb)
		{
			set(keys, cb);
		}

		explicit JSMapT(__ContainerType&& container)
		{
			__Super::swap(container);
		}

		JSMapT(JSMapT&& map)
		{
			__Super::swap(map);
		}

		JSMapT(const JSMapT& map)
			: __Super(map)
		{
		}

		explicit JSMapT(__InitList initList)
			: __Super(initList)
		{
		}

		template<typename T, typename = checkContainer_t<T>>
		explicit JSMapT(const T& container)
			: __Super(container)
		{
		}

		template<typename T, typename = checkContainer_t<T>>
		explicit JSMapT(T& container)
			: __Super(container)
		{
		}

		JSMapT& operator=(__ContainerType&& container)
		{
			__Super::swap(container);
			return *this;
		}

		JSMapT& operator=(JSMapT&& map)
		{
			__Super::swap(map);
			return *this;
		}

		JSMapT& operator=(const JSMapT& map)
		{
			__Super::assign(map);
			return *this;
		}

		JSMapT& operator=(__InitList initList)
		{
			__Super::assign(initList);
			return *this;
		}

		template<typename T>
		JSMapT& operator=(const T&t)
		{
			__Super::assign(t);
			return *this;
		}

		template<typename T>
		JSMapT& operator=(T&t)
		{
			__Super::assign(t);
			return *this;
		}

	protected:
		void _add(const __DataType& pr) override
		{
			m_data.insert(pr);
		}

		size_t _del(__KeyConstRef key) override
		{
			auto itr = m_data.find(key);
			if (itr == m_data.end())
			{
				return 0;
			}

			m_data.erase(itr);

			return 1;
		}

		bool _includes(__KeyConstRef key) const override
		{
			return m_data.find(key) != m_data.end();
		}

		virtual void _toString(stringstream& ss, const __DataType& pr) const override
		{
			tagSSTryLMove(ss) << '<' << pr.first << ", " << pr.second << '>';
		}

	public:
		template <typename T>
		JSMapT& operator+= (const T& rhs)
		{
			__Super::add(rhs);
			return *this;
		}

		JSMapT& operator+= (__InitList rhs)
		{
			__Super::add(rhs);
			return *this;
		}

		template <typename T>
		JSMapT& operator-= (const T& keys)
		{
			__Super::del(keys);
			return *this;
		}

		JSMapT& operator-= (__InitList_Key keys)
		{
			__Super::del(keys);
			return *this;
		}

		template <typename T>
		JSMapT operator+ (const T& rhs)
		{
			JSMapT map(*this);
			map += rhs;
			return map;
		}

		template <typename T>
		JSMapT operator- (const T& rhs)
		{
			JSMapT map(*this);
			map -= rhs;
			return map;
		}

		__ValueType& get(__KeyConstRef key)
		{
			return m_data[key];
		}

		bool get(__KeyConstRef key, __CB_ValueR_void cb)
		{
			auto itr = m_data.find(key);
			if (itr == m_data.end())
			{
				return false;
			}

			if (cb)
			{
				cb(itr->second);
			}

			return true;
		}

		bool get(__KeyConstRef key, __CB_ValueCR_void cb) const
		{
			auto itr = m_data.find(key);
			if (itr == m_data.end())
			{
				return false;
			}

			if (cb)
			{
				cb(itr->second);
			}

			return true;
		}

		bool del_if(__KeyConstRef key, __CB_ValueR_void cb=NULL)
		{
			auto itr = m_data.find(key);
			if (itr == m_data.end())
			{
				return false;
			}

			if (cb)
			{
				cb(itr->second);
			}

			m_data.erase(itr);

			return true;
		}

		size_t del_if(__CB_ValueR_DelConfirm cb)
		{
			if (!cb)
			{
				return 0;
			}

			return __Super::del_if([&](__DataType& data) {
				return cb(data.second);
			});
		}

		void forEachValue(__CB_ValueR_bool cb)
		{
			if (!cb)
			{
				return;
			}

			for (auto& pr : m_data)
			{
				if (!cb(pr.second))
				{
					break;
				}
			}
		}

		void forEachValue(__CB_ValueCR_bool cb) const
		{
			if (!cb)
			{
				return;
			}

			for (auto& pr : m_data)
			{
				if (!cb(pr.second))
				{
					break;
				}
			}
		}

		JSArray<__KeyType> keys(__CB_KeyCR_ValueCR_bool cb = NULL) const
		{
			JSArray<__KeyType> arr;
			for (auto& pr : m_data)
			{
				if (cb)
				{
					if (!cb(pr.first, pr.second))
					{
						continue;
					}
				}

				arr.add(pr.first);
			}

			return arr;
		}

		JSArray<__ValueType> values(__CB_KeyCR_ValueCR_bool cb = NULL) const
		{
			JSArray<__ValueType> arr;
			for (auto& pr : m_data)
			{
				if (cb)
				{
					if (!cb(pr.first, pr.second))
					{
						continue;
					}
				}

				arr.add(pr.second);
			}

			return arr;
		}

		__ValueType& set(__KeyConstRef key, __ValueConstRef value)
		{
			return m_data[key] = value;
		}

		template<typename T>
		void set(const T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				m_data.insert(container.begin(), container.end());
			}
		}

		void set(__InitList initList)
		{
			set<__InitList>(initList);
		}

		template <typename T>
		void set(const T& container, const function<__ValueType(__KeyType)>& cb)
		{
			if (cb)
			{
				for (auto& key : container)
				{
					this->set(key, cb(key));
				}
			}
		}

		void set(__InitList_Key keys, const function<__ValueType(__KeyType)>& cb)
		{
			set<__InitList_Key>(keys, cb);
		}

	public:
		template <typename T>
		JSMapT<__KeyType, T, __BaseType> map(__CB_KeyCR_ValueCR_T<T> cb) const
		{
			JSMapT<__KeyType, T, __BaseType> map;

			if (cb)
			{
				for (auto& pr : m_data)
				{
					map.set(pr.first, cb(pr.first, pr.second));
				}
			}

			return map;
		}

		template <typename CB, typename RET = decltype(declval<CB>()(__KeyType(), __ValueType()))>
		JSMapT<__KeyType, RET, __BaseType> map(const CB& cb) const
		{
			return map<RET>(cb);
		}

		JSMapT filter(__CB_KeyCR_ValueCR_bool cb) const
		{
			JSMapT map;

			if (cb)
			{
				for (auto& pr : m_data)
				{
					if (cb(pr.first, pr.second))
					{
						map.set(pr.first, pr.second);
					}
				}
			}

			return map;
		}
	};

	template <typename __KeyType, typename __ValueType>
	using JSMap = JSMapT<__KeyType, __ValueType, map>;

	template <typename __KeyType, typename __ValueType>
	using JSUnsortMap = JSMapT<__KeyType, __ValueType, unordered_map>;
}

#endif // __JSMap_H
