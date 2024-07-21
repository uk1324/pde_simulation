#pragma once

#include <engine/Math/Aabb.hpp>

std::array<Vec2, 4> aabbVertices(const Aabb& aabb);
Vec2 regularPolygonVertex(Vec2 center, f32 radius, f32 firstVertexAngle, i32 index, i32 vertexCount);