#pragma once

#include <engine/Input/KeyCode.hpp>
#include <optional>

struct InputButton {
	InputButton(KeyCode keycode);

	KeyCode keycode;
};

void inputButtonGui(std::optional<InputButton>& button, bool& watingForKeyPress);
bool inputButtonIsHeld(const InputButton& button);