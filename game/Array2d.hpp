#pragma once

#include <Types.hpp>
#include <engine/Math/Vec2.hpp>
#include "View2d.hpp"

template<typename T>
struct Array2d {
	static Array2d uninitialized(i64 sizeX, i64 sizeY);
	static Array2d uninitialized(Vec2T<i64> size);
	static Array2d filled(i64 sizeX, i64 sizeY, const T& value);
	static Array2d filled(Vec2T<i64> size, const T& value);
	Array2d(const Array2d&) = delete;
	Array2d(Array2d&& other) noexcept;
	~Array2d();

	Array2d& operator=(const Array2d&) = delete;
	Array2d& operator=(Array2d&&) noexcept;

	Array2d clone() const;

	T& operator()(i64 x, i64 y);
	const T& operator()(i64 x, i64 y) const;

	i64 sizeX() const;
	i64 sizeY() const;
	Vec2T<i64> size() const;
	T* data();
	const T* data() const;

private:
	Array2d(i64 sizeX, i64 sizeY);

	i64 sizeX_;
	i64 sizeY_;
	T* data_;
};

template<typename T>
View2d<T> view2d(Array2d<T>& a) {
	return View2d<T>(a.data(), a.sizeX(), a.sizeY());
}
template<typename T>
View2d<const T> constView2d(const Array2d<T>& a) {
	return View2d<const T>(a.data(), a.sizeX(), a.sizeY());
}

template<typename T>
Array2d<T> Array2d<T>::uninitialized(i64 sizeX, i64 sizeY) {
	return Array2d<T>(sizeX, sizeY);
}

template<typename T>
Array2d<T> Array2d<T>::uninitialized(Vec2T<i64> size) {
	return uninitialized(size.x, size.y);
}

template<typename T>
Array2d<T> Array2d<T>::filled(i64 sizeX, i64 sizeY, const T& value) {
	auto result = Array2d<T>::uninitialized(sizeX, sizeY);
	for (i64 y = 0; y < sizeY; y++) {
		for (i64 x = 0; x < sizeX; x++) {
			result(x, y) = value;
		}
	}
	return result;
}

template<typename T>
Array2d<T> Array2d<T>::filled(Vec2T<i64> size, const T& value) {
	return filled(size.x, size.y, value);
}

template<typename T>
Array2d<T>::Array2d(Array2d&& other) noexcept 
	: sizeX_(other.sizeX_)
	, sizeY_(other.sizeY_)
	, data_(other.data_) {
	other.data_ = nullptr;
	other.sizeX_ = 0;
	other.sizeY_ = 0;
}

template<typename T>
Array2d<T>::~Array2d() {
	operator delete(data_);
}

template<typename T>
Array2d<T>& Array2d<T>::operator=(Array2d&& other) noexcept {
	operator delete(data_);
	sizeX_ = other.sizeX_;
	sizeY_ = other.sizeY_;
	data_ = other.data_;
	other.data_ = nullptr;
	other.sizeX_ = 0;
	other.sizeY_ = 0;
}

template<typename T>
Array2d<T> Array2d<T>::clone() const {
	auto m = Array2d<T>::uninitialized(size());
	memcpy(m.data(), data_, sizeX_ * sizeY_ * sizeof(T));
	return m;
}

template<typename T>
T& Array2d<T>::operator()(i64 x, i64 y) {
	DEBUG_ASSERT(x >= 0 && x < sizeX_);
	DEBUG_ASSERT(y >= 0 && y < sizeY_);
	return data_[y * sizeX_ + x];
}

template<typename T>
const T& Array2d<T>::operator()(i64 x, i64 y) const {
	return const_cast<Array2d<T>*>(this)->operator()(x, y);
}

template<typename T>
i64 Array2d<T>::sizeX() const {
	return sizeX_;
}

template<typename T>
i64 Array2d<T>::sizeY() const {
	return sizeY_;
}

template<typename T>
Vec2T<i64> Array2d<T>::size() const {
	return Vec2T<i64>(sizeX_, sizeY_);
}

template<typename T>
T* Array2d<T>::data() {
	return data_;
}

template<typename T>
const T* Array2d<T>::data() const {
	return data_;
}

template<typename T>
Array2d<T>::Array2d(i64 sizeX, i64 sizeY)
	: sizeX_(sizeX)
	, sizeY_(sizeY) {
	data_ = reinterpret_cast<T*>(operator new(sizeX * sizeY * sizeof(T)));
}
