#pragma once

#include <engine/Math/Aabb.hpp>

std::optional<Vec2T<i64>> positionToGridPosition(Vec2 pos, const Aabb& gridBounds, Vec2T<i64> gridSize);