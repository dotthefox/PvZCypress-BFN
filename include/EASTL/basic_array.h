#ifndef EASTL_BASIC_ARRAY_H
#define EASTL_BASIC_ARRAY_H

#include <EASTL/internal/config.h>
#include <EASTL/allocator.h>
#include <EASTL/type_traits.h>
#include <EASTL/iterator.h>
#include <EASTL/algorithm.h>
#include <EASTL/memory.h>

#ifdef _MSC_VER
#pragma warning(push, 0)
#include <new>
#include <stddef.h>
#pragma warning(pop)
#else
#include <new>
#include <stddef.h>
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4530)  // C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
#pragma warning(disable: 4345)  // Behavior change: an object of POD type constructed with an initializer of the form () will be default-initialized
#pragma warning(disable: 4244)  // Argument: conversion from 'int' to 'const eastl::vector<T>::value_type', possible loss of data
#pragma warning(disable: 4127)  // Conditional expression is constant
#pragma warning(disable: 4480)  // nonstandard extension used: specifying underlying type for enum
#endif

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif

#ifndef EASTL_BASIC_ARRAY_HAS_CAPACITY
#define EASTL_BASIC_ARRAY_HAS_CAPACITY 1
#endif

namespace eastl
{
	struct BasicArrayRoot
	{
		static void* emptyArrayBegin() { return (void*)0x141EAAFB8; }

		enum { kSizeMask = 0x7fffffff };
		enum { kHasNoCapacityMask = 0x80000000 };
		enum { kSizeIndex = -1 };
		enum { kCapacityIndex = -2 };

		void* allocateArrayMemory(size_t n, size_t alignment, void* arena)
		{
			// todo: turn these into configurable macros
			auto arenaAlloc = reinterpret_cast<void* (*)(void*, __int64 size, __int64 alignment)>(0x140389540);
			auto findArena = reinterpret_cast<void* (*)(void*, bool)>(0x140389E70);

			void* pArena = arena;
			if (!pArena)
				pArena = findArena(this, true);
			void* allocMem = arenaAlloc(pArena, n, alignment);
			return allocMem;
		}

		void freeArrayMemory(void* mem, size_t size)
		{
			auto arenaFree = reinterpret_cast<void(*)(void*, void*)>(0x140389FC0);
			auto findArena = reinterpret_cast<void* (*)(void*, bool)>(0x140389E70);

			void* arena = findArena(this, true);
			arenaFree(arena, mem);
		}
	};

	template <typename T>
	class basic_array : public BasicArrayRoot
	{
		typedef EASTLAllocatorType	allocator_type;
		typedef eastl_size_t        size_type;
		typedef ptrdiff_t           difference_type;

	protected:
		T* mpBegin;

	public:
		typedef T                    value_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T* iterator;
		typedef const T* const_iterator;

	public:
		basic_array();
		basic_array(const basic_array& x);
		basic_array(const basic_array& x, void* arena);
		~basic_array();

		uint32_t* get_cursor() const { return reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(mpBegin)); }

		iterator begin();
		const_iterator begin() const;

		iterator end();
		const_iterator end() const;

		bool empty() const;
		size_type size() const;
		size_type capacity() const;

		pointer data();
		const_pointer data() const;

		reference operator[](size_type n);
		const_reference operator[](size_type n) const;

		void push_back(const value_type& value);
		void push_back(value_type&& value);
		void push_back();

		void* push_back_uninitialized(void* arena = nullptr);
		void* push_back_uninitialized_rawmove(void* arena = nullptr);

		iterator		erase(iterator position);
		iterator		erase(iterator first, iterator last);
		void			clear();
		void			clear(void* arena);

	protected:
		T* DoAllocate(size_type n, void* arena);
		void DoFree(T* ptr, size_type n);
		void DoDestroyValues(pointer first, pointer last);
		size_type GetNewCapacity(size_type currentCapacity);
	};

	template <typename T>
	inline basic_array<T>::basic_array()
		: mpBegin((T*)emptyArrayBegin())
	{ }


	template<typename T>
	inline basic_array<T>::basic_array(const basic_array& x)
	{
		size_type size = x.size();

		T* const pBegin = mpBegin = DoAllocate(size, nullptr);

		if (size)
		{
			uint32_t* cursor = get_cursor();
			cursor[kSizeIndex] = size;

#if(EASTL_BASIC_ARRAY_HAS_CAPACITY)
			cursor[kCapacityIndex] = size;
#endif
			eastl::uninitialized_copy_ptr(x.mpBegin, x.mpBegin + size, pBegin);
		}
	}

	// todo: currently unfinished
	template<typename T>
	inline basic_array<T>::basic_array(const basic_array& x, void* arena)
	{
		size_type size = x.size();

		T* const pBegin = mpBegin = DoAllocate(size, arena);

		if (size)
		{
			uint32_t* cursor = get_cursor();
			cursor[kSizeIndex] = size;

#if(EASTL_BASIC_ARRAY_HAS_CAPACITY)
			cursor[kCapacityIndex] = size;
#endif
		}
	}

	template<typename T>
	inline basic_array<T>::~basic_array()
	{
		T* const pBegin = mpBegin;

		if (pBegin == (T*)emptyArrayBegin())
			return;

		DoDestroyValues(pBegin, end());
		DoFree(pBegin, capacity());
	}

	template<typename T>
	inline typename basic_array<T>::iterator basic_array<T>::begin()
	{
		return mpBegin;
	}

	template<typename T>
	inline typename basic_array<T>::const_iterator basic_array<T>::begin() const
	{
		return mpBegin;
	}

	template<typename T>
	inline typename basic_array<T>::iterator basic_array<T>::end()
	{
		return mpBegin + size();
	}

	template<typename T>
	inline typename basic_array<T>::const_iterator basic_array<T>::end() const
	{
		return mpBegin + size();
	}

	template<typename T>
	inline bool basic_array<T>::empty() const
	{
		return false;
	}

	template<typename T>
	inline typename basic_array<T>::size_type basic_array<T>::size() const
	{
		//const uint32_t* cursor = reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(mpBegin));
		const uint32_t* cursor = get_cursor();

#if(EASTL_BASIC_ARRAY_HAS_CAPACITY)
		return size_type(cursor[kSizeIndex] & kSizeMask);
#else
		return size_type(cursor[kSizeIndex]);
#endif
	}

	template<typename T>
	inline typename basic_array<T>::size_type basic_array<T>::capacity() const
	{
		const uint32_t* cursor = get_cursor();

#if(EASTL_BASIC_ARRAY_HAS_CAPACITY)
		return size_type(cursor[kCapacityIndex]);
#else
		return size_type(cursor[kSizeIndex]);
#endif
	}

	template<typename T>
	inline typename basic_array<T>::pointer basic_array<T>::data()
	{
		return mpBegin;
	}

	template<typename T>
	inline typename basic_array<T>::const_pointer basic_array<T>::data() const
	{
		return mpBegin;
	}

	template<typename T>
	inline typename basic_array<T>::reference basic_array<T>::operator[](size_type n)
	{
		return *(mpBegin + n);
	}

	template<typename T>
	inline typename basic_array<T>::const_reference basic_array<T>::operator[](size_type n) const
	{
		return *(mpBegin + n);
	}

	template<typename T>
	inline void basic_array<T>::push_back(const value_type& value)
	{
		new (push_back_uninitialized()) value_type(value);
	}

	// todo: implement
	template<typename T>
	inline void basic_array<T>::push_back(value_type&& value)
	{

	}

	template<typename T>
	inline void basic_array<T>::push_back()
	{
		new (push_back_uninitialized()) value_type();
	}

	template<typename T>
	inline void* basic_array<T>::push_back_uninitialized(void* arena)
	{
		T* const pBegin = mpBegin;
		T* const pEnd = end();
		const size_type nPrevSize = size_type(pEnd - pBegin);

#if(EASTL_BASIC_ARRAY_HAS_CAPACITY)
		size_type currentCapacity = capacity();
		if (currentCapacity != nPrevSize)
		{
			uint32_t* cursor = get_cursor();
			cursor[kSizeIndex] = nPrevSize + 1;
			return pEnd;
		}
		else
#endif
		{
			const size_type nNewCapacity = GetNewCapacity(nPrevSize);
			pointer const pNewData = DoAllocate(nNewCapacity, arena);

			pointer pNewEnd = eastl::uninitialized_move_ptr(pBegin, pEnd, pNewData);

			DoDestroyValues(pBegin, pEnd);
			DoFree(pBegin, nPrevSize);

			mpBegin = pNewData;

			uint32_t* cursor = get_cursor();
			cursor[kSizeIndex] = nPrevSize + 1;

#if(EASTL_BASIC_ARRAY_HAS_CAPACITY)
			cursor[kCapacityIndex] = nNewCapacity;
#endif
			return pNewEnd;
		}
	}

	// todo: implement
	template<typename T>
	inline void* basic_array<T>::push_back_uninitialized_rawmove(void* arena)
	{
		return nullptr;
	}

	template<typename T>
	inline typename basic_array<T>::iterator basic_array<T>::erase(iterator position)
	{
		size_type sz = size();
		T* const pEnd = mpBegin + sz;

		if ((position + 1) < pEnd)
			eastl::copy(position + 1, pEnd, position);

		pEnd->~value_type();

		uint32_t* cursor = get_cursor();
		cursor[kSizeIndex] = sz - 1;

		return position;
	}

	template<typename T>
	inline typename basic_array<T>::iterator basic_array<T>::erase(iterator first, iterator last)
	{
		if (first == last)
			return last;

		size_type oldSize = size();
		T* const pEnd = mpBegin + oldSize;

		iterator const position = eastl::copy(last, pEnd, first);
		DoDestroyValues(position, pEnd);

		uint32_t* cursor = get_cursor();
		cursor[kSizeIndex] = uint32_t(oldSize - (last - first));

		return first;
	}

	template<typename T>
	inline void basic_array<T>::clear()
	{
		erase(mpBegin, end());
	}

	template<typename T>
	inline void basic_array<T>::clear(void* arena)
	{
		if (mpBegin == (T*)emptyArrayBegin())
			return;

		erase(mpBegin, end());

#if (EASTL_BASIC_ARRAY_HAS_CAPACITY)
		enum { kHeaderSize = sizeof(uint32_t) * 2 > EASTL_ALIGN_OF(T) ? sizeof(uint32_t) * 2 : EASTL_ALIGN_OF(T) };
#else
		enum { kHeaderSize = sizeof(uint32_t) > EASTL_ALIGN_OF(T) ? sizeof(uint32_t) : EASTL_ALIGN_OF(T) };
#endif
		auto arenaFree = reinterpret_cast<void(*)(void*, void*)>(0x140389FC0);
		arenaFree(arena, reinterpret_cast<uint8_t*>(mpBegin) - kHeaderSize);
		mpBegin = (T*)emptyArrayBegin();
	}

	template<typename T>
	inline T* basic_array<T>::DoAllocate(size_type n, void* arena)
	{
		if (!n)
			return (T*)emptyArrayBegin();

#if (EASTL_BASIC_ARRAY_HAS_CAPACITY)
		enum {kHeaderSize = sizeof(uint32_t) * 2 > EASTL_ALIGN_OF(T) ? sizeof(uint32_t) * 2 : EASTL_ALIGN_OF(T) };
#else
		enum {kHeaderSize = sizeof(uint32_t) > EASTL_ALIGN_OF(T) ? sizeof(uint32_t) : EASTL_ALIGN_OF(T) };
#endif

		void* mem = allocateArrayMemory(kHeaderSize + n * sizeof(T), EASTL_ALIGN_OF(T), arena);

		return (T*)(reinterpret_cast<uint8_t*>(mem) + kHeaderSize);
	}

	template<typename T>
	inline void basic_array<T>::DoFree(T* ptr, size_type n)
	{
		if (ptr == (T*)emptyArrayBegin())
			return;

#if(EASTL_BASIC_ARRAY_HAS_CAPACITY)
		enum { kHeaderSize = sizeof(uint32_t) * 2 > EASTL_ALIGN_OF(T) ? sizeof(uint32_t) * 2 : EASTL_ALIGN_OF(T) };
#else
		enum { kHeaderSize = sizeof(uint32_t) > EASTL_ALIGN_OF(T) ? sizeof(uint32_t) : EASTL_ALIGN_OF(T) };
#endif

		//checkCapacityAssert(mpBegin);

		freeArrayMemory(reinterpret_cast<uint8_t*>(p) - kHeaderSize, kHeaderSize + n * sizeof(T));
	}

	template<typename T>
	inline void basic_array<T>::DoDestroyValues(pointer first, pointer last)
	{
		for (; first < last; ++first)
			first->~value_type();
	}

	template<typename T>
	inline basic_array<T>::size_type basic_array<T>::GetNewCapacity(size_type currentCapacity)
	{
#if(EASTL_BASIC_ARRAY_HAS_CAPACITY)
		return (currentCapacity > 0) ? currentCapacity * 2 : 1;
#else
		return currentCapacity + 1;
#endif
	}
} // namespace eastl

#endif // Header include guard