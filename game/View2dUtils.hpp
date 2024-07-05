#pragma once

#include "Array2d.hpp"

template<typename T>
T max(const View2d<const T>& v);
template<typename T>
T max(const Array2d<T>& m);
template<typename T>
T min(const View2d<const T>& v);
template<typename T>
T min(const Array2d<T>& m);

template<typename T>
void fill(View2d<T> v, const T& value);
template<typename T>
void fill(Array2d<T>& v, const T& value);

template<typename T>
T max(const View2d<const T>& v) {
	if (v.sizeX() <= 0 && v.sizeY() <= 0) {
		return NAN;
	}
	
	T max = v.data()[0];
	for (i64 i = 1; i < v.sizeX() * v.sizeY(); i++) {
		const auto& value = v.data()[i];
		if (value > max) {
			max = value;
		}
	}
	return max;
}

template<typename T>
T max(const Array2d<T>& m) {
	return max(constView2d(m));
}

template<typename T>
T min(const View2d<const T>& v) {
	if (v.sizeX() <= 0 && v.sizeY() <= 0) {
		return NAN;
	}

	T min = v.data()[0];
	for (i64 i = 1; i < v.sizeX() * v.sizeY(); i++) {
		const auto& value = v.data()[i];
		if (value < min) {
			min = value;
		}
	}
	return min;
}

template<typename T>
T min(const Array2d<T>& m) {
	return min(constView2d(m));
}

template<typename T>
void fill(View2d<T> v, const T& value) {
	for (i64 i = 0; i < v.sizeX() * v.sizeY(); i++) {
		v.data()[i] = value;
	}
}

template<typename T>
void fill(Array2d<T>& v, const T& value) {
	fill(view2d(v), value);
}
