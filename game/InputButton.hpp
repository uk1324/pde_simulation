#pragma once

#include <engine/Input/KeyCode.hpp>
#include <optional>

struct InputButton {
	InputButton(KeyCode keycode);

	KeyCode keycode;

	bool operator==(const InputButton&) const = default;
};

bool inputButtonGui(std::optional<InputButton>& button, bool& watingForKeyPress);
bool inputButtonIsHeld(const InputButton& button);