
#ifndef __SSet_H
#define __SSet_H

#include "Container.h"

#include <unordered_set>
#include <set>

namespace NS_SSTL
{
#define __SSetSuper ContainerT<__DataType, __BaseType<__DataType>>

	template<typename __DataType, template<typename...> class __BaseType>
	class SSetT : public __SSetSuper
	{
	private:
		using __Super = __SSetSuper;

#ifndef _MSC_VER
	protected:
		__UsingSuperType(__ContainerType);
		__UsingSuperType(__ItrType);
		__UsingSuperType(__ItrConstType);

		__UsingSuperType(__InitList);

		__UsingSuperType(__DataConstRef);

		__UsingSuperType(__CB_ConstRef_void);
		__UsingSuperType(__CB_ConstRef_bool);
#endif

	private:
		__ContainerType& m_data = __Super::m_data;

	public:
		SSetT()
		{
		}

		template<typename... args>
		explicit SSetT(__DataConstRef data, const args&... others)
			: __Super(data, others...)
		{
		}

		explicit SSetT(__ContainerType&& container)
		{
			__Super::swap(container);
		}

		SSetT(SSetT&& set)
		{
			__Super::swap(set);
		}

		SSetT(const SSetT& set)
			: __Super(set)
		{
		}

		explicit SSetT(__InitList initList)
			: __Super(initList)
		{
		}

		template<typename T, typename = checkContainer_t<T>>
		explicit SSetT(const T& container)
			: __Super(container)
		{
		}

		template<typename T, typename = checkContainer_t<T>>
		explicit SSetT(T& container)
			: __Super(container)
		{
		}

		SSetT& operator=(__ContainerType&& container)
		{
			__Super::swap(container);
			return *this;
		}

		SSetT& operator=(SSetT&& set)
		{
			__Super::swap(set);
			return *this;
		}

		SSetT& operator=(const SSetT& set)
		{
			__Super::assign(set);
			return *this;
		}

		SSetT& operator=(__InitList initList)
		{
			__Super::assign(initList);
			return *this;
		}

		template <typename T>
		SSetT& operator=(const T&t)
		{
			__Super::assign(t);
			return *this;
		}

		template <typename T>
		SSetT& operator=(T&t)
		{
			__Super::assign(t);
			return *this;
		}

	private:
		void _add(__DataConstRef data) override
		{
			m_data.insert(data);
		}

		size_t _del(__DataConstRef data) override
		{
			auto itr = m_data.find(data);
			if (itr == m_data.end())
			{
				return 0;
			}

			m_data.erase(itr);

			return 1;
		}

		__ItrConstType _find(__DataConstRef data) const override
		{
			return m_data.find(data);
		}

	public:
		template <typename T>
		SSetT& operator+= (const T& rhs)
		{
			__Super::add(rhs);
			return *this;
		}

		SSetT& operator+= (__InitList rhs)
		{
			__Super::add(rhs);
			return *this;
		}

		template <typename T>
		SSetT& operator-= (const T& rhs)
		{
			__Super::del(rhs);
			return *this;
		}

		SSetT& operator-= (__InitList rhs)
		{
			__Super::del(rhs);
			return *this;
		}
		
		template <typename T>
		SSetT operator+ (const T& rhs)
		{
			SSetT set(*this);
			set += rhs;
			return set;
		}

		template <typename T>
		SSetT operator- (const T& rhs)
		{
			SSetT set(*this);
			set -= rhs;
			return set;
		}

		template <typename T>
		SSetT operator& (const T& rhs)
		{
			SSetT set;
			SSetT other(rhs);
			for (auto&data : m_data)
			{
				if (other.includes(data))
				{
					set.add(data);
				}
			}
			return set;
		}

	public:
		template <typename T>
		SSetT<T, __BaseType> map(CB_T_Ret<__DataConstRef, T> cb) const
		{
			SSetT<T, __BaseType> set;

			for (auto&data : m_data)
			{
				set.add(cb(data));
			}
			
			return set;
		}

		template <typename CB, typename RET = decltype(declval<CB>()(__DataType()))>
		SSetT<RET, __BaseType> map(const CB& cb) const
		{
			return map<RET>(cb);
		}

		SSetT filter(__CB_ConstRef_bool cb) const
		{
			SSetT set;

			for (auto&data : m_data)
			{
				if (cb(data))
				{
					set.add(data);
				}
			}

			return set;
		}
	};

	template <typename __DataType, template<typename...> class __BaseType = std::set>
	using SSet = SSetT<__DataType, __BaseType>;

	template <typename __DataType>
	using SUnsortSet = SSet < __DataType, std::unordered_set> ;
}

#endif // __SSet_H
