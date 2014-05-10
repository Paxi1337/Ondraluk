/**
 * This file is part of the Ondraluk memory managing library
 *
 * @author Christian Ondracek & Lukas Oberbichler
 * @date May 2014
 *
 * @file LinearAllocator.cpp
 */

#include "../includes/LinearAllocator.hpp"

#include <cstdio>

using namespace ondraluk;

LinearAllocator::LinearAllocator(size_t size) : mMem(nullptr), mCurrent(nullptr), mEnd(nullptr), mSize(size) {
	init();
}

LinearAllocator::LinearAllocator(LinearAllocator&& other) : mMem(other.mMem), mCurrent(other.mMem), mEnd(other.mEnd), mSize(other.mSize) {
	other.mMem = nullptr;
	other.mCurrent = nullptr;
	other.mEnd = nullptr;
	other.mSize = 0;
}

LinearAllocator::~LinearAllocator() {
	if (mSize > 0 && mMem != nullptr)
		::free(mMem);

	mMem = nullptr;
}

void LinearAllocator::init() {
	mMem = static_cast<byte*>(::malloc(mSize));
	mCurrent = mMem;
	mEnd = mMem + mSize;
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


