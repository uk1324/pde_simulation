#include "GridUtils.hpp"

std::optional<Vec2T<i64>> positionToGridPosition(Vec2 pos, const Aabb& gridBounds, Vec2T<i64> gridSize) {
	pos -= gridBounds.min;
	pos /= gridBounds.size();
	pos *= Vec2(gridSize);
	pos = pos.applied(floor);
	auto gridPos = Vec2T<i64>(pos);
	//gridPos.y = gridSize.y - 1 - gridPos.y;
	if (gridPos.x >= 0 && gridPos.y >= 0 && gridPos.x < gridSize.x && gridPos.y < gridSize.y) {
		return gridPos;
	}
	return std::nullopt;
}
