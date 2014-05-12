/**
 * This file is part of the Ondraluk memory managing library
 *
 * @author Christian Ondracek & Lukas Oberbichler
 * @date May 2014
 *
 * @file main.cpp
 */

#include "includes/MemoryManager.hpp"
#include "includes/LinearAllocator.hpp"

#include <string.h>
#include <cstdio>
#include <type_traits>
#include <cassert>
#include <memory>

#include <fstream>
#include <iomanip>


/**
 * BoundsCheckingPolicy
 *
 * Provides functions to fill memory with a given symbol and later check if the bounds were exceeded to
 * locate memory leaks
 */
template <size_t N, int S>
struct BoundsCheckingPolicy {

	/**
	 * fill
	 *
	 * @param void* begin
	 * @param size_t size

	 * Fills BOUNDSIZE bytes at beginning and end at given memory addr with SYMBOL
	 *
	 * @return void
	 */
	void fill(void* begin, size_t size) const {

		union {
			unsigned char* asByte;
			size_t* asSizeT;
			void* asVoid;
		};

		asVoid = begin;

		*asSizeT = size;
		asByte += sizeof(size_t);

		memset(asVoid, SYMBOL, BOUNDSIZE);
		asByte += size - BOUNDSIZE - SIZEOFALLOCATION;
		memset(asVoid, SYMBOL, BOUNDSIZE);
	}

	/**
	 * check
	 *
	 * @param void* begin
	 * @param size_t adujstment

	 * Fills BOUNDSIZE bytes at beginning and end at given memory addr with SYMBOL
	 *
	 * @return void
	 */
	void check(void* begin, size_t adjustment = 0) const {
		union {
			unsigned char* asByte;
			size_t* asSizeT;
			void* asVoid;
		};

		asVoid = begin;
		
		asByte -= (SIZEOFALLOCATION + BOUNDSIZE + adjustment);

		size_t allocationSize = *asSizeT;

		asByte += SIZEOFALLOCATION;

		int tmp[N];
		memset(tmp, SYMBOL, N);

		if (memcmp(asVoid, tmp, N) == 0) {

			asByte += (allocationSize - BOUNDSIZE - SIZEOFALLOCATION);
			if (memcmp(asVoid, tmp, N) == 0) {
				return;
			}
		}

		assert(false);
	}

	static_assert(N > 1, "N < 2");
	size_t BOUNDSIZE = N;
	size_t SIZEOFALLOCATION = sizeof(size_t);
	int SYMBOL = S;
};

/**
 * Specialization of a BoundsCheckingPolicy with 0-byte size
 */
template <>
struct BoundsCheckingPolicy<0, 0> {
	void fill(void*, size_t) const {};
	void check(void*, size_t adjustment = 0) const {};

	size_t BOUNDSIZE = 0;
	size_t SIZEOFALLOCATION = 0;
};

typedef BoundsCheckingPolicy<0,0> NoBoundsCheckingPolicy;


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

using namespace debuglib::logger;

int main() {

	//FileLogger f(NoFilter(),SimpleFormatter(),FileOutputter("log.txt"));

	ConsoleLogger g;

	LinearAllocator la(1000);
	MemoryManager<LinearAllocator, BoundsCheckingPolicy<4, 0xEF>> memoryManager(LinearAllocator(2000), BoundsCheckingPolicy<4, 0xEF>());
	//MemoryManager<LinearAllocator, NoBoundsCheckingPolicy, NoMemoryTracking> memoryManager2(LinearAllocator(2000));

	int* t = memoryManager.allocate<int, 4>();

	//char* tCh = reinterpret_cast<char*>(t);
	//tCh--;
	//*tCh = 0xDD;


	//int* tp = memoryManager2.allocate<int>();
		
	//myStruct* ms = memoryManager.allocate<myStruct>(2);

	memoryManager.deallocate<int, ARRAY::ENUM::NO>(t);
//
//	memoryManager.deallocate<myStruct, IS_ARRAY::TRUE>(ms);


	return 0;
}
