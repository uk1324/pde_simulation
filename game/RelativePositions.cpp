#include "RelativePositions.hpp"
#include <engine/Math/Rotation.hpp>

Vec2 calculateRelativePosition(Vec2 positionRelativeToObject, Vec2 objectPosition, f32 objectRotation) {
	Vec2 pos = positionRelativeToObject;
	pos *= Rotation(objectRotation);
	pos += objectPosition;
	return pos;
}
