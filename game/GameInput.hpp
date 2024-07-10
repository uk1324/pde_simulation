#pragma once

#include <engine/Math/Vec2.hpp>

struct GameInput {
	bool upButtonHeld;
	bool downButtonHeld;
	bool leftButtonHeld;
	bool rightButtonHeld;
	Vec2 cursorPos;
	bool cursorLeftDown;
	bool cursorLeftUp;
	bool cursorLeftHeld;
	bool cursorRightDown;
	bool undoDown;
	bool redoDown;
	bool ctrlHeld;
	bool deleteDown;
	bool switchModeDown;
};
