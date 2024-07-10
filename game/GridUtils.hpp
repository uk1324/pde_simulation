#pragma once

#include <engine/Math/Aabb.hpp>

struct GridAabb {
	GridAabb(Vec2T<i64> min, Vec2T<i64> max);

	Vec2T<i64> min;
	Vec2T<i64> max;
};

Vec2T<i64> gridClamp(Vec2T<i64> pos, Vec2T<i64> gridSize);

GridAabb aabbToClampedGridAabb(Aabb aabb, const Aabb& gridBounds, Vec2T<i64> gridSize);

// Doesn't do bounds checking
Vec2T<i64> positionToGridPosition(Vec2 pos, const Aabb& gridBounds, Vec2T<i64> gridSize);
Vec2T<i64> positionClampedToGridPosition(Vec2 pos, const Aabb& gridBounds, Vec2T<i64> gridSize);
// Does bounds checking
std::optional<Vec2T<i64>> positionToGridPositionInGrid(Vec2 pos, const Aabb& gridBounds, Vec2T<i64> gridSize);

//Vec2 gridPositionToCellBottomLeft(Vec2T<i64> gridPosition, Vec2 gridBoundsMin, f32 cellSize);
//Vec2 gridPositionToCellBottomLeft(i64 x, i64 y, Vec2 gridBoundsMin, f32 cellSize);
//
//Vec2 gridPositionToCellCenter(Vec2T<i64> gridPosition, Vec2 gridBoundsMin, f32 cellSize);
//Vec2 gridPositionToCellCenter(i64 x, i64 y, Vec2 gridBoundsMin, f32 cellSize);