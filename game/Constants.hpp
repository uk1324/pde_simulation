#pragma once

#include <engine/Math/Aabb.hpp>

namespace Constants {
	constexpr f32 CELL_SIZE = 0.1f;
	constexpr Vec2T<i64> DEFAULT_GRID_SIZE(300, 200);
	constexpr f32 CAMERA_SPEED = 10.0f;
	constexpr f32 EMITTER_DISPLAY_RADIUS = 0.25f;

	Aabb gridBounds(Vec2T<i64> gridSize);
};