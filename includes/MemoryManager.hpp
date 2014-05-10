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

template <bool> struct podness {};
template <bool> struct arrayness { static const bool value = false; };
template <> struct arrayness<true> { static const bool value = true; };

template <bool> struct arrayajustment { static const size_t adjustment = 0; };
template <> struct arrayajustment<true> { static const size_t adjustment = 4; };

namespace ondraluk {

enum IS_ARRAY { FALSE, TRUE};

	/**
	 * MemoryManager
	 *
	 * Policy-based memory manager class
	 * 	Allocator
	 * 	BoundsChecker
	 * 	Tracker
	 *
	 * Interoperates with the given policies to
	 * 	-allocate / deallocate memory
	 * 	-bounds checking
	 * 	-memory tracking
	 */
	template <class Allocator, class BoundsChecker, class Tracker>
	class MemoryManager {
	public:

		/**
		 * Constructor
		 *
		 * @param allocator
		 * @param boundsChecker
		 * @param tracker
		 */
		MemoryManager(Allocator allocator = Allocator(),
					  BoundsChecker boundsChecker = BoundsChecker(),
					  Tracker tracker = Tracker());

		/**
		 * Destructor
		 */
		~MemoryManager();

		/**
		 * allocate
		 *
		 * Allocates memory for one T
		 * Uses a variation of the intToType template to deduce if the user wants to
		 *
		 * 	- allocate a pod type variable or
		 * 	- construct an object
		 *
		 *
		 * @see MemoryManager::allocate(podness<true>)
		 * @see MemoryManager::allocate(podness<false>)
		 *
		 * @return T*
		 */
		template <typename T>
		T* allocate();

		/**
		 * allocate
		 *
		 * @param size_t n
		 *
		 * Allocates memory for n T
		 * Uses a variation of the intToType template to deduce if the user wants to
		 *
		 * 	- allocate an array of pod type variables or
		 * 	- construct an array of objects
		 *
		 * @see MemoryManager::allocate(podness<true>, size_t n)
		 * @see MemoryManager::allocate(podness<false>, size_t n)
		 *
		 * @return T*
		 */
		template <typename T>
		T* allocate(size_t n);

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
		T* allocate(podness<true>, size_t n);

		template <typename T>
		T* allocate(podness<false>, size_t n);

		template <typename T>
		T* allocate(podness<true>);

		template <typename T>
		T* allocate(podness<false>);

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
		Tracker mTracker;
	};

	template <class Allocator, class BoundsChecker, class Tracker>
	MemoryManager<Allocator, BoundsChecker, Tracker>::MemoryManager(Allocator allocator, BoundsChecker boundsChecker, Tracker tracker) : mAllocator(std::move(allocator)),
																											     mBoundsChecker(std::move(boundsChecker)),
																											     mTracker(std::move(tracker)) {
		
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	MemoryManager<Allocator, BoundsChecker, Tracker>::~MemoryManager() {
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	T* MemoryManager<Allocator, BoundsChecker, Tracker>::allocate() {
		return allocate<T>(podness<std::is_pod<T>::value >());
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	T* MemoryManager<Allocator, BoundsChecker, Tracker>::allocate(size_t n) {
		return allocate<T>(podness<std::is_pod<T>::value >(), n);
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	T* MemoryManager<Allocator, BoundsChecker, Tracker>::allocate(podness<true>, size_t n) {

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

		return asT;
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	T* MemoryManager<Allocator, BoundsChecker, Tracker>::allocate(podness<false>, size_t n) {
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


		const T* const beforeLast = asT + n;
		while(asT < beforeLast) {
			new (asT)T;
			asT++;
		}

		// return pointer to the first instance
		return (asT - n);
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	T* MemoryManager<Allocator, BoundsChecker, Tracker>::allocate(podness<false>) {
		size_t size = sizeof(T)+2 * mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION;

		void* addr = mAllocator.allocate(size);


		union {
			void* asVoid;
			unsigned char* asByte;
		};

		asVoid = addr;

		mBoundsChecker.fill(asVoid, size);

		asByte += (mBoundsChecker.BOUNDSIZE + mBoundsChecker.SIZEOFALLOCATION);

		T* obj = new (asVoid) T;
		return obj;
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	T* MemoryManager<Allocator, BoundsChecker, Tracker>::allocate(podness<true>) {
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


		return asT;
	}


	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T, IS_ARRAY E>
	void MemoryManager<Allocator, BoundsChecker, Tracker>::deallocate(T* addr) {

		mBoundsChecker.check(addr, arrayajustment<arrayness<E>::value>::adjustment);

		union {
			void* asVoid;
			unsigned char* asByte;
		};

		deallocate<T>(podness<std::is_pod<T>::value >(), addr, arrayness<E>());

		asVoid = addr;
		asByte -= mBoundsChecker.BOUNDSIZE;

		mAllocator.free(asVoid);
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker, Tracker>::deallocate(podness<true>, T*& addr, arrayness<true>) {
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker, Tracker>::deallocate(podness<false>, T*& addr, arrayness<true>) {
		union
		{
			size_t* asSizeT;
			T* asT;
			unsigned char* asByte;
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

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker, Tracker>::deallocate(podness<true>, T*&, arrayness<false>) {
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker, Tracker>::deallocate(podness<false>, T*& addr, arrayness<false>) {
		addr->~T();
	}
}

#endif
