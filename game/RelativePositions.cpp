#include "RelativePositions.hpp"
#include <engine/Math/Rotation.hpp>

Vec2 calculatePositionFromRelativePosition(Vec2 positionRelativeToObject, Vec2 objectPosition, f32 objectRotation) {
	Vec2 pos = positionRelativeToObject;
	pos *= Rotation(objectRotation);
	pos += objectPosition;
	return pos;
}

Vec2 calculateRelativePositionFromPosition(Vec2 absolutePosition, Vec2 objectPosition, f32 objectRotation) {
	Vec2 pos = absolutePosition;
	pos -= objectPosition;
	pos *= Rotation(-objectRotation);
	return pos;
}
