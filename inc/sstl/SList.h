
#ifndef __SList_H
#define __SList_H

#include "SContainer.h"

#include "SMap.h"

namespace NS_SSTL
{
	template<typename __DataType, template<typename...> class __BaseType>
	class SListT : public __SuperT
	{
	private:
		using __Super = __SuperT;

	protected:

	public:
		SListT() = default;

	private:

	};
}

#endif // __SList_H
