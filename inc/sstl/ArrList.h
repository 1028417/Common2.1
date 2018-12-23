
#ifndef __ArrList_H
#define __ArrList_H

namespace NS_SSTL
{
#define __ArrListSuper SListT<__DataType, __BaseType>

	template<typename __DataType, template<typename...> class __BaseType>
	class ArrListT : public __ArrListSuper
	{
	private:
		__UsingSuper(__ArrListSuper)

		__UsingSuperType(__RItrType)
		__UsingSuperType(__CRItrType)

		PtrArray<__DataType> m_ptrArray;

	private:
		template <class T = PtrArray<__DataType>>
		class __ArrListOperator
		{
		public:
			__ArrListOperator(T& ptrArray)
				: m_ptrArray(ptrArray)
			{
			}

		private:
			T& m_ptrArray;
		
		public:
			template <typename CB>
			bool get(TD_PosType pos, CB cb)
			{
				if (pos >= m_ptrArray.size())
				{
					return false;
				}

				cb(*m_ptrArray[pos]);

				return true;
			}

			template <typename CB, typename = checkCBBool_t<CB, __DataRef, TD_PosType>>
			void forEach(const CB& cb, size_t startPos, size_t count)
			{
				if (startPos >= m_ptrArray.size())
				{
					return;
				}

				for (auto pos = startPos; pos < m_ptrArray.size(); pos++)
				{
					if (!cb(*m_ptrArray[pos], pos))
					{
						break;
					}

					if (count > 0)
					{
						count--;
						if (0 == count)
						{
							break;
						}
					}
				}
			}

			template <typename CB, typename = checkCBVoid_t<CB, __DataRef, TD_PosType>, typename = void>
			void forEach(const CB& cb, size_t startPos, size_t count)
			{
				forEach([&](__DataRef data, size_t pos) {
					cb(data, pos);
					return true;
				}, startPos, count);
			}

			template <typename CB, typename = checkCBVoid_t<CB, __DataRef>, typename = void, typename = void>
			void forEach(const CB& cb, size_t startPos, size_t count)
			{
				forEach([&](__DataRef data, size_t pos) {
					cb(data);
					return true;
				}, startPos, count);
			}

			template <typename CB, typename = checkCBBool_t<CB, __DataRef>
				, typename = void, typename = void, typename = void>
			void forEach(const CB& cb, size_t startPos, size_t count)
			{
				forEach([&](__DataRef data, size_t pos) {
					return cb(data);
				}, startPos, count);
			}
		};

		__ArrListOperator<> m_ListOperator = __ArrListOperator<>(m_ptrArray);
		__ArrListOperator<>& _getOperator()
		{
			return m_ListOperator;
		}
		__ArrListOperator<const __ContainerType>& _getOperator() const
		{
			return (__ArrListOperator<const PtrArray<__DataType>>&)m_ListOperator;
		}

	public:
		ArrListT() = default;

		template<typename... args>
		explicit ArrListT(__DataConstRef data, const args&... others)
			: __Super(data, others...)
			, m_ptrArray(m_data)
		{
		}

		explicit ArrListT(__ContainerType&& container)
		{
			__Super::swap(container);

			m_ptrArray.assign(m_data);
		}

		ArrListT(ArrListT&& lst)
		{
			__Super::swap(lst);

			m_ptrArray.assign(m_data);
		}

		ArrListT(const ArrListT& lst)
			: __Super(lst)
			, m_ptrArray(m_data)
		{
		}

		explicit ArrListT(__InitList initList)
			: __Super(initList)
			, m_ptrArray(m_data)
		{
		}

		template<typename T, typename = checkContainerData_t<T, __DataType>>
		explicit ArrListT(const T& container)
			: __Super(container)
			, m_ptrArray(m_data)
		{
		}

		ArrListT& operator=(__ContainerType&& container)
		{
			__Super::swap(container);
			m_ptrArray.assign(m_data);
			return *this;
		}

		ArrListT& operator=(ArrListT&& lst)
		{
			__Super::swap(lst);
			m_ptrArray.assign(m_data);
			return *this;
		}

		ArrListT& operator=(const ArrListT& lst)
		{
			__Super::assign(lst);
			m_ptrArray.assign(m_data);
			return *this;
		}

		ArrListT& operator=(__InitList initList)
		{
			__Super::assign(initList);
			m_ptrArray.assign(m_data);
			return *this;
		}

		template <typename T>
		ArrListT& operator=(const T&t)
		{
			__Super::assign(t);
			m_ptrArray.assign(m_data);
			return *this;
		}

		template <typename T>
		ArrListT& operator=(T&t)
		{
			__Super::assign(t);
			m_ptrArray.assign(m_data);
			return *this;
		}

		template<typename CB>
		void operator() (const CB& cb, TD_PosType startPos = 0, size_t count = 0)
		{
			_getOperator().forEach(cb, startPos, count);
		}

		template<typename CB>
		void operator() (const CB& cb, TD_PosType startPos = 0, size_t count = 0) const
		{
			_getOperator().forEach(cb, startPos, count);
		}

	public:
		const __ContainerType& operator->() const
		{
			return m_data;
		}

		const __ContainerType& data() const
		{
			return m_data;
		}

		operator __ContainerType& () = delete;
		operator const __ContainerType& () const
		{
			return m_data;
		}

		__CItrType begin() const
		{
			return m_data.cbegin();
		}
		__CItrType end() const
		{
			return m_data.cend();
		}

		__CRItrType rbegin() const
		{
			return m_data.crbegin();
		}
		__CRItrType rend() const
		{
			return m_data.crend();
		}

	public:
		int find(CB_T_Pos_RET<__DataConstRef, bool> cb, TD_PosType stratPos = 0) const
		{
			int iRetPos = -1;

			(*this)([&](__DataConstRef data, TD_PosType pos) {
				if (cb(data, pos))
				{
					iRetPos = pos;
					return false;
				}

				return true;
			});

			return iRetPos;
		}

		bool get(TD_PosType pos, __CB_Ref_void cb)
		{
			return _getOperator().get(pos, cb);
		}
		bool get(TD_PosType pos, __CB_ConstRef_void cb) const
		{
			return _getOperator().get(pos, cb);
		}

		bool set(TD_PosType pos, __DataConstRef& data)
		{
			return get([&](__DataRef m_data) {
				m_data = data;
			});
		}

		template<typename T>
		ArrListT& addFront(const T& container)
		{
			if (!__Super::checkIsSelf(container))
			{
				itrReverseVisit(container, [&](__DataConstRef data) {
					m_data.push_front(data);
					m_ptrArray.addFront(m_data.front());
				});
			}

			return *this;
		}

		ArrListT& addFront(__InitList initList)
		{
			addFront<__InitList>(initList);
		}

		template<typename... args>
		ArrListT& addFront(__DataConstRef data, const args&... others)
		{
			(void)tagDynamicArgsExtractor<const __DataType>::extractReverse([&](__DataConstRef data) {
				m_data.push_front(data);
				m_ptrArray.addFront(m_data.front());
				return true;
			}, data, others...);

			return *this;
		}

		bool popBack(__CB_ConstRef_void cb = NULL)
		{
			(void)m_ptrArray.popBack();

			return __Super::popBak(cb);
		}

		bool popBack(__DataRef data)
		{
			(void)m_ptrArray.popBack();

			return __Super::popBak(data);
		}

		ArrListT& sort(__CB_Sort_T<__DataType> cb = NULL)
		{
			__Super::sort(cb);
			m_ptrArray.assign(m_data);

			return *this;
		}

		ArrListT& Reverse()
		{
			reverse(m_data.begin(), m_data.end());
			m_ptrArray.Reverse();

			return *this;
		}

		ArrListT slice(int startPos) const
		{
			ArrListT ret;

			startPos = _checkPos(startPos);
			if (startPos >= 0)
			{
				forEach([&](__DataConstRef data) {
					ret.add(data);
				}, (TD_PosType)startPos);
			}

			return ret;
		}

		ArrListT slice(int startPos, int endPos) const
		{
			ArrListT ret;

			startPos = _checkPos(startPos);
			endPos = _checkPos(endPos);

			if (startPos >= 0 && endPos >= 0 && startPos <= endPos)
			{
				(*this)([&](__DataConstRef data) {
					ret.add(data);
				}, (TD_PosType)startPos, size_t(endPos - startPos + 1));
			}

			return ret;
		}

	private:
		inline void _add(__DataConstRef data) override
		{
			m_data.push_back(data);
			m_ptrArray.add(m_data.back());
		}

		bool _popFront(__CB_Ref_void cb = NULL) override
		{
			(void)m_ptrArray.popFront();

			return __Super::_popFront(cb);
		}

	private:
		int _checkPos(int pos) const
		{
			auto size = m_data.size();
			if (0 == size)
			{
				return -1;
			}

			if (pos < 0)
			{
				return (int)size + pos;
			}
			else
			{
				if (pos >= (int)size)
				{
					return -1;
				}

				return pos;
			}
		}
	};
}

#endif //__ArrList_H
