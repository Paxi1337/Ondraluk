#ifndef LINEARALLOCATOR_HPP
#define LINEARALLOCATOR_HPP

#include <cstdlib>

typedef unsigned char byte;

namespace ondraluk {

	class LinearAllocator {
	public:
		LinearAllocator();
		explicit LinearAllocator(size_t size);
		~LinearAllocator();

		void init(size_t initSize);

		void* allocate(size_t size);
		void free(void* mem);
	private:
		byte* mMem;

		union {
			void* asVoid;
			byte* mCurrent;
		};

		byte* mEnd;
	};

}


#endif
