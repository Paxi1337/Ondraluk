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

	MemoryManager<LinearAllocator, BoundsCheckingPolicy<4, 0xEF>> memoryManager(LinearAllocator(2000));

	char* t = memoryManager.allocate<char>(10);

	memoryManager.deallocate<char, ARRAY::ENUM::YES>(t);

	myStruct* ch = memoryManager.allocate<myStruct>(10);

	//char* tCh = reinterpret_cast<char*>(t);
	//*(ch-1) = 0xDD;

	memoryManager.deallocate<myStruct, ARRAY::ENUM::YES>(ch);

//
//	memoryManager.deallocate<myStruct, IS_ARRAY::TRUE>(ms);


	return 0;
}
