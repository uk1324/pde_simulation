#pragma once

#include <gfx2d/Camera.hpp>
#include <engine/Math/Vec2.hpp>

struct Gizmo {
	f32 angle;

	enum class State {
		INACTIVE,
		GRAB_START,
		GRAB_END,
		GRABBED,
	};
	State currentState = State::INACTIVE;

	struct Result {
		Vec2 translation;
		f32 rotation;
		State state;
	};

	Result update(const Camera& camera, Vec2 gizmoPosition, Vec2 cursorPos, bool grabDown, bool grabUp, bool grabHeld);

	enum class Part {
		NONE,
		CENTER,
		X_AXIS,
		Y_AXIS,
	};

	Vec2 cursorPositionAtGrabStart = Vec2(0.0f);
	Part grabbedPart = Part::NONE;

	f32 arrowLength(const Camera& camera) const;
	f32 arrowWidth(const Camera& camera) const;

	struct Arrows {
		Vec2 xArrow;
		Vec2 yArrow;
	} getArrows(const Camera& camera) const;
};
