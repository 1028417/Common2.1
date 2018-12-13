
#ifndef __SSet_H
#define __SSet_H

#include "SContainer.h"

namespace NS_SSTL
{
	template<typename __DataType, template<typename...> class __BaseType>
	class SSetT : public __SuperT
	{
	private:
		using __Super = __SuperT;

	protected:
		__UsingSuperType(__ContainerType)

		__UsingSuperType(__ItrType)
		__UsingSuperType(CB_Find)
		__UsingSuperType(__CItrType)
		__UsingSuperType(CB_ConstFind)

		__UsingSuperType(__InitList)

		__UsingSuperType(__DataConstRef)

		__UsingSuperType(__CB_ConstRef_void)
		__UsingSuperType(__CB_ConstRef_bool)

	private:
		__ContainerType& m_data = __Super::m_data;

	public:
		SSetT() = default;

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

		size_t _find(__DataConstRef data, const CB_Find& cb = NULL) override
		{
			auto itr = m_data.find(data);
			if (itr == m_data.end())
			{
				return 0;
			}

			if (cb)
			{
				(void)cb(itr);
			}

			return 1;
		}

		size_t _cfind(__DataConstRef data, const CB_ConstFind& cb = NULL) const override
		{
			auto itr = m_data.find(data);
			if (itr == m_data.end())
			{
				return 0;
			}

			if (cb)
			{
				(void)cb(itr);
			}

			return 1;
		}
		
	public:		
		template <typename T>
		friend SSetT operator& (const SSetT& lhs, const T& rhs)
		{
			SSetT set;
			for (auto&data : rhs)
			{
				if (lhs.includes(data))
				{
					set.add(data);
				}
			}

			return set;
		}

		friend SSetT operator& (const SSetT& lhs, __InitList rhs)
		{
			return lhs & SSetT(rhs);
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
}

#endif // __SSet_H
