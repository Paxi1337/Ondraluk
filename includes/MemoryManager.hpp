/**
 * This file is part of the Ondraluk memory managing library
 *
 * @author Christian Ondracek & Lukas Oberbichler
 * @date May 2014
 *
 * @file MemoryManager.hpp
 */

#ifndef MEMORYMANAGER_HPP
#define MEMORYMANAGER_HPP

#include <cstdlib>
#include <new>
#include <type_traits>
#include <utility>

#define ONDRALUK_TRACKING 1

#ifdef ONDRALUK_TRACKING
#include "Logger.h"
#endif

template <bool> struct podness {};
template <bool> struct arrayness { static const bool value = false; };
template <> struct arrayness<true> { static const bool value = true; };

template <bool> struct arrayallocation {};

template <size_t n>
struct wasarrayallocation {
	static_assert(n>0, "allocation of 0 bytes");
	static const bool value = n > 1;
};


template <bool> struct arrayadjustment { static const size_t adjustment = 0; };
template <> struct arrayadjustment<true> { static const size_t adjustment = 4; };

namespace ondraluk {

	enum IS_ARRAY { FALSE, TRUE};

	/**
	 * MemoryManager
	 *
	 * Policy-based memory manager class
	 * 	Allocator
	 * 	BoundsChecker
	 *
	 * Interoperates with the given policies to
	 * 	-allocate / deallocate memory
	 * 	-bounds checking
	 */
	template <class Allocator, class BoundsChecker>
	class MemoryManager {
	public:

		template <typename T>
		struct Allocation {
			union {
				unsigned char* mByte;
				void* mVoid;
				T* mT;
			};
			size_t mInternalSize;
		};

		/**
		 * Constructor
		 *
		 * @param allocator
		 * @param boundsChecker
		 */
		MemoryManager(Allocator allocator = Allocator(),
					  BoundsChecker boundsChecker = BoundsChecker());

		/**
		 * Destructor
		 */
		~MemoryManager();

		/**
		 * allocate
		 *
		 * @param size_t n
		 *
		 * Allocates memory for n T
		 *
		 * @remark Internally uses compile time function lookup.
		 *
		 * @see MemoryManager::allocate(podness<true>, size_t n)
		 * @see MemoryManager::allocate(podness<false>, size_t n)
		 *
		 * @return T*
		 */
		template <typename T, size_t = 1>
		T* allocate();

		/**
		 * deallocate
		 *
		 * @param T* addr
		 *
		 * Deallocates the memory at given addr
		 * Second template parameter declares the arrayness
		 * Uses a variation of the intToType template to deduce if the user wants to
		 *
		 * 	- deallocate pod(s) or
		 * 	- destruct object(s)
		 *
		 * @see MemoryManager::deallocate(T* addr)
		 *
		 * @return void
		 */
		template <typename T, IS_ARRAY E>
		void deallocate(T* addr);

	private:

		template <typename T>
		Allocation<T> allocate(podness<true>, arrayallocation<true>, size_t n);

		template <typename T>
		Allocation<T> allocate(podness<false>, arrayallocation<true>, size_t n);

		template <typename T>
		Allocation<T> allocate(podness<true>, arrayallocation<false>, size_t n);

		template <typename T>
		Allocation<T> allocate(podness<false>, arrayallocation<false>, size_t n);

		template <typename T>
		void deallocate(podness<true>, T*& addr, arrayness<true>);

		template <typename T>
		void deallocate(podness<false>, T*& addr, arrayness<true>);

		template <typename T>
		void deallocate(podness<true>, T*& addr, arrayness<false>);

		template <typename T>
		void deallocate(podness<false>, T*& addr, arrayness<false>);

		/**
		 * Variables
		 */

		Allocator mAllocator;
		BoundsChecker mBoundsChecker;
	};

	template <class Allocator, class BoundsChecker>
	MemoryManager<Allocator, BoundsChecker>::MemoryManager(Allocator allocator, BoundsChecker boundsChecker) :
													mAllocator(std::move(allocator)),
													mBoundsChecker(std::move(boundsChecker)) {

	}

	template <class Allocator, class BoundsChecker>
	MemoryManager<Allocator, BoundsChecker>::~MemoryManager() {
	}

	template <class Allocator, class BoundsChecker>
	template <typename T, size_t n>
	T* MemoryManager<Allocator, BoundsChecker>::allocate() {
		Allocation<T> alloc = allocate<T>(podness<std::is_pod<T>::value >(), arrayallocation<wasarrayallocation<n>::value>(), n);

#ifdef ONDRALUK_TRACKING
		LOG(1, debuglib::logger::DEBUG, "Memory allocated:\n"
				"\tstartaddress: %#08x\n"
				"\tnumInstances: %u \n"
				"\trequested size: %u \n"
				"\tinternal size: %u \n"
				"\tline: %u\n", alloc.mByte, n, n*sizeof(T), alloc.mInternalSize, __LINE__);
#endif

		return alloc.mT;
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
	MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<true>, arrayallocation<true>, size_t n) {

		Allocation<T> allocation;

		union
	    {
			void* asVoid;
			size_t* asSizeT;
			T* asT;
			unsigned char* asByte;
	     };

		size_t size = sizeof(T)*n + sizeof(size_t)+2 * mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION;

		asVoid = mAllocator.allocate(size);

		mBoundsChecker.fill(asVoid, size);

		asByte += (mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION);

		allocation.mVoid = asVoid;
		allocation.mInternalSize = size;

		return allocation;
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
	MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<false>, arrayallocation<true>, size_t n) {

		Allocation<T> allocation;

		union
		  {
		    void* asVoid;
		    size_t* asSizeT;
		    T* asT;
		    unsigned char* asByte;
		  };

		size_t size = sizeof(T)*n + sizeof(size_t)+2 * mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION;

		// need to allocate + sizeof(size_t) to be able to store n in the four bytes before
		asVoid = mAllocator.allocate(size);

		mBoundsChecker.fill(asVoid, size);

		asByte += (mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION);

		*asSizeT = n;

		asByte += sizeof(size_t);


		allocation.mVoid = asVoid;
		allocation.mInternalSize = size;

		const T* const beforeLast = asT + n;
		while(asT < beforeLast) {
			new (asT)T;
			asT++;
		}

		// return pointer to the first instance
		return allocation;
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
	MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<false>, arrayallocation<false>, size_t n) {
		Allocation<T> allocation;

		size_t size = sizeof(T)+2 * mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION;

		void* addr = mAllocator.allocate(size);


		union {
			void* asVoid;
			unsigned char* asByte;
		};

		asVoid = addr;

		mBoundsChecker.fill(asVoid, size);

		asByte += (mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION);

		allocation.mVoid = asVoid;
		allocation.mInternalSize = size;

		new (asVoid) T;

		return allocation;
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
	MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<true>, arrayallocation<false>, size_t n) {
		Allocation<T> allocation;

		size_t size = sizeof(T)+2 * mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION;

		void* addr = mAllocator.allocate(size);

		union {
			void* asVoid;
			unsigned char* asByte;
			T* asT;
		};

		asVoid = addr;

		mBoundsChecker.fill(asVoid, size);

		asByte += (mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION);

		allocation.mVoid = asVoid;
		allocation.mInternalSize = size;

		return asT;
	}


	template <class Allocator, class BoundsChecker>
	template <typename T, IS_ARRAY E>
	void MemoryManager<Allocator, BoundsChecker>::deallocate(T* addr) {

		mBoundsChecker.check(addr, arrayadjustment<arrayness<E>::value>::adjustment);

		union {
			void* asVoid;
			unsigned char* asByte;
		};


		deallocate<T>(podness<std::is_pod<T>::value >(), addr, arrayness<E>());

		asVoid = addr;
		asByte -= mBoundsChecker.BOUNDSIZE;

		mAllocator.free(asVoid);
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker>::deallocate(podness<true>, T*& addr, arrayness<true>) {
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker>::deallocate(podness<false>, T*& addr, arrayness<true>) {
		union
		{
			size_t* asSizeT;
			T* asT;
			unsigned char* asByte;
			void* asVoid;
		};

		// set to address
		asT = addr;

		// get the size which is stored in the bytes before the instance
		const size_t n = asSizeT[-1];

		// destruct from top
		for (size_t i = n - 1; i > 0; --i) {
			asT[i].~T();
		}

		asByte -= sizeof(size_t);

		addr = asT;
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker>::deallocate(podness<true>, T*&, arrayness<false>) {
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker>::deallocate(podness<false>, T*& addr, arrayness<false>) {
		addr->~T();
	}
}

#endif
