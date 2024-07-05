#pragma once

#include "Array2d.hpp"

// I think using a view instead of just passing pointers is better, because it automatically detects size errors using asserts.

template<typename T>
void swapRows(View2d<T>& m, i64 a, i64 b);
template<typename T>
void addRows(View2d<T>& m, i64 lhsAndOutputRow, i64 rhsRow, const T& rhsScale);
template<typename T>
void matrixMultiply(const View2d<const T>& lhs, const View2d<const T>& rhs, View2d<T> output);
template<typename T>
Array2d<T> operator*(const Array2d<T>& lhs, const Array2d<T>& rhs);
template<typename T>
void subtract(const View2d<const T>& lhs, const View2d<const T>& rhs, View2d<T> output);
template<typename T>
Array2d<T> operator-(const Array2d<T>& lhs, const Array2d<T>& rhs);

template<typename T>
void swapRows(View2d<T>& m, i64 a, i64 b) {
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
void addRows(View2d<T>& m, i64 lhsAndOutputRow, i64 rhsRow, const T& rhsScale) {
	ASSERT(m.isYInRange(lhsAndOutputRow));
	ASSERT(m.isYInRange(rhsRow));

	for (i64 i = 0; i < m.sizeX(); i++) {
		m(i, lhsAndOutputRow) += rhsScale * m(i, rhsRow);
	}
}

template<typename T>
void matrixMultiply(const View2d<const T>& lhs, const View2d<const T>& rhs, View2d<T> output) {
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
Array2d<T> operator*(const Array2d<T>& lhs, const Array2d<T>& rhs) {
	auto output = Array2d<T>::uninitialized(rhs.sizeX(), lhs.sizeY());
	matrixMultiply(constView2d(lhs), constView2d(rhs), view2d(output));
	return output;
}

template<typename T>
void subtract(const View2d<const T>& lhs, const View2d<const T>& rhs, View2d<T> output) {
	ASSERT(lhs.size() == rhs.size());
	ASSERT(rhs.size() == output.size());

	for (i64 y = 0; y < output.sizeY(); y++) {
		for (i64 x = 0; x < output.sizeX(); x++) {
			output(x, y) = lhs(x, y) - rhs(x, y);
		}
	}
}

template<typename T>
Array2d<T> operator-(const Array2d<T>& lhs, const Array2d<T>& rhs) {
	auto output = Array2d<T>::uninitialized(rhs.size());
	subtract(constView2d(lhs), constView2d(rhs), view2d(output));
	return output;
}