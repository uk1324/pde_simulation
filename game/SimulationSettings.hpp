#pragma once

// Should the settings be shared between the simulation and the editor or should they be restored to the ones in the editor when the scene is switched?
// If the former then how should undo redo work?

enum class SimulationBoundaryCondition {
	REFLECTING,
	ABSORBING,
	PERIODIC,
};

struct SimulationSettings {
	f32 speedScale;
	i32 rigidbodySimulationSubstepCount;
	i32 waveEquationSimulationSubstepCount;

};