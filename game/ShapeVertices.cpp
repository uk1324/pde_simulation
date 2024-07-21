#include "ShapeVertices.hpp"
#include <engine/Math/Constants.hpp>

std::array<Vec2, 4> aabbVertices(const Aabb& aabb) {
	return std::array<Vec2, 4>{
		aabb.min,
		Vec2(aabb.max.x, aabb.min.y),
		aabb.max,
		Vec2(aabb.min.x, aabb.max.y),
	};
}

Vec2 regularPolygonVertex(Vec2 center, f32 radius, f32 firstVertexAngle, i32 index, i32 vertexCount) {
	f32 angle = firstVertexAngle + (f32(index) / f32(vertexCount)) * TAU<f32>;
	return center + Vec2(cos(angle), sin(angle)) * radius;
}

i32 cappedLineOutlineVertexCount(i32 capPointCount) {
	return 2 * capPointCount;
}

Vec2 cappedLineOutlineVertex(Vec2 endpoint0, Vec2 endpoint1, f32 width, i32 i, i32 capPointCount) {
	const auto angle = (endpoint1 - endpoint0).angle();

	if (i < capPointCount) {
		const auto t = f32(i) / f32(capPointCount - 1);
		const auto a = angle + t * PI<f32> - PI<f32> / 2.0f;
		return endpoint1 + Vec2::fromPolar(a, width / 2.0f);
	} 

	i -= capPointCount;
	const auto t = f32(i) / f32(capPointCount - 1);
	const auto a = angle - PI<f32> * 1.5f + t * PI<f32>;
	return endpoint0 + Vec2::fromPolar(a, width / 2.0f);
}
