
#ifndef __JSSet_H
#define __JSSet_H

#include "Container.h"

#include <unordered_set>
#include <set>

namespace NS_JSTL
{
#define __JSSetSuper ContainerT<__DataType, __BaseType<__DataType>>

	template<typename __DataType, template<typename...> class __BaseType>
	class JSSetT : public __JSSetSuper
	{
	private:
		using __Super = __JSSetSuper;

#ifndef _MSC_VER
	protected:
		__UsingSuperType(__ContainerType);

		__UsingSuperType(__InitList);

		__UsingSuperType(__DataConstRef);

		__UsingSuperType(__CB_ConstRef_void);
		__UsingSuperType(__CB_ConstRef_bool);
#endif

	protected:
		__ContainerType& _data()
		{
			return __Super::m_data;
		}

		const __ContainerType& _data() const
		{
			return __Super::m_data;
		}

	public:
		JSSetT()
		{
		}

		template<typename... args>
		explicit JSSetT(__DataConstRef data, const args&... others)
		{
			__Super::add(data, others...);
		}

		explicit JSSetT(const JSSetT& set)
			: __Super(set)
		{
		}

		JSSetT(JSSetT&& set)
		{
			__Super::swap(set);
		}

		JSSetT(__InitList initList)
			: __Super(initList)
		{
		}

		template<typename T, typename _ITR = decltype(declval<T>().begin())>
		explicit JSSetT(const T& container)
			: __Super(container)
		{
		}

		JSSetT& operator=(const JSSetT& set)
		{
			__Super::assign(set);
			return *this;
		}

		JSSetT& operator=(JSSetT&& set)
		{
			__Super::swap(set);
			return *this;
		}

		JSSetT& operator=(__InitList initList)
		{
			__Super::assign(initList);
			return *this;
		}

		template <typename T>
		JSSetT& operator=(const T&t)
		{
			__Super::assign(t);
			return *this;
		}

	protected:
		size_t _add(__DataConstRef data) override
		{
			_data().insert(data);

			return _data().size();
		}

		size_t _del(__DataConstRef data) override
		{
			auto itr = _data().find(data);
			if (itr == _data().end())
			{
				return 0;
			}

			_data().erase(itr);

			return 1;
		}

		bool _includes(__DataConstRef data) const override
		{
			return _data().find(data) != _data().end();
		}

	public:
		template <typename T>
		JSSetT& operator+= (const T& rhs)
		{
			__Super::add(rhs);
			return *this;
		}

		JSSetT& operator+= (__InitList rhs)
		{
			__Super::add(rhs);
			return *this;
		}

		template <typename T>
		JSSetT& operator-= (const T& rhs)
		{
			__Super::del(rhs);
			return *this;
		}

		JSSetT& operator-= (__InitList rhs)
		{
			__Super::del(rhs);
			return *this;
		}
		
		template <typename T>
		JSSetT operator+ (const T& rhs)
		{
			JSSetT set(*this);
			set += rhs;
			return set;
		}

		template <typename T>
		JSSetT operator- (const T& rhs)
		{
			JSSetT set(*this);
			set -= rhs;
			return set;
		}

		template <typename T>
		JSSetT operator& (const T& rhs)
		{
			JSSetT set;
			JSSetT other(rhs);
			for (auto& data : _data())
			{
				if (other.includes(data))
				{
					set.add(data);
				}
			}
			return set;
		}

		template<typename... args>
		size_t add(__DataConstRef data, const args&... others)
		{
			return __Super::add(data, others...);
		}

		template<typename T>
		size_t add(const T& container)
		{
			return __Super::add(container);
		}

		size_t add(__InitList initList)
		{
			return add<__InitList>(initList);
		}

	public:
		template <typename T>
		JSSetT<T, __BaseType> map(CB_T_Ret<__DataConstRef, T> cb) const
		{
			JSSetT<T, __BaseType> set;

			if (cb)
			{
				for (auto&data : _data())
				{
					set.add(cb(data));
				}
			}

			return set;
		}

		template <typename CB, typename RET = decltype(declval<CB>()(__DataType()))>
		JSSetT<RET, __BaseType> map(const CB& cb) const
		{
			return map<RET>(cb);
		}

		JSSetT filter(__CB_ConstRef_bool cb) const
		{
			JSSetT set;

			if (cb)
			{
				for (auto&data : _data())
				{
					if (cb(data))
					{
						set.add(data);
					}
				}
			}

			return set;
		}
	};

	template <typename __DataType>
	using JSSet = JSSetT<__DataType, set>;

	template <typename __DataType>
	using JSUnsortSet = JSSetT<__DataType, unordered_set>;
}

#endif // __JSSet_H
