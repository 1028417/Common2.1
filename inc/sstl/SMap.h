
#ifndef __SMap_H
#define __SMap_H

#include "Container.h"

#include <map>
#include <unordered_map>

namespace NS_SSTL
{
#define __SMapSuper ContainerT<pair<__KeyType const, __ValueType>, __BaseType<__KeyType, __ValueType>, __KeyType>

	template<typename __KeyType, typename __ValueType, template<typename...> typename __BaseType>
	class SMapT : public __SMapSuper
	{
	private:
		using __Super = __SMapSuper;

#ifndef _MSC_VER
	protected:
		__UsingSuperType(__ContainerType);
		__UsingSuperType(__ItrType);
		__UsingSuperType(__ItrConstType);

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

		const bool m_bMulti = is_same<__ContainerType, std::multimap<__KeyType, __ValueType>>::value
			|| is_same<__ContainerType, std::unordered_multimap<__KeyType, __ValueType>>::value;
		
		template <typename __ContainerType>
		using enable_if_NoMulti = enable_if_t<is_same<__ContainerType, std::map<__KeyType, __ValueType>>::value
			|| is_same<__ContainerType, std::unordered_map<__KeyType, __ValueType>>::value>;

		template <typename __ContainerType>
		using enable_if_Multi = enable_if_t<is_same<__ContainerType, std::multimap<__KeyType, __ValueType>>::value
			|| is_same<__ContainerType, std::unordered_multimap<__KeyType, __ValueType>>::value>;

	public:
		SMapT()
		{
		}

		template <typename T>
		explicit SMapT(const T& keys, const function<__ValueType(__KeyConstRef)>& cb)
		{
			set(keys, cb);
		}

		explicit SMapT(__InitList_Key keys, const function<__ValueType(__KeyConstRef)>& cb)
		{
			set(keys, cb);
		}

		explicit SMapT(__ContainerType&& container)
		{
			__Super::swap(container);
		}

		SMapT(SMapT&& map)
		{
			__Super::swap(map);
		}

		SMapT(const SMapT& map)
			: __Super(map)
		{
		}

		explicit SMapT(__InitList initList)
			: __Super(initList)
		{
		}

		template<typename T, typename = checkContainer_t<T>>
		explicit SMapT(const T& container)
			: __Super(container)
		{
		}

		template<typename T, typename = checkContainer_t<T>>
		explicit SMapT(T& container)
			: __Super(container)
		{
		}

		SMapT& operator=(__ContainerType&& container)
		{
			__Super::swap(container);
			return *this;
		}

		SMapT& operator=(SMapT&& map)
		{
			__Super::swap(map);
			return *this;
		}

		SMapT& operator=(const SMapT& map)
		{
			__Super::assign(map);
			return *this;
		}

		SMapT& operator=(__InitList initList)
		{
			__Super::assign(initList);
			return *this;
		}

		template<typename T>
		SMapT& operator=(const T&t)
		{
			__Super::assign(t);
			return *this;
		}

		template<typename T>
		SMapT& operator=(T&t)
		{
			__Super::assign(t);
			return *this;
		}

	private:
		virtual void _toString(stringstream& ss, const __DataType& pr) const override
		{
			tagSSTryLMove(ss) << '<' << pr.first << ", " << pr.second << '>';
		}
		
		void _add(const __DataType& pr) override
		{
			m_data.insert(pr);
		}

		size_t _del(__KeyConstRef key) override
		{
			return _find(key, [&](__ItrType& itr) {
				itr = m_data.erase(itr);
				return true;
			});
		}

		__ItrConstType _find(__KeyConstRef key) const override
		{
			return m_data.find(key);
		}

		size_t _find(__KeyConstRef key, const function<bool(__ItrType& itr)>& cb)
		{
			auto itr = m_data.find(key);
			if (itr == m_data.end())
			{
				return 0;
			}

			size_t uRet = 1;
			while (true)
			{
				auto lpValue = &itr->second;
				if (!cb(itr) || !m_bMulti)
				{
					break;
				}

				if (itr == m_data.end())
				{
					break;
				}

				if (lpValue == &itr->second)
				{
					if (++itr == m_data.end())
					{
						break;
					}
				}

				if (itr->first != key)
				{
					break;
				}

				uRet++;
			}

			return uRet;
		}

		size_t _find(__KeyConstRef key, const function<bool(const __ItrConstType& itr)>& cb) const
		{
			auto itr = m_data.find(key);
			if (itr == m_data.end())
			{
				return 0;
			}

			size_t uRet = 1;
			while (cb(itr) && m_bMulti)
			{
				if (++itr == m_data.end())
				{
					break;
				}
				
				if (itr->first != key)
				{
					break;
				}

				uRet++;
			}

			return uRet;
		}

		template <typename _V>
		auto _insert(__KeyConstRef key, const _V& value)->decltype(m_data.insert({ key, value }).first->second)&
		{
			return m_data.insert({ key, value }).first->second;
		}

		template <typename _V>
		auto _insert(__KeyConstRef key, const _V& value)->decltype(m_data.insert({ key, value })->second)&
		{
			return m_data.insert({ key, value })->second;
		}

	public:
		template <typename T>
		SMapT& operator+= (const T& rhs)
		{
			__Super::add(rhs);
			return *this;
		}

		SMapT& operator+= (__InitList rhs)
		{
			__Super::add(rhs);
			return *this;
		}

		template <typename T>
		SMapT& operator-= (const T& keys)
		{
			__Super::del(keys);
			return *this;
		}

		SMapT& operator-= (__InitList_Key keys)
		{
			__Super::del(keys);
			return *this;
		}

		template <typename T>
		SMapT operator+ (const T& rhs)
		{
			SMapT map(*this);
			map += rhs;
			return map;
		}

		template <typename T>
		SMapT operator- (const T& rhs)
		{
			SMapT map(*this);
			map -= rhs;
			return map;
		}

		size_t get(__KeyConstRef key, __CB_ValueR_void cb)
		{
			return _find(key, [&](__ItrType& itr) {
				cb(itr->second);

				return true;
			});
		}

		bool get(__KeyConstRef key, __CB_ValueCR_void cb) const
		{
			return _find(key, [&](__ItrConstType& itr) {
				cb(itr->second);
				
				return true;
			});
		}

		size_t get_if(__KeyConstRef key, __CB_ValueR_bool cb)
		{
			return _find(key, [&](__ItrType& itr) {
				return cb(itr->second);
			});
		}

		bool get_if(__KeyConstRef key, __CB_ValueCR_bool cb) const
		{
			return _find(key, [&](__ItrConstType& itr) {
				return cb(itr->second);
			});
		}

		size_t del_key(__KeyConstRef key, __CB_ValueR_void cb=NULL)
		{
			return _find(key, [&](__ItrType& itr) {
				if (cb)
				{
					cb(itr->second);
				}
				
				itr = m_data.erase(itr);
				return true;
			});
		}

		size_t del_if(__CB_ValueR_DelConfirm cb)
		{
			return __Super::del_if([&](__DataType& data) {
				return cb(data.second);
			});
		}

		void forEachValue(__CB_ValueR_bool cb)
		{
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
			for (auto& pr : m_data)
			{
				if (!cb(pr.second))
				{
					break;
				}
			}
		}

		SArray<__KeyType> keys(__CB_KeyCR_ValueCR_bool cb = NULL) const
		{
			SArray<__KeyType> arr;
			for (auto& pr : m_data)
			{
				if (cb && !cb(pr.first, pr.second))
				{
					continue;
				}
				
				arr.add(pr.first);
			}

			return arr;
		}

		SArray<__ValueType> values(__CB_KeyCR_ValueCR_bool cb = NULL) const
		{
			SArray<__ValueType> arr;
			for (auto& pr : m_data)
			{
				if (cb && !cb(pr.first, pr.second))
				{
					continue;
				}

				arr.add(pr.second);
			}

			return arr;
		}

		__ValueType& set(__KeyConstRef key, __ValueConstRef value)
		{
			auto itr = m_data.find(key);
			if (itr != m_data.end())
			{
				return itr->second = value;
			}
			else
			{
				return _insert(key, value);
			}
		}
				
		__ValueType& insert(__KeyConstRef key, __ValueConstRef value)
		{
			return _insert(key, value);
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
		void set(const T& container, const function<__ValueType(__KeyConstRef)>& cb)
		{
			for (auto& key : container)
			{
				this->set(key, cb(key));
			}
		}

		void set(__InitList_Key keys, const function<__ValueType(__KeyConstRef)>& cb)
		{
			set<__InitList_Key>(keys, cb);
		}

	public:
		template <typename T>
		SMapT<__KeyType, T, __BaseType> map(__CB_KeyCR_ValueCR_T<T> cb) const
		{
			SMapT<__KeyType, T, __BaseType> map;

			for (auto& pr : m_data)
			{
				map.set(pr.first, cb(pr.first, pr.second));
			}
			
			return map;
		}

		template <typename CB, typename RET = decltype(declval<CB>()(__KeyType(), __ValueType()))>
		SMapT<__KeyType, RET, __BaseType> map(const CB& cb) const
		{
			return map<RET>(cb);
		}

		SMapT filterValue(__CB_ValueCR_bool cb) const
		{
			SMapT map;

			for (auto& pr : m_data)
			{
				if (cb(pr.second))
				{
					map.set(pr.first, pr.second);
				}
			}

			return map;
		}

		SMapT filter(__CB_KeyCR_ValueCR_bool cb) const
		{
			SMapT map;

			for (auto& pr : m_data)
			{
				if (cb(pr.first, pr.second))
				{
					map.set(pr.first, pr.second);
				}
			}

			return map;
		}
	};

	template <typename __KeyType, typename __ValueType, template<typename...> class __BaseType = std::map>
	using SMap = SMapT<__KeyType, __ValueType, __BaseType>;

	template <typename __KeyType, typename __ValueType>
	using SHashMap = SMap<__KeyType, __ValueType, std::unordered_map>;

	template <typename __KeyType, typename __ValueType>
	using SMultiMap = SMap<__KeyType, __ValueType, std::multimap>;

	template <typename __KeyType, typename __ValueType>
	using SMultiHashMap = SMap<__KeyType, __ValueType, std::unordered_multimap>;
}

#endif // __SMap_H
