
#ifndef __Define_H
#define __Define_H

#include <initializer_list>

#include <vector>

#include <functional>

using namespace std;

namespace NS_SSTL
{
	typedef size_t TD_PosType;

	template <typename T>
	using InitList_T = const initializer_list<T>&;

	template <typename T, typename RET>
	using CB_T_Ret = const function<RET(const T&v)>&;

	template <typename T>
	using CB_T_void = CB_T_Ret<T, void>;

	template <typename T>
	using CB_T_bool = CB_T_Ret<T, bool>;

	template <typename T, typename RET>
	using CB_T_Pos_RET = const function<RET(T, TD_PosType)>&;

	template <typename T>
	using __CB_Sort_T = const function<bool(T&lhs, T&rhs)>&;

	enum class E_DelConfirm
	{
		DC_Yes
		, DC_No
		, DC_Abort
		, DC_YesAbort
	};


	template<typename __DataType, template<typename...> class __BaseType> class SArrayT;

	template <typename __DataType, template<typename...> class __BaseType = std::vector>
	using SArray = SArrayT<__DataType, __BaseType>;
};

#define __SuperType(T) typename __Super::T
#define __UsingSuperType(T) using T = __SuperType(T)

#endif // __Define_H
