#pragma once

#include <engine/Math/Aabb.hpp>

std::array<Vec2, 4> aabbVertices(const Aabb& aabb);
Vec2 regularPolygonVertex(Vec2 center, f32 radius, f32 firstVertexAngle, i32 index, i32 vertexCount);
// Made this so I don't need to copy the code multiple times.
i32 cappedLineOutlineVertexCount(i32 capPointCount);
Vec2 cappedLineOutlineVertex(Vec2 endpoint0, Vec2 endpoint1, f32 width, i32 i, i32 capPointCount);