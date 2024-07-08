#pragma once

#include <engine/Math/Aabb.hpp>

namespace Constants {
	constexpr f32 CELL_SIZE = 0.1f;
	constexpr Vec2T<i64> DEFAULT_GRID_SIZE(300, 200);

	Aabb gridBounds(Vec2T<i64> gridSize);
};