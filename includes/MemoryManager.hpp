#ifndef MEMORYMANAGER_HPP
#define MEMORYMANAGER_HPP

#include <cstdlib>
#include <new>
#include <utility>

namespace ondraluk {

	template <class Allocator>
	class MemoryManager {
	public:
		MemoryManager(Allocator allocator = Allocator());
		~MemoryManager();

		void* allocateRaw(size_t sizeInBytes);

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
	};

	template <class Allocator>
	MemoryManager<Allocator>::MemoryManager(Allocator allocator) : mAllocator(std::move(allocator)) {
		
	}

	template <class Allocator>
	MemoryManager<Allocator>::~MemoryManager() {

	}

	template <class Allocator>
	void* MemoryManager<Allocator>::allocateRaw(size_t sizeInBytes) {
		return mAllocator.allocate(sizeInBytes);
	}

	template <class Allocator>
	template <typename T>
	T* MemoryManager<Allocator>::allocatePODsIntegrals(size_t arraySize) {
		return static_cast<T*>(mAllocator.allocate(arraySize*sizeof(T)));
	}

	template <class Allocator>
	template <typename T>
	T* MemoryManager<Allocator>::construct() {
		T* obj = new (mAllocator.allocate(sizeof(T))) T;
		return obj;
	}

	template <class Allocator>
	template <typename T>
	void MemoryManager<Allocator>::destruct(T* addr) {
		addr->~T();
		mAllocator.free(static_cast<void*>(addr));
	}

	template <class Allocator>
	template <typename T>
	T* MemoryManager<Allocator>::constructArray(size_t n) {
		union
		  {
		    void* asVoid;
		    size_t* asSizeT;
		    T* asT;
		  };

		// need to allocate + sizeof(size_t) to be able to store n in the four bytes before
		asVoid = allocateRaw(sizeof(T)*n + sizeof(size_t));
		*asSizeT++ = n;

		const T* const beforeLast = asT + n;
		while(asT < beforeLast) {
			new (asT)T;
			asT++;
		}

		// return pointer to the first instance
		return (asT - n);
	}

	template <class Allocator>
	template <typename T>
	void MemoryManager<Allocator>::destructArray(T* addr) {
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
		for(size_t i = n; i > 0; --i) {
			asT[i - 1].~T();
		}

		// free all
		mAllocator.free(static_cast<void*>(asSizeT - 1));
	}
}

#endif
