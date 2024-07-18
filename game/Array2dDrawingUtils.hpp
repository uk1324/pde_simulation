#pragma once

#include "Array2d.hpp"

template<typename T>
void fillCircle(Array2d<T>& mat, Vec2T<i64> center, i64 radius, const T& value) {
	const auto minX = std::clamp(center.x - radius, 0ll, mat.size().x - 1);
	const auto maxX = std::clamp(center.x + radius, 0ll, mat.size().x - 1);
	const auto minY = std::clamp(center.y - radius, 0ll, mat.size().y - 1);
	const auto maxY = std::clamp(center.y + radius, 0ll, mat.size().y - 1);

	for (i64 x = minX; x <= maxX; x++) {
		for (i64 y = minY; y <= maxY; y++) {
			if (Vec2(center).distanceTo(Vec2(x, y)) < radius) {
				mat(x, y) = value;
			}
		}
	}
};