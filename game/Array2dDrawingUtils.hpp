#pragma once

#include "Array2d.hpp"

template<typename T>
void fillCircle(Array2d<T>& mat, Vec2T<i64> center, i64 radius, const T& value) {
	/*const auto min = (cursorGridPos - r).clamped(Vec2T<i64>{ 1 }, fluid.gridSize - Vec2T<i64>{ 2 });
	const auto max = (cursorGridPos + r).clamped(Vec2T<i64>{ 1 }, fluid.gridSize - Vec2T<i64>{ 2 });*/

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

	//const auto minX = std::clamp(center.x - radius, 0ll, mat.size().x - 1);
	//const auto maxX = std::clamp(center.x + radius, 0ll, mat.size().x - 1);
	//const auto minY = std::clamp(center.y - radius, 0ll, mat.size().y - 1);
	//const auto maxY = std::clamp(center.y + radius, 0ll, mat.size().y - 1);

	//Vec2 centerPos = Vec2(center) + Vec2(0.5f);
	//for (i64 y = minY; y <= maxY; y++) {
	//	for (i64 x = minX; x <= maxX; x++) {
	//		Vec2 pixelPos = Vec2(x, y) + Vec2(0.5f);
	//		if (centerPos.distanceSquaredTo(pixelPos) <= radius * radius) {
	//			mat(x, y) = value;
	//		}
	//	}
	//}
};