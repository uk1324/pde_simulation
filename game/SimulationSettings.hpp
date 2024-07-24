#pragma once

#include <engine/Math/Vec2.hpp>

// Should the settings be shared between the simulation and the editor or should they be restored to the ones in the editor when the scene is switched?
// If the former then how should undo redo work?

enum class SimulationBoundaryCondition {
	REFLECTING,
	ABSORBING,
};

struct SimulationSettings {
	static SimulationSettings makeDefault();

	bool paused;
	f32 timeScale;
	i32 rigidbodySimulationSubStepCount;
	i32 waveEquationSimulationSubStepCount;

	SimulationBoundaryCondition topBoundaryCondition;
	SimulationBoundaryCondition bottomBoundaryCondition;
	SimulationBoundaryCondition leftBoundaryCondition;
	SimulationBoundaryCondition rightBoundaryCondition;

	f32 dampingPerSecond;
	f32 speedDampingPerSecond;

	Vec2 gravity;
};

void simulationSettingsGui(SimulationSettings& settings);