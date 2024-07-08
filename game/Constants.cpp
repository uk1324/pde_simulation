#include "Constants.hpp"
#pragma once

Aabb Constants::gridBounds(Vec2T<i64> gridSize) {
	return Aabb(Vec2(0.0f), Vec2(gridSize) * CELL_SIZE);
}
