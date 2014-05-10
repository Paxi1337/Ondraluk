/**
 * This file is part of the Ondraluk memory managing library
 *
 * @author Christian Ondracek & Lukas Oberbichler
 * @date May 2014
 *
 * @file LinearAllocator.hpp
 */

#ifndef LINEARALLOCATOR_HPP
#define LINEARALLOCATOR_HPP

#include <cstdlib>

typedef unsigned char byte;

namespace ondraluk {

	/**
	 * LinearAllocator
	 *
	 * Allocator which allocates an initial linear area of memory and
	 * parts the init-memory into smaller pieces
	 */
	class LinearAllocator {
	public:
		/**
		 * Constructor
		 *
		 * @see LinearAllocator::init()
		 * @param size - initial size in bytes
		 */
		explicit LinearAllocator(size_t size);

		/**
		 * Move constructor
		 * @param
		 */
		LinearAllocator(LinearAllocator&&);

		/**
		 * Destructor
		 *
		 * Frees the allocated memory
		 */
		~LinearAllocator();

		/**
		 * Allocates the initial area of memory
		 *
		 * @return void
		 */
		void init();

		/**
		 * allocate
		 *
		 * @param size_t size
		 *
		 * Returns the next free size-bytes memory and moves the mCurrent pointer to the next free block of memory
		 *
		 * @return void* pointer to memory
		 */
		void* allocate(size_t size);

		/**
		 * free
	     *
		 * @param void* mem
		 *
		 * Sets mCurrent to given memory address
		 *
		 * @return void
		 */
		void free(void* mem);
	private:
		/**
		 * Private copy constructor
		 * @param
		 */
		LinearAllocator(const LinearAllocator&);

		/**
		 * Variables
		 */

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
