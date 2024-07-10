#include <game/Shared.hpp>
#include <gfx2d/CameraUtils.hpp>
#include <game/Constants.hpp>

void cameraMovement(Camera& camera, const GameInput& input, f32 dt) {
	zoomOnCursorPos(camera, dt);

	Vec2 movement(0.0f);

	if (input.rightButtonHeld) {
		movement.x += 1.0f;
	}
	if (input.leftButtonHeld) {
		movement.x -= 1.0f;
	}
	if (input.upButtonHeld) {
		movement.y += 1.0f;
	}
	if (input.downButtonHeld) {
		movement.y -= 1.0f;
	}

	camera.pos += movement.normalized() * Constants::CAMERA_SPEED * dt;
}