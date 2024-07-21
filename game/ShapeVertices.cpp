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
