#ifndef MEMORYMANAGER_HPP
#define MEMORYMANAGER_HPP

#include <cstdlib>
#include <new>

namespace ondraluk {

	template <class Allocator>
	class MemoryManager {
	public:
		MemoryManager(Allocator allocator = Allocator());
		~MemoryManager();

		void* allocateRaw(size_t sizeInBytes);

		template <typename T>
		T* allocateTyped(size_t amountOfTs);

		template <typename T>
		T* construct();

		template <typename T>
		void destruct(T* addr);

	private:
		Allocator mAllocator;
	};



	template <class Allocator>
	MemoryManager<Allocator>::MemoryManager(Allocator allocator) : mAllocator(allocator) {

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

}

#endif
