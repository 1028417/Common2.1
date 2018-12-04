
#ifndef __Define_H
#define __Define_H

#include <initializer_list>

#include <vector>

#include <functional>

#include <algorithm>

using namespace std;

namespace NS_JSTL
{
	template<class _Iter>
#ifdef _MSC_VER
	using checkIter_t = typename enable_if<_Is_iterator<_Iter>::value, void>::type;
#else
	using checkIter_t = _RequireInputIter<_Iter>;
#endif

	template<class T>
	using checkContainer_t = checkIter_t<decltype(declval<T>().begin())>;
	// = typename enable_if<_Is_iterator<decltype(declval<T>().begin())>::value, void>::type;

	//template <typename T>
	//struct tagCheckNotContainerT
	//{
	//	template <typename = checkContainer_t<T>>
	//	static void test(T t);

	//	static T test(...);

	//	enum { value = is_same<decltype(test(T())), T>::value };
	//};
	//template<typename T>
	//using checkNotContainer_t = typename enable_if<tagCheckNotContainerT<T>::value, void>::type;


	template <typename T1, typename T2>
	using checkClass_t = typename enable_if<is_class<T1>::value && is_class<T2>::value, bool>::type;

	template <typename T1, typename T2>
	using checkNotSameType_t = typename enable_if<!is_same<T1, T2>::value, bool>::type;

	template <typename T>
	using checkNotConstType_t = typename enable_if<!is_same<typename remove_const<T>::type, T>::value, bool>::type;


	typedef size_t TD_PosType;

	template <typename T>
	using InitList_T = const initializer_list<T>&;

	template <typename T, typename RET>
	using CB_T_Ret = const function<RET(const T&v)>&;

	template <typename T>
	using CB_T_void = CB_T_Ret<T, void>;

	template <typename T>
	using CB_T_bool = CB_T_Ret<T, bool>;

	template <typename T>
	using CB_T_Pos = const function<bool(T, TD_PosType)>&;

	template <typename T>
	using __CB_Sort_T = const function<bool(T&lhs, T&rhs)>&;


	template<typename __DataType, template<typename...> class __BaseType> class JSArrayT;

	template <typename __DataType, template<typename...> class __BaseType = vector>
	using JSArray = JSArrayT<__DataType, __BaseType>;
};

#define __SuperType(T) typename __Super::T
#define __UsingSuperType(T) using T = __SuperType(T)

#endif // __Define_H
