#include "InputButton.hpp"
#include <engine/Input/Input.hpp>
#include <engine/Input/InputUtils.hpp>
#include <imgui/imgui.h>

InputButton::InputButton(KeyCode keycode) 
	: keycode(keycode) {}

bool inputButtonGui(std::optional<InputButton>& button, bool& watingForKeyPress) {
	bool modificationFinished = false;

	ImGui::PushID(&watingForKeyPress);
	if (!button.has_value()) {
		if (watingForKeyPress) {
			ImGui::Text("press key, esc to cancel");
			if (Input::isKeyDown(KeyCode::ESCAPE)) {
				modificationFinished = true;
				watingForKeyPress = false;
			} else if (Input::lastKeycodeDownThisFrame().has_value()) {
				button = InputButton(*Input::lastKeycodeDownThisFrame());
				modificationFinished = true;
				watingForKeyPress = false;
			}

		} else {
			if (ImGui::Button("no key bound", ImVec2(-1.0f, 0.0f))) {
				watingForKeyPress = true;
			}
		}
	} else {
		if (ImGui::Button(toString(button->keycode), ImVec2(-1.0f, 0.0f))) {
			button = std::nullopt;
			watingForKeyPress = true;
		}
	}
	ImGui::PopID();

	return modificationFinished;
}

bool inputButtonIsHeld(const InputButton& button) {
	return Input::isKeyHeld(button.keycode);
}
