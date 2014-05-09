#ifndef MEMORYMANAGER_HPP
#define MEMORYMANAGER_HPP

#include <cstdlib>
#include <new>
#include <utility>
#include <type_traits>


template <bool> struct podness {};
template <bool> struct arrayness {};

//template <> struct<true> podness { static const size_t = };

namespace ondraluk {

enum IS_ARRAY { FALSE, TRUE};

//	struct BOUNDSCHECKINGMODE {
//		enum ENUM {
//			ENABLE,
//			DISABLE
//		};
//	};
//
//	struct MEMORYTRACKINGMODE {
//		enum ENUM {
//			OFF,
//			MINIMAL,
//			DETAILED
//		};
//	};

	template <class Allocator, class BoundsChecker, class Tracker>
	class MemoryManager {
	public:
		MemoryManager(Allocator allocator = Allocator(),
					  BoundsChecker boundsChecker = BoundsChecker(),
					  Tracker tracker = Tracker());

		~MemoryManager();

		template <typename T>
		T* allocate();

		template <typename T>
		T* allocate(size_t n);

		template <typename T, IS_ARRAY E = IS_ARRAY::FALSE>
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
		void deallocate(podness<true>, T* addr, arrayness<true>);

		template <typename T>
		void deallocate(podness<false>, T* addr, arrayness<true>);

		template <typename T>
		void deallocate(podness<true>, T* addr, arrayness<false>);

		template <typename T>
		void deallocate(podness<false>, T* addr, arrayness<false>);


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

		size_t size = sizeof(T)*n + sizeof(size_t) + 2 * mBoundsChecker.BOUNDSIZE;
		asVoid = mAllocator.allocate(size);

		mBoundsChecker.fill(asVoid, size);

		asByte += mBoundsChecker.BOUNDSIZE;

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

		size_t size = sizeof(T)*n + sizeof(size_t) + 2 * mBoundsChecker.BOUNDSIZE;

		// need to allocate + sizeof(size_t) to be able to store n in the four bytes before
		asVoid = mAllocator.allocate(size);

		mBoundsChecker.fill(asVoid, size);

		asByte += mBoundsChecker.BOUNDSIZE;

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
		size_t size = sizeof(T) + 2 * mBoundsChecker.BOUNDSIZE;

		void* addr = mAllocator.allocate(size);


		union {
			void* asVoid;
			unsigned char* asByte;
		};

		asVoid = addr;

		mBoundsChecker.fill(asVoid, size);

		asByte += mBoundsChecker.BOUNDSIZE;



		T* obj = new (asVoid) T;
		return obj;
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	T* MemoryManager<Allocator, BoundsChecker, Tracker>::allocate(podness<true>) {
		size_t size = sizeof(T) + 2 * mBoundsChecker.BOUNDSIZE;

		void* addr = mAllocator.allocate(size);

		union {
			void* asVoid;
			unsigned char* asByte;
			T* asT;
		};

		asVoid = addr;

		mBoundsChecker.fill(asVoid, size);

		asByte += mBoundsChecker.BOUNDSIZE;


		return asT;
	}

//	template <class Allocator, class BoundsChecker, class Tracker>
//	template <typename T>
//	void MemoryManager<Allocator, BoundsChecker, Tracker>::deallocate(T* addr) {
//		deallocate<T>(podness<std::is_pod<T>::value >(), addr, arrayness<false>());
//	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T, IS_ARRAY E>
	void MemoryManager<Allocator, BoundsChecker, Tracker>::deallocate(T* addr) {
		deallocate<T>(podness<std::is_pod<T>::value >(), addr, arrayness<E>());
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker, Tracker>::deallocate(podness<true>, T* addr, arrayness<true>) {
		union {
			void* asVoid;
			unsigned char* asByte;
		};

		asVoid = addr;
		asByte -= mBoundsChecker.BOUNDSIZE;

		mAllocator.free(asVoid);
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker, Tracker>::deallocate(podness<false>, T* addr, arrayness<true>) {
		union
		  {
			size_t* asSizeT;
			T* asT;
		  };

		// set to address
		asT = addr;

		// get the size which is stored in the bytes before the instance
		const size_t n = asSizeT[-1];

		// destruct from top
		for(size_t i = n - 1; i > 0; --i) {
			asT[i].~T();
		}

		union {
			void* asVoid;
			unsigned char* asByte;
		};

		asVoid = addr;
		asByte -= sizeof(size_t);
		asByte -= mBoundsChecker.BOUNDSIZE;

		// free all
		mAllocator.free(asVoid);
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker, Tracker>::deallocate(podness<true>, T* addr, arrayness<false>) {

		union {
			void* asVoid;
			unsigned char* asByte;
		};

		asVoid = addr;
		asByte -= mBoundsChecker.BOUNDSIZE;

		mAllocator.free(asVoid);
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker, Tracker>::deallocate(podness<false>, T* addr, arrayness<false>) {
		addr->~T();

		union {
			void* asVoid;
			unsigned char* asByte;
		};

		asVoid = addr;
		asByte -= mBoundsChecker.BOUNDSIZE;

		mAllocator.free(asVoid);
	}
}

#endif
