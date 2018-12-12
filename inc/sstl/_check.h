
#ifndef __Check_H
#define __Check_H

#include "_define.h"

namespace NS_SSTL
{
	template <bool _test>
	using enable_if_t = typename enable_if<_test, void>::type;

	template <bool _test>
	using enable_ifnot_t = enable_if_t<!_test>;

	template<template<typename...> typename _check, typename... _types>
	using enableIf_t = enable_if_t<_check<_types...>::value>;

	template<template<typename...> typename _check, typename... _types>
	using enableIfNot_t = enable_ifnot_t<_check<_types...>::value>;

	template <typename _check1, typename _check2>
	using enableIf_and_t = enable_if_t<_check1::value && _check2::value>;

	template <typename _check1, typename _check2>
	using enableIf_or_t = enable_if_t<_check1::value || _check2::value>;

	template <typename T1, typename T2>
	using checkSameType_t = enableIf_t<is_same, T1, T2>;

	template <typename T1, typename T2>
	using checkNotSameType_t = enableIfNot_t<is_same, T1, T2>;

	template <typename CB, typename Ret, typename... Paras>
	using checkCBRet_t = checkSameType_t<decltype(declval<CB>()(declval<Paras>()...)), Ret>;

	template <typename CB, typename... Paras>
	using checkCBVoid_t = checkSameType_t<decltype(declval<CB>()(declval<Paras>()...)), void>;

	template <typename CB, typename... Paras>
	using checkCBBool_t = checkSameType_t<decltype(declval<CB>()(declval<Paras>()...)), bool>;

	template <typename T1, typename T2>
	using checkClass_t = enableIf_and_t<is_class<T1>, is_class<T2>>;

	template <typename T>
	using checkIsConst_t = enableIf_t<is_const, T>;

	template <typename _Base, typename T>
	using checkIsBase_t = enableIf_t<is_base_of, _Base, T>;

	template <typename _Base, typename T>
	using checkNotIsBase_t = enableIfNot_t<is_base_of, _Base, T>;


	template<class _Iter>
#ifdef _MSC_VER
	using checkIter_t = enableIf_t<_Is_iterator, _Iter>;
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
