#pragma once

#include <game/GameInput.hpp>
#include <gfx2d/Camera.hpp>

void cameraMovement(Camera& camera, const GameInput& input, f32 dt);

// Putting this here so id conflicts don't occur.
const auto editorSimulationSettingsWindowName = "simulation###editorSimulation";
const auto editorEditorSettingsWindowName = "editor";
const auto simulationSimulationSettingsWindowName = "simulation###simulationSimulation";

bool gameBeginPropertyEditor(const char* id);
bool emitterSettings(f32& strength, bool& oscillate, f32& period, f32& phaseOffset);

const auto EMITTER_STRENGTH_DEFAULT = 5.0f;