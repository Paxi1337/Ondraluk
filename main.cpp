#include "includes/MemoryManager.hpp"
#include "includes/LinearAllocator.hpp"

#include <string.h>
#include <cstdio>
#include <type_traits>

template <int N, int S>
struct BoundsCheckingPolicy {
	void fill(void* begin, size_t size) const {

		union {
			unsigned char* asByte;
			int* asInt;
			void* asVoid;
		};

		asVoid = begin;
		memset(asVoid, SYMBOLARR[0], BOUNDSIZE);
		asByte += size - BOUNDSIZE;
		memset(asVoid, SYMBOLARR[0], BOUNDSIZE);
	}

	void check(void* begin, size_t size) const {
		union {
					unsigned char* asByte;
					int* asInt;
					void* asVoid;
				};

//		if(memcmp()) {
//
//		}

	}

	static_assert(N > 1, "N < 2");
	const int BOUNDSIZE = N;
	const int SYMBOLARR[N] = {S};
};

template <>
struct BoundsCheckingPolicy<0, 0> {
	void fill(void*) const {
	}

	const int BOUNDSIZE = 0;
	const int SYMBOL = 0;
};

typedef BoundsCheckingPolicy<0,0> NoBoundsCheckingPolicy;

struct NoMemoryTracking {
	void track(void*, size_t, size_t, unsigned int) {
	}
};

#include <memory.h>

using namespace ondraluk;

struct myStruct {
	myStruct() : a(0), b(0), c(0), d(0) {}
	~myStruct() {}

	unsigned long a;
	int b;
	char c;
	char d;
};

int main() {

	//LinearAllocator la(1000);
	MemoryManager<LinearAllocator, BoundsCheckingPolicy<4,0xCD>, NoMemoryTracking> memoryManager(LinearAllocator(2000));


	int* t = memoryManager.allocate<int>();

	myStruct* ms = memoryManager.allocate<myStruct>(2);

	memoryManager.deallocate<int>(t);

	//int* t2 = memoryManager.allocate<int>();

	memoryManager.deallocate<myStruct, IS_ARRAY::TRUE>(ms);

	//myStruct* ms2 = memoryManager.allocate<myStruct>(2);

	//printf("%d\n", std::is_array<int>::value);
	//printf("%d\n", std::is_array<int*>::value);
	//printf("%d\n", std::is_array<int[]>::value);

//	int* test = memoryManager.allocatePODsIntegrals<int>(200);

	//myStruct* s = memoryManager.constructArray<myStruct>(100);

	//s[0].b = 10;
	//s[99].b = 100;


	//memoryManager.destructArray(s);

	//void* mem = memoryManager.allocateRaw(4);

	//memoryManager.deallocateRaw(mem);

	//void* mem4 = memoryManager.allocateRaw(4);





	//void* before = s;

	//memoryManager.destruct(s);

	//char* after = memoryManager.allocatePODsIntegrals<char>(50);



	//memset(test, 0, 200);

	//for(int i = 0; i < 50; ++i) {
	//	test[i] = i;
	//}

	return 0;
}
