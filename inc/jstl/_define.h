
#ifndef __Define_H
#define __Define_H

#include <initializer_list>

#include <vector>
#include <deque>

#include <functional>

using namespace std;

namespace NS_JSTL
{
	typedef size_t TD_PosType;
	typedef size_t TD_SizeType;

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

	template<typename __DataType, template<typename...> class __BaseType> class JSArrayT;
	template <typename __DataType, template<typename...> class __BaseType = vector>
	using JSArray = JSArrayT<__DataType, __BaseType>;
	
	template <typename __DataType>
	using Vector = JSArray<__DataType, vector>;

	template <typename __DataType>
	using Deque = JSArray<__DataType, deque>;
}

#define __SuperType(T) typename __Super::T
#define __UsingSuperType(T) using T = __SuperType(T)

#endif // __Define_H
