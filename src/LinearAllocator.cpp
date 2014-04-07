#include "../includes/LinearAllocator.hpp"

using namespace ondraluk;

LinearAllocator::LinearAllocator() : mMem(nullptr), mCurrent(nullptr), mEnd(nullptr) {
}

LinearAllocator::LinearAllocator(size_t size) {
	init(size);
}

LinearAllocator::~LinearAllocator() {
	::free(mMem);
}

void LinearAllocator::init(size_t initSize) {
	mMem = static_cast<byte*>(::malloc(initSize));
	mCurrent = mMem;
	mEnd = mMem + initSize;
}

void* LinearAllocator::allocate(size_t size) {
	void* address = asVoid;
	mCurrent += size;

	if(mCurrent >= mEnd) {
		mCurrent -= size;
		return nullptr;
	}

	return address;
}

void LinearAllocator::free(void* mem) {
	asVoid = mem;
}


