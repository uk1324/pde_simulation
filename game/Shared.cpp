#include <game/Shared.hpp>
#include <gfx2d/CameraUtils.hpp>
#include <game/Constants.hpp>
#include <Gui.hpp>

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

bool gameBeginPropertyEditor(const char* id) {
	return Gui::beginPropertyEditor(id, Gui::PropertyEditorFlags::TableStetchToFit);
}

bool emitterSettings(f32& strength, bool& oscillate, f32& period, f32& phaseOffset) {
	bool modificationFinished = false;

	Gui::inputFloat("strength", strength);
	modificationFinished |= ImGui::IsItemDeactivatedAfterEdit();
	modificationFinished |= Gui::checkbox("oscillate", oscillate);
	if (oscillate) {
		Gui::inputFloat("period", period);
		modificationFinished |= ImGui::IsItemDeactivatedAfterEdit();
		Gui::inputFloat("phase offset", phaseOffset);
		modificationFinished |= ImGui::IsItemDeactivatedAfterEdit();
	}

	return modificationFinished;
}