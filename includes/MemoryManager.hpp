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
#include <cassert>

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

/**
* BoundsCheckingPolicy
*
* Provides functions to fill memory with a given symbol and later check if the bounds were exceeded to
* locate memory leaks
*/
template <size_t N, int S>
struct BoundsCheckingPolicy {

	/**
	* fill
	*
	* @param void* begin
	* @param size_t size

	* Fills BOUNDSIZE bytes at beginning and end at given memory addr with SYMBOL
	*
	* @return void
	*/
	void fill(void* begin, size_t size) const {

		union {
			unsigned char* asByte;
			size_t* asSizeT;
			void* asVoid;
		};

		asVoid = begin;

		memset(asVoid, SYMBOL, BOUNDSIZE);
		asByte += size - BOUNDSIZE - sizeof(size_t);
		memset(asVoid, SYMBOL, BOUNDSIZE);
	}

	/**
	* check
	*
	* @param void* begin
	* @param size_t adujstment

	* Fills BOUNDSIZE bytes at beginning and end at given memory addr with SYMBOL
	*
	* @return void
	*/
	void check(void* begin, size_t size) const {
		union {
			unsigned char* asByte;
			size_t* asSizeT;
			void* asVoid;
		};

		asVoid = begin;

		asByte -= BOUNDSIZE;

		int tmp[N];
		memset(tmp, SYMBOL, N);

		if (memcmp(asVoid, tmp, N) == 0) {

			asByte += size + BOUNDSIZE;
			if (memcmp(asVoid, tmp, N) == 0) {
				return;
			}
		}

		assert(false);
	}

	static_assert(N > 1, "N < 2");
	size_t BOUNDSIZE = N;
	int SYMBOL = S;
};

/**
* Specialization of a BoundsCheckingPolicy with 0-byte size
*/
template <>
struct BoundsCheckingPolicy<0, 0> {
	void fill(void*, size_t) const {};
	void check(void*, size_t adjustment = 0) const {};

	size_t BOUNDSIZE = 0;
};

typedef BoundsCheckingPolicy<0, 0> NoBoundsCheckingPolicy;

namespace ondraluk {

	struct ARRAY {
		enum ENUM { NO , YES};
	};
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
	 *
	 * Tested with gcc4.8, clang3.5, msvc2013
	 */
	template <class Allocator, class BoundsChecker>
	class MemoryManager {
	public:

		/**
		 * Constructor
		 *
		 * @param allocator The used allocator
		 * @param boundsChecker The used boundschecker
		 */
		MemoryManager(Allocator allocator = Allocator(),
					  BoundsChecker boundsChecker = BoundsChecker());

		/**
		 * Destructor
		 */
		~MemoryManager();

		/**
		 * Allocate
		 *
		 * Allocates memory for one instance of T
		 *
		 * @remark Internally uses compile time function lookup for differentiating between array, pods etc.
		 *		   May reserve more memory than actually requested because of boundschecking, tracking ..
		 *
		 * @return T*
		 */
		template <typename T>
		T* allocate();
		/**
		 * Allocate
		 *
		 * @param size_t n
		 *
		 * Allocates memory for n * T
		 *
		 * @remark Internally uses compile time function lookup for differentiating between array, pods etc.
		 *		   May reserve more memory than actually requested because of boundschecking, tracking ..
		 *
		 * @return T*
		 */
		template <typename T>
		T* allocate(size_t n);

		/**
		 * Deallocate
		 *
		 * @param T* addr
		 *
		 * Deallocates the memory at given addr
		 *
		 * @remark Internally uses compile time function lookup for differentiating between array, pods etc.
		 *
		 * @return void
		 */
		template <typename T, ARRAY::ENUM E>
		void deallocate(T* addr);

	private:

		// Encapsulates some information about an allocation
		// Mainly used for tracking
		template <typename T>
		struct Allocation {
			Allocation() : mInternalSize(0), mSize(0) {}
			Allocation(size_t internalsize) : mInternalSize(internalsize), mSize(0) {}

			union {
				unsigned char* mByte;
				void* mVoid;
				T* mT;
			};
			size_t mInternalSize;
			size_t mSize;
		};

		template <typename T>
#ifdef _WIN32
		typename Allocation<T> allocate(podness<true>, arrayallocation<true>, size_t n);
#else
		Allocation<T> allocate(podness<true>, arrayallocation<true>, size_t n);
#endif
		template <typename T>
#ifdef _WIN32
		typename Allocation<T> allocate(podness<false>, arrayallocation<true>, size_t n);
#else
		Allocation<T> allocate(podness<false>, arrayallocation<true>, size_t n);
#endif
		template <typename T>
#ifdef _WIN32
		typename Allocation<T> allocate(podness<true>, arrayallocation<false>, size_t n);
#else
		Allocation<T> allocate(podness<true>, arrayallocation<false>, size_t n);
#endif
		template <typename T>
#ifdef _WIN32
		typename Allocation<T> allocate(podness<false>, arrayallocation<false>, size_t n);
#else
		Allocation<T> allocate(podness<false>, arrayallocation<false>, size_t n);
#endif
		template <typename T>
		void deallocate(podness<true>, T*& addr, arrayness<true>, size_t size);

		template <typename T>
		void deallocate(podness<false>, T*& addr, arrayness<true>, size_t size);

		template <typename T>
		void deallocate(podness<true>, T*& addr, arrayness<false>, size_t size);

		template <typename T>
		void deallocate(podness<false>, T*& addr, arrayness<false>, size_t size);

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
	template <typename T>
	T* MemoryManager<Allocator, BoundsChecker>::allocate() {
		Allocation<T> alloc = allocate<T>(podness<std::is_pod<T>::value >(), arrayallocation<false>(), 1);

		alloc.mSize = sizeof(T);

#ifdef ONDRALUK_TRACKING
		LOG(1, debuglib::logger::DEBUG, "\nMemory allocated:\n"
				"\tstartaddress: %#08x\n"
				"\tnumInstances: %u \n"
				"\trequested size: %u byte(s) \n"
				"\tinternal size: %u byte(s) \n"
				"\tline: %u\n", alloc.mByte, 1, alloc.mSize, alloc.mInternalSize, __LINE__);
#endif

		return alloc.mT;
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
	T* MemoryManager<Allocator, BoundsChecker>::allocate(size_t n) {
		Allocation<T> alloc = allocate<T>(podness<std::is_pod<T>::value >(), arrayallocation<true>(), n);

		alloc.mSize = n * sizeof(T);

#ifdef ONDRALUK_TRACKING
		LOG(1, debuglib::logger::DEBUG, "\nMemory allocated:\n"
				"\tstartaddress: %#08x\n"
				"\tnumInstances: %u \n"
				"\trequested size: %u byte(s) \n"
				"\tinternal size: %u byte(s) \n"
				"\tline: %u\n", alloc.mByte, n, alloc.mSize, alloc.mInternalSize, __LINE__);
#endif

		return alloc.mT;
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
#ifdef _WIN32
	typename MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<true>, arrayallocation<true>, size_t n) {
#else
	MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<true>, arrayallocation<true>, size_t n) {
#endif

		Allocation<T> allocation;

		union
	    {
			void* asVoid;
			size_t* asSizeT;
			T* asT;
			unsigned char* asByte;
	     };

		size_t size = sizeof(T) * n + sizeof(size_t) + 2 * mBoundsChecker.BOUNDSIZE;

		asVoid = mAllocator.allocate(size);

		*asSizeT = sizeof(T) * n;

		asByte += sizeof(size_t);

		mBoundsChecker.fill(asVoid, size);

		asByte += mBoundsChecker.BOUNDSIZE;


		allocation.mVoid = asVoid;
		allocation.mInternalSize = size;

		return allocation;
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
#ifdef _WIN32
	typename MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<false>, arrayallocation<true>, size_t n) {
#else
	MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<false>, arrayallocation<true>, size_t n) {
#endif
		Allocation<T> allocation;

		union
		  {
		    void* asVoid;
		    size_t* asSizeT;
		    T* asT;
		    unsigned char* asByte;
		  };

		size_t size = sizeof(T) * n + sizeof(size_t) + 2 * mBoundsChecker.BOUNDSIZE;

		// need to allocate + sizeof(size_t) to be able to store n in the four bytes before
		asVoid = mAllocator.allocate(size);


		*asSizeT = sizeof(T) * n;

		asByte += sizeof(size_t);

		mBoundsChecker.fill(asVoid, size);

		asByte += mBoundsChecker.BOUNDSIZE;

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
#ifdef _WIN32
	typename MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<false>, arrayallocation<false>, size_t n) {
#else
	MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<false>, arrayallocation<false>, size_t n) {
#endif
		Allocation<T> allocation;

		size_t size = sizeof(T) + 2 * mBoundsChecker.BOUNDSIZE + sizeof(size_t);

		void* addr = mAllocator.allocate(size);

		union {
			void* asVoid;
		    size_t* asSizeT;
			unsigned char* asByte;
		};

		asVoid = addr;

		*asSizeT = sizeof(T) * n;

		asByte += sizeof(size_t);

		mBoundsChecker.fill(asVoid, size);

		asByte += (mBoundsChecker.BOUNDSIZE );

		allocation.mVoid = asVoid;
		allocation.mInternalSize = size;

		new (asVoid) T;

		return allocation;
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
#ifdef _WIN32
	typename MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<true>, arrayallocation<false>, size_t n) {
#else
	MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<true>, arrayallocation<false>, size_t n) {
#endif
		Allocation<T> allocation;

		size_t size = sizeof(T) + 2 * mBoundsChecker.BOUNDSIZE + sizeof(size_t);

		void* addr = mAllocator.allocate(size);

		union {
			void* asVoid;
		    size_t* asSizeT;
			unsigned char* asByte;
			T* asT;
		};

		asVoid = addr;

		*asSizeT = sizeof(T);
		asByte += sizeof(size_t);

		mBoundsChecker.fill(asVoid, size);

		asByte += mBoundsChecker.BOUNDSIZE;

		allocation.mVoid = asVoid;
		allocation.mInternalSize = size;

		return allocation;
	}


	template <class Allocator, class BoundsChecker>
	template <typename T, ARRAY::ENUM E>
	void MemoryManager<Allocator, BoundsChecker>::deallocate(T* addr) {
		Allocation<T> allocation;

		union {
			void* asVoid;
		    size_t* asSizeT;
			unsigned char* asByte;
		};

		asVoid = addr;
		asByte -= (mBoundsChecker.BOUNDSIZE + sizeof(size_t));

		size_t size = *asSizeT;

		allocation.mSize = size;
		allocation.mInternalSize = size + (sizeof(size_t) + 2 * mBoundsChecker.BOUNDSIZE);

		mBoundsChecker.check(addr, size);

		deallocate<T>(podness<std::is_pod<T>::value >(), addr, arrayness<E>(), size);

		asVoid = addr;

		allocation.mVoid = asVoid;

#ifdef ONDRALUK_TRACKING
		LOG(1, debuglib::logger::DEBUG, "\nMemory deallocated:\n"
				"\tstartaddress: %#08x\n"
				"\tnumInstances: %u \n"
				"\trequested size: %u byte(s) \n"
				"\tinternal size: %u byte(s) \n"
				"\tline: %u\n", allocation.mByte, allocation.mSize / sizeof(T), allocation.mSize, allocation.mInternalSize, __LINE__);
#endif

		asByte -= (mBoundsChecker.BOUNDSIZE);

		mAllocator.free(asVoid);
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker>::deallocate(podness<true>, T*& addr, arrayness<true>, size_t size) {
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker>::deallocate(podness<false>, T*& addr, arrayness<true>, size_t size) {

		union
		{
			size_t* asSizeT;
			T* asT;
			unsigned char* asByte;
			void* asVoid;
		};

		asT = addr;

		int numInstances = size / sizeof(T);

		// destruct from top
		for (size_t i = numInstances - 1; i > 0; --i) {
			asT[i].~T();
		}
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker>::deallocate(podness<true>, T*& addr, arrayness<false>, size_t size) {
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker>::deallocate(podness<false>, T*& addr, arrayness<false>, size_t size) {
		addr->~T();
	}
}

#endif
