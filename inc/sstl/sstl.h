
#ifndef __SSTL_H
#define __SSTL_H

#include "_define.h"

#include "_check.h"

#include "_util.h"

namespace NS_SSTL
{
	template<typename __ContainerType__, typename __KeyType__ = removeConstRef_t<typename tagCheckContainer<__ContainerType__>::Ref_Type>> class SContainerT;
	template <template<typename...> typename __BaseType, class __DataType>
	using SContainer = SContainerT<__BaseType<__DataType>>;


	template<typename __DataType, template<typename...> class __BaseType> class SListT;
	template <typename __DataType, template<typename...> class __BaseType = std::list>
	using SList = SListT<__DataType, __BaseType>;
	
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
}

#include "SContainer.h"

#include "SArray.h"
#include "PtrArray.h"

#include "SList.h"

#include "SSet.h"

#include "SMap.h"

#endif // __SSTL_H
