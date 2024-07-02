#pragma once

#include "MatrixView.hpp"
#include <sstream>
#include <iomanip>

inline size_t number_of_digits(double n) {
	std::ostringstream strs;

	strs << n;
	return strs.str().size();
}
#include <iostream>
template<typename T>
void matrixPrint(const MatrixView<T>& matrix) {
	constexpr size_t nmax{ 100 };
	size_t max_len_per_column[nmax];
	const auto n = matrix.sizeX();
	const auto m = matrix.sizeY();
	for (size_t i = 0; i < n; ++i) {
		size_t max_len{};

		for (size_t j = 0; j < m; ++j)
			if (const auto num_length{ number_of_digits(matrix(i, j)) }; num_length > max_len)
				max_len = num_length;

		max_len_per_column[i] = max_len;
	}

	for (size_t j = 0; j < m; ++j)
		for (size_t i = 0; i < n; ++i)
			std::cout << (i == 0 ? "\n| " : "") << std::setw(max_len_per_column[i]) << matrix(i, j) << (i == n - 1 ? " |" : " ");

	std::cout << '\n';
}

template<typename T>
void matrixPrint(const Matrix<T>& matrix) {
	matrixPrint(matrixViewFromConstMatrix(matrix));
}

template<typename T>
Matrix<T> matrixTranspose(const Matrix<T>& m) {
	auto out = Matrix<T>::uninitialized(m.sizeY(), m.sizeX());
	for (i64 i = 0; i < m.sizeX(); i++) {
		for (i64 j = 0; j < m.sizeY(); j++) {
			out(j, i) = m(i, j);
		}
	}
	return out;
}