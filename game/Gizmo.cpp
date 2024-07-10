#include <game/Gizmo.hpp>
#include <engine/Math/Constants.hpp>
#include <engine/Math/LineSegment.hpp>

Gizmo::Result Gizmo::update(const Camera& camera, Vec2 gizmoPosition, Vec2 cursorPos, bool grabDown, bool grabUp, bool grabHeld) {
	const auto arrows = getArrows(camera);
	const auto xDirection = arrows.xArrow.normalized();
	const auto yDirection = arrows.yArrow.normalized();

	const f32 xAxisGizmoDistance = distanceLineSegmentToPoint(gizmoPosition, gizmoPosition + arrows.xArrow, cursorPos);
	const f32 yAxisGizmoDistance = distanceLineSegmentToPoint(gizmoPosition, gizmoPosition + arrows.yArrow, cursorPos);
	const f32 centerDistance = gizmoPosition.distanceTo(cursorPos);

	const auto arrowWidth = this->arrowWidth(camera);
	const auto acivationDistance = (arrowWidth / 2.0f) * 4.0f;

	Vec2 translation(0.0f);

	if (grabDown) {
		currentState = State::GRAB_START;
		if (centerDistance < acivationDistance) {
			grabbedPart = Part::CENTER;
		} else if (xAxisGizmoDistance < acivationDistance) {
			grabbedPart = Part::X_AXIS;
		} else if (yAxisGizmoDistance < acivationDistance) {
			grabbedPart = Part::Y_AXIS;
		}
		cursorPositionAtGrabStart = cursorPos;
	} else if (currentState == State::GRAB_START && grabHeld) {
		currentState = State::GRABBED;
	}
		
	if (currentState == State::GRABBED) {
			
		const auto cursorChangeSinceGrab = cursorPos - cursorPositionAtGrabStart;
		switch (grabbedPart) {
			using enum Part;

		case CENTER:
			translation = cursorChangeSinceGrab;
			break;

		case X_AXIS:
			translation = xDirection * dot(cursorChangeSinceGrab, xDirection);
			break;

		case Y_AXIS:
			translation = yDirection * dot(cursorChangeSinceGrab, yDirection);
			break;

		case NONE:
			break;
		}
	}

	if (currentState == State::GRABBED && grabUp) {
		grabbedPart = Part::NONE;
		currentState = State::GRAB_END;
	} else if (currentState == State::GRAB_END) {
		currentState = State::INACTIVE;
	}

	return Result{
		.translation = translation,
		.rotation = 0.0f,
		.state = currentState,
	};
}

Gizmo::Arrows Gizmo::getArrows(const Camera& camera) const {
	const auto length = arrowLength(camera);
	return Arrows{ .xArrow = Vec2::fromPolar(angle, length), .yArrow = Vec2::fromPolar(angle + PI<f32> / 2.0f, length) };
}

f32 Gizmo::arrowLength(const Camera& camera) const {
	return 0.2f / camera.zoom;
}

f32 Gizmo::arrowWidth(const Camera& camera) const {
	return 0.009 / camera.zoom;
}

