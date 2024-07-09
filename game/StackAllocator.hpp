#pragma once

#include <View.hpp>

// TODO
struct StackAllocator {
	void* allocate(i64 size, i64 alignment);
	void free(void* ptr);

	template<typename T>
	View<T> allocateArray(i64 size);
};

template<typename T>
View<T> StackAllocator::allocateArray(i64 size) {
	return View<T>(allocate(size * sizeof(T), alignof(T)), size);
}
