#pragma once

#include <engine/Math/Vec2.hpp>

Vec2 calculatePositionFromRelativePosition(Vec2 positionRelativeToObject, Vec2 objectPosition, f32 objectRotation);
Vec2 calculateRelativePositionFromPosition(Vec2 absolutePosition, Vec2 objectPosition, f32 objectRotation);