#pragma once

#include "Matrix.hpp"

template<typename T>
struct MatrixView {
	//static const MatrixView<const T> fromConstMatrix(const Matrix<T>& matrix);
	MatrixView(T* data, i64 sizeX, i64 sizeY);
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

template<typename T>
MatrixView<T> matrixViewFromMatrix(Matrix<T>& matrix) {
	return MatrixView<T>(matrix.data(), matrix.sizeX(), matrix.sizeY());
}

// This is impossible, becuase MatrixView<T> can only take non const pointers. If you could somehow return a const MatrixView<T> then you could copy it and modify it so there needs to be an matrixViewFromConstMatrix.
//template<typename T>
//MatrixView<T> matrixViewFromMatrix(const Matrix<T>& matrix) {
//	return MatrixView<T>(matrix.data(), matrix.sizeX(), matrix.sizeY());
//}
// If you didn't use a struct I guess you would need to pass const T* anyway.

template<typename T>
MatrixView<const T> matrixViewFromConstMatrix(const Matrix<T>& matrix) {
	return MatrixView<const T>(matrix.data(), matrix.sizeX(), matrix.sizeY());
}

template<typename T>
MatrixView<T> matrixViewEmpty() {
	return MatrixView<T>(nullptr, 0, 0);
}

//template<typename T>
//MatrixView<const T> matrixViewFromConstMatrix(const Matrix<T>& matrix) {
//	return MatrixView<const T>(matrix.data(), matrix.sizeX(), matrix.sizeY());
//}

//template<typename T>
//const MatrixView<const T> MatrixView<T>::fromConstMatrix(const Matrix<T>& matrix) {
//	return MatrixView(matrix.data(), matrix.sizeX(), matrix.sizeY());
//}

template<typename T>
MatrixView<T>::MatrixView(T* data, i64 sizeX, i64 sizeY) 
	: data_(data)
	, sizeX_(sizeX) 
	, sizeY_(sizeY) {}

template<typename T>
T& MatrixView<T>::operator()(i64 x, i64 y) {
	return data_[y * sizeX_ + x];
}

template<typename T>
const T& MatrixView<T>::operator()(i64 x, i64 y) const {
	ASSERT(isXInRange(x));
	ASSERT(isYInRange(y));
	return data_[y * sizeX_ + x];
}

template<typename T>
bool MatrixView<T>::isXInRange(i64 x) const {
	return x >= 0 && x < sizeX_;
}

template<typename T>
bool MatrixView<T>::isYInRange(i64 y) const {
	return y >= 0 && y < sizeY_;
}

template<typename T>
i64 MatrixView<T>::sizeX() const {
	return	sizeX_;
}

template<typename T>
i64 MatrixView<T>::sizeY() const {
	return sizeY_;
}

template<typename T>
Vec2T<i64> MatrixView<T>::size() const {
	return Vec2T<i64>(sizeX_, sizeY_);
}

template<typename T>
T* MatrixView<T>::data() {
	return data_;
}

template<typename T>
inline const T* MatrixView<T>::data() const {
	return data_;
}

template<typename T>
void swapRows(MatrixView<T>& m, i64 a, i64 b) {
	if (a == b) {
		return;
	}
	ASSERT(m.isYInRange(a));
	ASSERT(m.isYInRange(b));

	for (i64 i = 0; i < m.sizeX(); i++) {
		const auto temp = m(i, a);
		m(i, a) = m(i, b);
		m(i, b) = temp;
	}
}

template<typename T>
void addRows(MatrixView<T>& m, i64 lhsAndOutputRow, i64 rhsRow, const T& rhsScale) {
	ASSERT(m.isYInRange(lhsAndOutputRow));
	ASSERT(m.isYInRange(rhsRow));

	for (i64 i = 0; i < m.sizeX(); i++) {
		m(i, lhsAndOutputRow) += rhsScale * m(i, rhsRow);
	}
}

template<typename T>
void matrixMultiply(const MatrixView<const T>& lhs, const MatrixView<const T>& rhs, MatrixView<T> output) {
	// Matrix multiplicaiton just multiplies each column of rhs by lhs, because of this the output column count is the same as rhs column count.
	// The output dimension of the matrix is it's height so the height of the output is the height of the lhs.
	ASSERT(lhs.sizeX() == rhs.sizeY());
	const auto sumIndexMax = lhs.sizeX();
	ASSERT(output.sizeY() == lhs.sizeY());
	ASSERT(output.sizeX() == rhs.sizeX());

	for (i64 row = 0; row < output.sizeY(); row++) {
		for (i64 column = 0; column < output.sizeX(); column++) {
			output(column, row) = 0.0f;
			for (i64 i = 0; i < sumIndexMax; i++) {
				output(column, row) += lhs(i, row) * rhs(column, i);
			}
		}
	}
}

template<typename T>
void matrixMultiply(const Matrix<T>& lhs, const Matrix<T>& rhs, Matrix<T>& output) {
	matrixMultiply(matrixViewFromConstMatrix(lhs), matrixViewFromConstMatrix(rhs), matrixViewFromMatrix(output));
}

template<typename T>
Matrix<T> operator*(const Matrix<T>& lhs, const Matrix<T>& rhs) {
	auto output = Matrix<T>::uninitialized(rhs.sizeX(), lhs.sizeY());
	matrixMultiply(lhs, rhs, output);
	return output;
}

template<typename T>
void matrixSubtract(const MatrixView<const T>& lhs, const MatrixView<const T>& rhs, MatrixView<T> output) {
	ASSERT(lhs.size() == rhs.size());
	ASSERT(rhs.size() == output.size());

	for (i64 y = 0; y < output.sizeY(); y++) {
		for (i64 x = 0; x < output.sizeX(); x++) {
			output(x, y) = lhs(x, y) - rhs(x, y);
		}
	}
}

template<typename T>
Matrix<T> operator-(const Matrix<T>& lhs, const Matrix<T>& rhs) {
	auto output = Matrix<T>::uninitialized(rhs.size());
	matrixSubtract(matrixViewFromConstMatrix(lhs), matrixViewFromConstMatrix(rhs), matrixViewFromMatrix(output));
	return output;
}

template<typename T>
T matrixMax(const Matrix<T>& m) {
	if (m.sizeX() <= 0 && m.sizeY() <= 0) {
		return NAN;
	}

	T max = m.data()[0];
	for (i64 i = 1; i < m.sizeX() * m.sizeY(); i++) {
		const auto& value = m.data()[i];
		if (value > max) {
			max = value;
		}
	}
	return max;
}

template<typename T>
T matrixMin(const Matrix<T>& m) {
	if (m.sizeX() <= 0 && m.sizeY() <= 0) {
		return NAN;
	}

	T min = m.data()[0];
	for (i64 i = 1; i < m.sizeX() * m.sizeY(); i++) {
		const auto& value = m.data()[i];
		if (value < min) {
			min = value;
		}
	}
	return min;
}

template<typename T>
void matrixFill(Matrix<T>& m, const T& value) {
	for (i64 i = 0; i < m.sizeX() * m.sizeY(); i++) {
		m.data()[i] = value;
	}
}