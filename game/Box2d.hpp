#pragma once

#include <box2d/box2d.h>
#include <engine/Math/Aabb.hpp>

inline Vec2 toVec2(b2Vec2 v) {
	return Vec2(v.x, v.y);
}

inline b2Vec2 fromVec2(Vec2 v) {
	return b2Vec2{ v.x, v.y };
}

Aabb bShape_GetAABB(b2ShapeId shapeId);

bool bShape_TestPoint(b2ShapeId shapeId, Vec2 p);