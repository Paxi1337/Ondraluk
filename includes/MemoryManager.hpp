#ifndef MEMORYMANAGER_HPP
#define MEMORYMANAGER_HPP

#include <cstdlib>
#include <new>
#include <utility>

namespace ondraluk {

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

		void* allocateRaw(size_t sizeInBytes);
		void deallocateRaw(void* addr);

		template <typename T>
		T* allocatePODsIntegrals(size_t amountOfTs);

		template <typename T>
		T* construct();

		template <typename T>
		void destruct(T* addr);

		template <typename T>
		T* constructArray(size_t n);

		template <typename T>
		void destructArray(T* addr);

	private:
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
	void* MemoryManager<Allocator, BoundsChecker, Tracker>::allocateRaw(size_t sizeInBytes) {
		size_t size = sizeInBytes + 2 * BoundsChecker::BOUNDSIZE;

		union {
			void* asVoid;
			unsigned char* asByte;
		};

		asVoid = mAllocator.allocate(size);

		mBoundsChecker(asVoid, size);

		asByte+=BoundsChecker::BOUNDSIZE;

		return asVoid;
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	void MemoryManager<Allocator, BoundsChecker, Tracker>::deallocateRaw(void* addr) {
		union {
			void* asVoid;
			unsigned char* asByte;
		};

		asVoid = addr;
		asByte-=BoundsChecker::BOUNDSIZE;

		mAllocator.free(asVoid);
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	T* MemoryManager<Allocator, BoundsChecker, Tracker>::allocatePODsIntegrals(size_t arraySize) {
		return static_cast<T*>(mAllocator.allocate(arraySize*sizeof(T)));
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	T* MemoryManager<Allocator, BoundsChecker, Tracker>::construct() {
		T* obj = new (mAllocator.allocate(sizeof(T))) T;
		return obj;
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	void MemoryManager<Allocator, BoundsChecker, Tracker>::destruct(T* addr) {
		addr->~T();
		mAllocator.free(static_cast<void*>(addr));
	}

	template <class Allocator, class BoundsChecker, class Tracker>
	template <typename T>
	T* MemoryManager<Allocator, BoundsChecker, Tracker>::constructArray(size_t n) {
		union
		  {
		    void* asVoid;
		    size_t* asSizeT;
		    T* asT;
		  };

		// need to allocate + sizeof(size_t) to be able to store n in the four bytes before
		asVoid = allocateRaw(sizeof(T)*n + sizeof(size_t));
		asSizeT += sizeof(size_t);
		*asSizeT = n;

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
	void MemoryManager<Allocator, BoundsChecker, Tracker>::destructArray(T* addr) {
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

		// free all
		mAllocator.free(static_cast<void*>(asSizeT - 1));
	}
}

#endif
