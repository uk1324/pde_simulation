#include "GridUtils.hpp"

Vec2T<i64> gridClamp(Vec2T<i64> pos, Vec2T<i64> gridSize) {
	if (pos.x < 0) {
		pos.x = 0;
	}
	if (pos.y < 0) {
		pos.y = 0;
	}
	if (pos.x >= gridSize.x) {
		pos.x = gridSize.x - 1;
	}
	if (pos.y >= gridSize.y) {
		pos.y = gridSize.y - 1;
	}
	return pos;
}

GridAabb aabbToClampedGridAabb(Aabb aabb, const Aabb& gridBounds, Vec2T<i64> gridSize) {
	const auto gridBoundsSize = gridBounds.size();
	{
		aabb.min -= gridBounds.min;
		aabb.min /= gridBounds.size();
		aabb.min *= Vec2(gridSize);
		aabb.min = aabb.min.applied(floor);
	}
	{
		aabb.max -= gridBounds.min;
		aabb.max /= gridBounds.size();
		aabb.max *= Vec2(gridSize);
		aabb.max = aabb.max.applied(ceil);
	}
	return GridAabb(gridClamp(Vec2T<i64>(aabb.min), gridSize), gridClamp(Vec2T<i64>(aabb.max), gridSize));
}

Vec2T<i64> positionClampedToGridPosition(Vec2 pos, const Aabb& gridBounds, Vec2T<i64> gridSize) {
	pos -= gridBounds.min;
	pos /= gridBounds.size();
	pos *= Vec2(gridSize);
	pos = pos.applied(floor);
	return Vec2T<i64>(pos);
}

std::optional<Vec2T<i64>> positionToGridPositionInGrid(Vec2 pos, const Aabb& gridBounds, Vec2T<i64> gridSize) {
	auto gridPos = positionClampedToGridPosition(pos, gridBounds, gridSize);
	//gridPos.y = gridSize.y - 1 - gridPos.y;
	if (gridPos.x >= 0 && gridPos.y >= 0 && gridPos.x < gridSize.x && gridPos.y < gridSize.y) {
		return gridPos;
	}
	return std::nullopt;
}

//Vec2 gridPositionToCellBottomLeft(Vec2T<i64> gridPosition, Vec2 gridBoundsMin, f32 cellSize) {
//	return gridPositionToCellBottomLeft(gridPosition.x, gridPosition.y, gridBoundsMin, cellSize);
//}
//
//Vec2 gridPositionToCellBottomLeft(i64 x, i64 y, Vec2 gridBoundsMin, f32 cellSize) {
//	return Vec2(x, y) * cellSize + gridBoundsMin;
//}
//
//Vec2 gridPositionToCellCenter(Vec2T<i64> gridPosition, Vec2 gridBoundsMin, f32 cellSize) {
//	return gridPositionToCellCenter(gridPosition.x, gridPosition.y, gridBoundsMin, cellSize);
//}
//
//Vec2 gridPositionToCellCenter(i64 x, i64 y, Vec2 gridBoundsMin, f32 cellSize) {
//	//return gridPositionToCellBottomLeft(x, y, gridBoundsMin, cellSize) + Vec2(0.5f) * cellSize;
//	return gridPositionToCellBottomLeft(x, y, gridBoundsMin, cellSize) + Vec2(-0.5f, -0.5f) * cellSize;
//}

GridAabb::GridAabb(Vec2T<i64> min, Vec2T<i64> max) 
	: min(min)
	, max(max) {}
