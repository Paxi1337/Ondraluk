#include "includes/MemoryManager.hpp"
#include "includes/LinearAllocator.hpp"

#include <string.h>

template <int N, int S>
struct BoundsCheckingPolicy {
	void operator()(void* begin, size_t size) const {

		union {
			unsigned char* asByte;
			int* asInt;
			void* asVoid;
		};

		asVoid = begin;
		memset(asVoid, SYMBOL, BOUNDSIZE);
		asByte += size - BOUNDSIZE;
		memset(asVoid, SYMBOL, BOUNDSIZE);
	}

	static_assert(N > 1, "N < 2");
	static const int BOUNDSIZE = N;
	static const int SYMBOL = S;
};

template <>
struct BoundsCheckingPolicy<0, 0> {
	void operator()(void*) const {
	}

	static const int BOUNDSIZE = 0;
	static const int SYMBOL = 0;
};

typedef BoundsCheckingPolicy<0,0> NoBoundsCheckingPolicy;

struct NoMemoryTracking {
	void track(void* originalAddr, void* offsetAddr, size_t size, ) {
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
//	int* test = memoryManager.allocatePODsIntegrals<int>(200);

	//myStruct* s = memoryManager.constructArray<myStruct>(100);

	//s[0].b = 10;
	//s[99].b = 100;


	//memoryManager.destructArray(s);

	void* mem = memoryManager.allocateRaw(4);

	memoryManager.deallocateRaw(mem);

	void* mem4 = memoryManager.allocateRaw(4);



	//void* before = s;

	//memoryManager.destruct(s);

	//char* after = memoryManager.allocatePODsIntegrals<char>(50);



	//memset(test, 0, 200);

	//for(int i = 0; i < 50; ++i) {
	//	test[i] = i;
	//}

	return 0;
}
