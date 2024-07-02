#pragma once

#include <Types.hpp>
#include <engine/Math/Vec2.hpp>

template<typename T>
struct Matrix {
	static Matrix uninitialized(i64 sizeX, i64 sizeY);
	static Matrix uninitialized(Vec2T<i64> size);
	static Matrix zero(i64 sizeX, i64 sizeY);
	static Matrix zero(Vec2T<i64> size);
	Matrix(const Matrix&) = delete;
	Matrix(Matrix&& other) noexcept;
	~Matrix();

	Matrix& operator=(const Matrix&) = delete;
	Matrix& operator=(Matrix&&) noexcept;

	Matrix clone() const;

	T& operator()(i64 x, i64 y);
	const T& operator()(i64 x, i64 y) const;

	i64 sizeX() const;
	i64 sizeY() const;
	Vec2T<i64> size() const;
	T* data();
	const T* data() const;

private:
	Matrix(i64 sizeX, i64 sizeY);

	i64 sizeX_;
	i64 sizeY_;
	T* data_;
};

template<typename T>
Matrix<T> Matrix<T>::uninitialized(i64 sizeX, i64 sizeY) {
	return Matrix<T>(sizeX, sizeY);
}

template<typename T>
Matrix<T> Matrix<T>::uninitialized(Vec2T<i64> size) {
	return uninitialized(size.x, size.y);
}

template<typename T>
Matrix<T> Matrix<T>::zero(i64 sizeX, i64 sizeY) {
	auto result = Matrix<T>::uninitialized(sizeX, sizeY);
	for (i64 y = 0; y < sizeY; y++) {
		for (i64 x = 0; x < sizeX; x++) {
			result(x, y) = T(0.0);
		}
	}
	return result;
}

template<typename T>
Matrix<T> Matrix<T>::zero(Vec2T<i64> size) {
	return zero(size.x, size.y);
}

template<typename T>
Matrix<T>::Matrix(Matrix&& other) noexcept 
	: sizeX_(other.sizeX_)
	, sizeY_(other.sizeY_)
	, data_(other.data_) {
	other.data_ = nullptr;
	other.sizeX_ = 0;
	other.sizeY_ = 0;
}

template<typename T>
Matrix<T>::~Matrix() {
	operator delete(data_);
}

template<typename T>
Matrix<T>& Matrix<T>::operator=(Matrix&& other) noexcept {
	operator delete(data_);
	sizeX_ = other.sizeX_;
	sizeY_ = other.sizeY_;
	data_ = other.data_;
	other.data_ = nullptr;
	other.sizeX_ = 0;
	other.sizeY_ = 0;
}

template<typename T>
Matrix<T> Matrix<T>::clone() const {
	auto m = Matrix<T>::uninitialized(size());
	memcpy(m.data(), data_, sizeX_ * sizeY_ * sizeof(T));
	return m;
}

template<typename T>
T& Matrix<T>::operator()(i64 x, i64 y) {
	return data_[y * sizeX_ + x];
}

template<typename T>
const T& Matrix<T>::operator()(i64 x, i64 y) const {
	return data_[y * sizeX_ + x];
}

template<typename T>
i64 Matrix<T>::sizeX() const {
	return sizeX_;
}

template<typename T>
i64 Matrix<T>::sizeY() const {
	return sizeY_;
}

template<typename T>
Vec2T<i64> Matrix<T>::size() const {
	return Vec2T<i64>(sizeX_, sizeY_);
}

template<typename T>
T* Matrix<T>::data() {
	return data_;
}

template<typename T>
const T* Matrix<T>::data() const {
	return data_;
}

template<typename T>
Matrix<T>::Matrix(i64 sizeX, i64 sizeY)
	: sizeX_(sizeX)
	, sizeY_(sizeY) {
	data_ = reinterpret_cast<T*>(operator new(sizeX * sizeY * sizeof(T)));
}
