
#ifndef __Util_H
#define __Util_H

namespace NS_SSTL
{
	template <typename T>
	using decayType_t = typename decay<T>::type;

	template <typename T, typename U> struct decay_is_same
		: is_same<typename decay<T>::type, U>::type
	{
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

	template <typename T> struct tagTryCompare {
		bool operator()(const T&t1, const T&t2)
		{
			return _compare(t1, t2);
		}

		static bool compare(const T&t1, const T&t2)
		{
			return _compare(t1, t2);
		}

		template <typename A, typename B>
		static bool _compare(const pair<A,B>&t1, const pair<A, B>&t2)
		{
			return tagTryCompare<A>::compare(t1.first, t2.first)
				&& tagTryCompare<B>::compare(t1.second, t2.second);
		}

		template <typename U, typename RET = decltype(declval<const U&>() == declval<const U&>())>
		static RET _compare(const U&t1, const U&t2)
		{
			return t1 == t2;
		}

		static bool _compare(...)
		{
			return false;
		}
	};

	//template <typename T> struct tagSortT {
	//	tagSortT(__CB_Sort_T<T> cb) : m_cb(cb)
	//	{
	//	}

	//	__CB_Sort_T<T> m_cb;

	//	bool operator() (const T&lhs, const T&rhs)const
	//	{
	//		return m_cb(lhs, rhs);
	//	}
	//};

	template <typename T> struct tagTrySort {
		tagTrySort(__CB_Sort_T<T> cb = NULL) : m_cb(cb)
		{
		}

		__CB_Sort_T<T> m_cb;

		bool operator() (T&lhs, T&rhs) const
		{
			if (m_cb)
			{
				return m_cb(lhs, rhs);
			}

			return _compare(lhs, rhs);
		}

		template <typename U>
		static auto _compare(U&lhs, U&rhs) -> decltype(declval<U>() < declval<U>())
		{
			return lhs < rhs;
		}

		static bool _compare(...)
		{
			return false;
		}
	};

	template <typename T, typename U> struct tagTryLMove {
		static void lmove(T&t, const U&u)
		{
			_lmove(&t, &u);
		}

		template <typename X, typename Y>
		static auto _lmove(X*px, const Y*py) -> decltype(declval<X*>()->operator<<(declval<Y>()))
		{
			return *px << *py;
		}

		static bool _lmove(...)
		{
			return false;
		}

		enum { value = is_same<decltype(_lmove(declval<T*>(), declval<U*>())), T&>::value };
	};

	template <typename T, typename U = int>	struct tagLMove {
		tagLMove(T&t) : m_t(t)
		{
		}

		T& m_t;

		tagLMove& operator<<(const U& u)
		{
			tagTryLMove<T, U>::lmove(m_t, u);
			return *this;;
		}

		template <typename V>
		tagLMove& operator<<(const V&v)
		{
			tagLMove<T, V>(m_t) << v;
			return *this;
		}
	};

	using tagSSTryLMove = tagLMove<stringstream>;

	template<typename __DataType> class tagDynamicArgsExtractor
	{
	public:
		using FN_ExtractCB = const function<bool(__DataType&v)>&;

		template<typename... args>
		static bool extract(FN_ExtractCB cb, __DataType&v, args&... others)
		{
			return _extract(cb, true, v, others...);
		}

		template<typename... args>
		static bool extractReverse(FN_ExtractCB cb, __DataType&v, args&... others)
		{
			return _extract(cb, false, v, others...);
		}

		template<typename... args>
		static bool _extract(FN_ExtractCB cb, bool bForward, __DataType&v, args&... others)
		{
			if (bForward)
			{
				if (!_extract(cb, bForward, v))
				{
					return false;
				}
			}

			if (sizeof...(others))
			{
				if (!_extract(cb, bForward, others...))
				{
					return false;
				}
			}

			if (!bForward)
			{
				if (!_extract(cb, bForward, v))
				{
					return false;
				}
			}

			return true;
		}

		static bool _extract(FN_ExtractCB cb, bool bForward, __DataType&v)
		{
			return cb(v);
		}
	};
	
	template <typename T, typename C>
	T reduce(const C& container, const function<T(const T& t1, const T& t2)>& cb)
	{
		auto itr = container.begin();
		if (itr == container.end())
		{
			return T();
		}

		auto itrPrev = itr++;
		if (itr == container.end())
		{
			return *itrPrev;
		}

		T ret = cb(*itrPrev, *itr);
		while (true)
		{
			if (++itr == container.end())
			{
				break;
			}

			ret = cb(ret, *itr);
		}

		return ret;
	}

	template <typename C, typename CB, typename T
		= decltype(declval<CB>()(*(declval<C>().begin()), *(declval<C>().begin())))>
	T reduce(const C& container, const CB& cb)
	{
		return reduce<T, C>(container, cb);
	}

	template <typename T>
	void qsort(T* lpData, size_t size, __CB_Sort_T<T> cbCompare)
	{
		if (size < 2)
		{
			return;
		}
		int end = (int)size - 1;

		tagTrySort<T> sort;
		auto fnCompare = [&](T& lhs, T& rhs) {
			if (cbCompare) {
				return cbCompare(lhs, rhs);
			}

			return sort(lhs, rhs);
		};

		function<void(int, int)> fnSort;
		fnSort = [&](int begin, int end) {
			if (begin >= end) {
				return;
			}

			int i = begin;
			int j = end;
			T t = lpData[begin];

			do {
				do {
					if (fnCompare(lpData[j], t)) {
						lpData[i] = lpData[j];
						i++;
						break;
					}
					j--;
				} while (i < j);

				while (i < j) {
					if (fnCompare(t, lpData[i])) {
						lpData[j] = lpData[i];
						j--;
						break;
					}
					i++;
				}
			} while (i < j);

			lpData[i] = t;

			fnSort(begin, i - 1);
			fnSort(i + 1, end);
		};
		
		fnSort(0, end);
	}

	template <typename T>
	void qsort(vector<T>& vecData, __CB_Sort_T<T> cb = NULL)
	{
		size_t size = vecData.size();
		if (size > 1)
		{
			qsort<T>(&vecData.front(), size, cb);
		}
	}

	template <typename _C, typename DATA, typename CB>
	size_t find(_C& container, const DATA& data, const CB& cb)
	{
		size_t uRet = 0;

		for (auto itr = container.begin(); itr != container.end(); )
		{
			if (tagTryCompare<DATA>::compare(*itr, data))
			{
				uRet++;

				if (cb)
				{
					auto lpData = &*itr;
					if (!cb(itr) || itr == container.end())
					{
						break;
					}

					if (&*itr != lpData)
					{
						continue;
					}
				}
			}

			++itr;
		}

		return uRet;
	}

	template <typename _C, typename DATA, typename CB>
	size_t find(const _C& container, const DATA& data, const CB& cb)
	{
		size_t uRet = 0;
		
		for (auto itr = container.begin(); itr != container.end(); ++itr)
		{
			if (tagTryCompare<DATA>::compare(*itr, data))
			{
				uRet++;

				if (!cb && !cb(itr))
				{
					break;
				}
			}
		}

		return uRet;
	}

	template<typename _C, typename CB>
	void itrReverseVisit(_C& container, const CB& cb)
	{
		auto itrBegin = container.begin();
		auto itr = container.end();
		while (itr != itrBegin)
		{
			itr--;
			cb(*itr);
		}
	}
}

#endif // __Util_H
