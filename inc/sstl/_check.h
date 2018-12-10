
#ifndef __Check_H
#define __Check_H

#include "_define.h"

namespace NS_SSTL
{
	template <bool _test>
	using enable_if_t = typename enable_if<_test, void>::type;

	template <typename T1, typename T2>
	using checkSameType_t = enable_if_t<is_same<T1, T2>::value>;

	template <typename T1, typename T2>
	using checkNotSameType_t = enable_if_t<!is_same<T1, T2>::value>;

	template<class _Iter>
#ifdef _MSC_VER
	using checkIter_t = enable_if_t<_Is_iterator<_Iter>::value>;
#else
	using checkIter_t = _RequireInputIter<_Iter>;
#endif

	struct tagItrVisitor
	{
		template <class __C>
		static auto begin(__C& container)->decltype(container.begin())
		{
			return container.begin();
		}

		template <class __C>
		static auto begin(const __C& container)->decltype(container.begin())
		{
			return container.begin();
		}

		template <class __C>
		static auto end(__C& container)->decltype(container.end())
		{
			return container.end();
		}

		template <class __C>
		static auto end(const __C& container)->decltype(container.end())
		{
			return container.end();
		}
	};

	template<class T>
	using checkContainer_t = checkIter_t<decltype(tagItrVisitor::begin(declval<T>()))>;

	struct tagCheckNotContainer
	{
		template <typename T, typename = checkContainer_t<T>>
		static void check();

		template <typename T>
		static T check(...);

		//enum { value = is_same<T, decltype(check<T>())>::value };
	};

	template<typename T>
	using checkNotContainer_t = checkSameType_t<T, decltype(tagCheckNotContainer::check<T>())>;

	template <class __C>
	struct CItrVisitor : private tagItrVisitor
	{
	public:
		CItrVisitor(__C& container)
			: m_container(container)
		{
		}

	private:
		__C& m_container;

		typedef decltype(tagItrVisitor::begin(declval<__C&>())) __ITR;
		__ITR m_itr;

	public:
		__ITR& begin()
		{
			return m_itr = tagItrVisitor::begin(m_container);
		}

		__ITR& end()
		{
			return m_itr = tagItrVisitor::end(m_container);
		}
	};


	template <typename T1, typename T2>
	using checkClass_t = enable_if_t<is_class<T1>::value && is_class<T2>::value>;

	//template <typename T>
	//using checkNotConstType_t = checkNotSameType_t<typename remove_const<T>::type, T>;

	template <typename T> struct tagGetTypeT {
		typedef T type;
		typedef T& type_ref;
		typedef T* type_pointer;
	};

	template<typename T> struct tagGetTypeT<T&> {
		typedef typename remove_reference<T>::type type;
		typedef typename remove_reference<T>::type_ref type_ref;
		typedef typename remove_reference<T>::type_pointer type_pointer;
	};

	template<typename T> struct tagGetTypeT<T*> {
		typedef typename remove_reference<T>::type type;
		typedef typename remove_reference<T>::type_ref type_ref;
		typedef typename remove_reference<T>::type_pointer type_pointer;
	};

	template <typename T, typename U> struct decay_is_same
		: is_same<typename decay<T>::type, U>::type
	{
	};
};

#endif // __Check_H
