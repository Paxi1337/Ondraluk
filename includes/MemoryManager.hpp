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

		template <typename T>
		T* allocate(size_t size);

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
	template <typename T>
	T* MemoryManager<Allocator>::allocate(size_t size) {
		return static_cast<T*>(mAllocator.allocate(size));
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
