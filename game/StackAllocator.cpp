#include "StackAllocator.hpp"

void* StackAllocator::allocate(i64 size, i64 alignment) {
	return malloc(size);
}

void StackAllocator::free(void* ptr) {
	::free(ptr);
}
