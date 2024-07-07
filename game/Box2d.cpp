#include "Box2d.hpp"

Aabb bShape_GetAABB(b2ShapeId shapeId) {
	const auto aabb = b2Shape_GetAABB(shapeId);
	return Aabb(toVec2(aabb.lowerBound), toVec2(aabb.upperBound));
}

bool bShape_TestPoint(b2ShapeId shapeId, Vec2 p) {
	return b2Shape_TestPoint(shapeId, fromVec2(p));
}
