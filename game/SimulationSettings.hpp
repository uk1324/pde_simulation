#pragma once

#include <Types.hpp>

// Should the settings be shared between the simulation and the editor or should they be restored to the ones in the editor when the scene is switched?
// If the former then how should undo redo work?

enum class SimulationBoundaryCondition {
	REFLECTING,
	ABSORBING,
};

struct SimulationSettings {
	static SimulationSettings makeDefault();

	f32 timeScale;
	i32 rigidbodySimulationSubStepCount;
	i32 waveEquationSimulationSubStepCount;

	SimulationBoundaryCondition topBoundaryCondition;
	SimulationBoundaryCondition bottomBoundaryCondition;
	SimulationBoundaryCondition leftBoundaryCondition;
	SimulationBoundaryCondition rightBoundaryCondition;

	bool dampingEnabled;
	f32 dampingPerSecond;

	bool speedDampingEnabled;
	f32 speedDampingPerSecond;
};