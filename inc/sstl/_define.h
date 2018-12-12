
#ifndef __Define_H
#define __Define_H

#include <initializer_list>

#include <vector>
#include <deque>

#include <list>

#include <set>
#include <unordered_set>

#include <map>
#include <unordered_map>

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


	template <template<typename...> typename __BaseType, class __PtrType> class ptrcontainerT;

	template <class __PtrType>
	using ptrvectorT = ptrcontainerT<vector, __PtrType>;

	template <class __Type>
	using ptrvector = ptrvectorT<__Type*>;

	template <class __PtrType>
	using ptrlistT = ptrcontainerT<list, __PtrType>;

	template <class __Type>
	using ptrlist = ptrlistT<__Type*>;


	template<typename __DataType, template<typename...> class __BaseType> class SArrayT;
	template <typename __DataType, template<typename...> class __BaseType = std::vector>
	using SArray = SArrayT<__DataType, __BaseType>;

	template <typename __DataType>
	using SVector = SArray<__DataType>;

	template <typename __DataType>
	using SDeque = SArrayT<__DataType, std::deque>;


	template<typename __Type, template<typename...> class __BaseType> class PtrArrayT;
	template<typename __Type, template<typename...> class __BaseType = ptrvectorT>
	using PtrArray = PtrArrayT<__Type, __BaseType>;

	template<typename __Type, template<typename...> class __BaseType = ptrvectorT>
	using ConstPtrArray = PtrArray<const __Type, __BaseType>;


	template<typename __DataType, template<typename...> class __BaseType> class SSetT;
	template <typename __DataType, template<typename...> class __BaseType = std::set>
	using SSet = SSetT<__DataType, __BaseType>;

	template <typename __DataType>
	using SHashSet = SSet < __DataType, std::unordered_set>;
	

	template<typename __KeyType, typename __ValueType, template<typename...> typename __BaseType> class SMapT;
	template <typename __KeyType, typename __ValueType, template<typename...> class __BaseType = std::map>
	using SMap = SMapT<__KeyType, __ValueType, __BaseType>;

	template <typename __KeyType, typename __ValueType>
	using SHashMap = SMap<__KeyType, __ValueType, std::unordered_map>;

	template <typename __KeyType, typename __ValueType>
	using SMultiMap = SMap<__KeyType, __ValueType, std::multimap>;

	template <typename __KeyType, typename __ValueType>
	using SMultiHashMap = SMap<__KeyType, __ValueType, std::unordered_multimap>;
};

#define __SuperType(T) typename __Super::T
#define __UsingSuperType(T) using T = __SuperType(T)

#endif // __Define_H
