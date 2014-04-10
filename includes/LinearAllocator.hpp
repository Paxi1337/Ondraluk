#ifndef LINEARALLOCATOR_HPP
#define LINEARALLOCATOR_HPP

#include <cstdlib>

typedef unsigned char byte;

namespace ondraluk {

	class LinearAllocator {
	public:
		explicit LinearAllocator(size_t size);

		// move constructor
		LinearAllocator(LinearAllocator&&);

		~LinearAllocator();

		void init();

		void* allocate(size_t size);
		void free(void* mem);
	private:

		// private copy constructor
		LinearAllocator(const LinearAllocator&) {}

		byte* mMem;

		union {
			void* asVoid;
			byte* mCurrent;
		};

		byte* mEnd;

		size_t mSize;
	};

}


#endif
