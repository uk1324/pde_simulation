#pragma once

#include <Types.hpp>

template<typename T>
struct View2d {
	static View2d empty();
	//static const MatrixView<const T> fromConstMatrix(const Matrix<T>& matrix);
	View2d(T* data, i64 sizeX, i64 sizeY);
	//MatrixView(Matrix<T>& matrix);

	T& operator()(i64 x, i64 y);
	const T& operator()(i64 x, i64 y) const;

	bool isXInRange(i64 x) const;
	bool isYInRange(i64 y) const;

	i64 sizeX() const;
	i64 sizeY() const;
	Vec2T<i64> size() const;
	T* data();
	const T* data() const;

private:
	T* data_;
	i64 sizeX_;
	i64 sizeY_;
};

//template<typename T>
//View2d<T> matrixViewFromMatrix(Array2d<T>& matrix) {
//	return View2d<T>(matrix.data(), matrix.sizeX(), matrix.sizeY());
//}

// This is impossible, becuase MatrixView<T> can only take non const pointers. If you could somehow return a const MatrixView<T> then you could copy it and modify it so there needs to be an matrixViewFromConstMatrix.
//template<typename T>
//MatrixView<T> matrixViewFromMatrix(const Matrix<T>& matrix) {
//	return MatrixView<T>(matrix.data(), matrix.sizeX(), matrix.sizeY());
//}
// If you didn't use a struct I guess you would need to pass const T* anyway.

//template<typename T>
//View2d<const T> matrixViewFromConstMatrix(const Array2d<T>& matrix) {
//	return View2d<const T>(matrix.data(), matrix.sizeX(), matrix.sizeY());
//}

//template<typename T>
//View2d<T> matrixViewEmpty() {
//	return View2d<T>(nullptr, 0, 0);
//}

//template<typename T>
//MatrixView<const T> matrixViewFromConstMatrix(const Matrix<T>& matrix) {
//	return MatrixView<const T>(matrix.data(), matrix.sizeX(), matrix.sizeY());
//}

//template<typename T>
//const MatrixView<const T> MatrixView<T>::fromConstMatrix(const Matrix<T>& matrix) {
//	return MatrixView(matrix.data(), matrix.sizeX(), matrix.sizeY());
//}

template<typename T>
View2d<T> View2d<T>::empty() {
	return View2d(nullptr, 0, 0);
}

template<typename T>
View2d<T>::View2d(T* data, i64 sizeX, i64 sizeY) 
	: data_(data)
	, sizeX_(sizeX) 
	, sizeY_(sizeY) {}

template<typename T>
T& View2d<T>::operator()(i64 x, i64 y) {
	return data_[y * sizeX_ + x];
}

template<typename T>
const T& View2d<T>::operator()(i64 x, i64 y) const {
	ASSERT(isXInRange(x));
	ASSERT(isYInRange(y));
	return data_[y * sizeX_ + x];
}

template<typename T>
bool View2d<T>::isXInRange(i64 x) const {
	return x >= 0 && x < sizeX_;
}

template<typename T>
bool View2d<T>::isYInRange(i64 y) const {
	return y >= 0 && y < sizeY_;
}

template<typename T>
i64 View2d<T>::sizeX() const {
	return	sizeX_;
}

template<typename T>
i64 View2d<T>::sizeY() const {
	return sizeY_;
}

template<typename T>
Vec2T<i64> View2d<T>::size() const {
	return Vec2T<i64>(sizeX_, sizeY_);
}

template<typename T>
T* View2d<T>::data() {
	return data_;
}

template<typename T>
inline const T* View2d<T>::data() const {
	return data_;
}