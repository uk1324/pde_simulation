#include "ShapeVertices.hpp"

std::array<Vec2, 4> aabbVertices(const Aabb& aabb) {
	return std::array<Vec2, 4>{
		aabb.min,
		Vec2(aabb.max.x, aabb.min.y),
		aabb.max,
		Vec2(aabb.min.x, aabb.max.y),
	};
}
