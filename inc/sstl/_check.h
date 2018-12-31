
#ifndef __Check_H
#define __Check_H

namespace NS_SSTL
{
	template <bool _test>
	using enable_if_t = typename enable_if<_test, void>::type;

	template <bool _test>
	using enable_ifnot_t = enable_if_t<!_test>;

	template<template<typename...> class _check, typename... _types>
	using enableIf_t = enable_if_t<_check<_types...>::value>;

	template<template<typename...> class _check, typename... _types>
	using enableIfNot_t = enable_ifnot_t<_check<_types...>::value>;

	template <typename _check1, typename _check2>
	using enableIf_and_t = enable_if_t<_check1::value && _check2::value>;

	template <typename _check1, typename _check2>
	using enableIf_or_t = enable_if_t<_check1::value || _check2::value>;


	template <typename T>
	using removeConst_t = typename std::remove_const<T>::type;

	template <typename T>
	using removeConstRef_t = removeConst_t<typename std::remove_reference<T>::type>;


	template <typename T1, typename T2>
	using checkSameType_t = enableIf_t<is_same, T1, T2>;

	template <typename T1, typename T2>
	using checkNotSameType_t = enableIfNot_t<is_same, T1, T2>;

	template <typename T1, typename T2>
	using checkSameDecayType_t = checkSameType_t<typename decay<T1>::type, typename decay<T2>::type>;
	
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


	template <typename T, typename... others>
	struct tagCheckArgs0
	{
		typedef T type;
	};
	template <typename... args>
	using args0Type_t = typename tagCheckArgs0<args...>::type;

	template <typename T, typename... args>
	using checkArgs0Type_t = checkSameType_t<T, args0Type_t<args...>>;
	
	template<class _Iter> using checkIter_t =
#ifdef _MSC_VER
	//enableIf_t<_Is_iterator, _Iter>;
	typename iterator_traits<_Iter>::iterator_category;
#else
	_RequireInputIter<_Iter>;
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

	template <typename _C, typename __Itr = decltype(tagItrVisitor::begin(declval<_C&>())), typename = checkIter_t<__Itr>>
	struct tagCheckContainer
	{
		typedef __Itr Itr_Type;
		typedef decltype(tagItrVisitor::begin(declval<const _C&>())) CItr_Type;

		typedef decltype(*declval<__Itr>()) Ref_Type;
		typedef removeConstRef_t<Ref_Type> Data_Type;
	};

	template<class T>
	using containerItrType_t = typename tagCheckContainer<T>::Itr_Type;
	template<class T>
	using containerCItrType_t = typename tagCheckContainer<T>::CItr_Type;

	template<class T>
	using containerRItrType_t = typename tagCheckContainer<T>::RItr_Type;
	template<class T>
	using containerCRItrType_t = typename tagCheckContainer<T>::CRItr_Type;

	template<class T>
	using containerRefType_t = typename tagCheckContainer<T>::Ref_Type;

	template<class T>
	using containerDataType_t = typename tagCheckContainer<T>::Data_Type;

	template<class T>
	using checkContainer_t = containerDataType_t<T>;

	template<class T, typename DATA>
	using checkContainerData_t = checkSameType_t<containerDataType_t<T>, DATA>;

	struct tagCheckNotContainer
	{
		template <typename T, typename = checkContainer_t<T>>
		static void check();

		template <typename T>
		static T check(...);
	};
	template<typename T>
	using checkNotContainer_t = checkSameType_t<T, decltype(tagCheckNotContainer::check<T>())>;


	template <class __C>
	class CItrVisitor : private tagItrVisitor
	{
	public:
		CItrVisitor(__C& container)
			: m_container(container)
		{
		}

	private:
		__C& m_container;

		typedef decltype(tagItrVisitor::begin(declval<__C&>())) __ITR;

	public:
		__ITR begin()
		{
			return tagItrVisitor::begin(m_container);
		}

		__ITR end()
		{
			return tagItrVisitor::end(m_container);
		}
	};
};

#endif // __Check_H
