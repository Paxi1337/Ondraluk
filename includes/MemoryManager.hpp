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
		 * @param size_t n
		 *
		 * Allocates memory for n * T
		 *
		 * @remark Internally uses compile time function lookup for differentiating between array, pods etc.
		 *		   May reserve more memory than actually requested because of boundschecking, tracking ..
		 *
		 * @return T*
		 */
		template <typename T, size_t = 1>
		T* allocate();

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
			Allocation() : mInternalSize(0), mNumInstances(1) {}
			Allocation(size_t internalsize) : mInternalSize(internalsize), mNumInstances(1) {}

			union {
				unsigned char* mByte;
				void* mVoid;
				T* mT;
			};
			size_t mInternalSize;
			size_t mNumInstances;
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
#ifdef _WIN32
		typename Allocation<T> deallocate(podness<true>, T*& addr, arrayness<true>);
#else
		Allocation<T> deallocate(podness<true>, T*& addr, arrayness<true>);
#endif
		template <typename T>
#ifdef _WIN32
		typename Allocation<T> deallocate(podness<false>, T*& addr, arrayness<true>);
#else
		Allocation<T> deallocate(podness<false>, T*& addr, arrayness<true>);
#endif
		template <typename T>
#ifdef _WIN32
		typename Allocation<T> deallocate(podness<true>, T*& addr, arrayness<false>);
#else
		Allocation<T> deallocate(podness<true>, T*& addr, arrayness<false>);
#endif
		template <typename T>
#ifdef _WIN32
		typename Allocation<T> deallocate(podness<false>, T*& addr, arrayness<false>);
#else
		Allocation<T> deallocate(podness<false>, T*& addr, arrayness<false>);
#endif

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

		alloc.mNumInstances = n;

#ifdef ONDRALUK_TRACKING
		LOG(1, debuglib::logger::DEBUG, "\nMemory allocated:\n"
				"\tstartaddress: %#08x\n"
				"\tnumInstances: %u \n"
				"\trequested size: %u byte(s) \n"
				"\tinternal size: %u byte(s) \n"
				"\tline: %u\n", alloc.mByte, alloc.mNumInstances, alloc.mNumInstances * sizeof(T), alloc.mInternalSize, __LINE__);
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

		size_t size = sizeof(T)*n + sizeof(size_t)+2 * mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION;

		asVoid = mAllocator.allocate(size);

		mBoundsChecker.fill(asVoid, size);

		asByte += (mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION);

		*asSizeT = n;

		asByte += sizeof(size_t);

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
#ifdef _WIN32
	typename MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<false>, arrayallocation<false>, size_t n) {
#else
	MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<false>, arrayallocation<false>, size_t n) {
#endif
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
#ifdef _WIN32
	typename MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<true>, arrayallocation<false>, size_t n) {
#else
	MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::allocate(podness<true>, arrayallocation<false>, size_t n) {
#endif
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

		return allocation;
	}


	template <class Allocator, class BoundsChecker>
	template <typename T, ARRAY::ENUM E>
	void MemoryManager<Allocator, BoundsChecker>::deallocate(T* addr) {
		Allocation<T> alloc;

		mBoundsChecker.check(addr, arrayadjustment<arrayness<E>::value>::adjustment);

		union {
			void* asVoid;
			unsigned char* asByte;
		};

		alloc = deallocate<T>(podness<std::is_pod<T>::value >(), addr, arrayness<E>());

		asVoid = addr;

		alloc.mVoid = asVoid;

#ifdef ONDRALUK_TRACKING
		LOG(1, debuglib::logger::DEBUG, "\nMemory deallocated:\n"
				"\tstartaddress: %#08x\n"
				"\tnumInstances: %u \n"
				"\trequested size: %u byte(s) \n"
				"\tinternal size: %u byte(s) \n"
				"\tline: %u\n", alloc.mByte, alloc.mNumInstances, alloc.mNumInstances * sizeof(T), alloc.mInternalSize, __LINE__);
#endif

		asByte -= mBoundsChecker.BOUNDSIZE;

		mAllocator.free(asVoid);
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
#ifdef _WIN32
	typename MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::deallocate(podness<true>, T*& addr, arrayness<true>) {
#else
	MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::deallocate(podness<true>, T*& addr, arrayness<true>) {
#endif
		union
		{
			size_t* asSizeT;
			T* asT;
			unsigned char* asByte;
			void* asVoid;
		};

		// set to address
		asT = addr;

		Allocation<T> allocation;
		// get the size which is stored in the bytes before the instance
		const size_t n = asSizeT[-1];
		allocation.mNumInstances = n;
		allocation.mInternalSize = n * sizeof(T)+2 * mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION + sizeof(size_t);
		return allocation;
		
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
#ifdef _WIN32
	typename MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::deallocate(podness<false>, T*& addr, arrayness<true>) {
#else
	MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::deallocate(podness<false>, T*& addr, arrayness<true>) {
#endif
		Allocation<T> allocation;

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

		allocation.mVoid = asVoid;
		allocation.mNumInstances = n;
		allocation.mInternalSize = n * sizeof(T)+2 * mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION + sizeof(size_t);

		// destruct from top
		for (size_t i = n - 1; i > 0; --i) {
			asT[i].~T();
		}

		asByte -= sizeof(size_t);

		addr = asT;

		return allocation;
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
#ifdef _WIN32
	typename MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::deallocate(podness<true>, T*&, arrayness<false>) {
#else
	MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::deallocate(podness<true>, T*&, arrayness<false>) {
#endif
		Allocation<T> allocation;
		
		allocation.mNumInstances = 1;
		allocation.mInternalSize = sizeof(T) +2 * mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION;

		return allocation;
	}

	template <class Allocator, class BoundsChecker>
	template <typename T>
#ifdef _WIN32
	typename MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::deallocate(podness<false>, T*& addr, arrayness<false>) {
#else
	MemoryManager<Allocator, BoundsChecker>::Allocation<T> MemoryManager<Allocator, BoundsChecker>::deallocate(podness<false>, T*& addr, arrayness<false>) {
#endif
		Allocation<T> allocation;

		allocation.mNumInstances = 1;
		allocation.mInternalSize = sizeof(T) +2 * mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION;

		addr->~T();
		return allocation;
	}
}

#endif
