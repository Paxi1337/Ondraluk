#include "includes/MemoryManager.hpp"
#include "includes/LinearAllocator.hpp"

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

	MemoryManager<LinearAllocator> memoryManager(LinearAllocator(1000));
 	int* test = memoryManager.allocateTyped<int>(50);

 	myStruct* s = memoryManager.construct<myStruct>();

 	void* before = s;

 	memoryManager.destruct(s);

 	char* after = memoryManager.allocateTyped<char>(50);



	memset(test, 0, 200);

	for(int i = 0; i < 50; ++i) {
		test[i] = i;
	}

	return 0;
}