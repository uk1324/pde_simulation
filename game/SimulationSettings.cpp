#include <game/SimulationSettings.hpp>

SimulationSettings SimulationSettings::makeDefault() {
	return SimulationSettings{
		.timeScale = 1.0f,
		.rigidbodySimulationSubStepCount = 4,
		.waveEquationSimulationSubStepCount = 1,
		.topBoundaryCondition = SimulationBoundaryCondition::REFLECTING,
		.bottomBoundaryCondition = SimulationBoundaryCondition::REFLECTING,
		.leftBoundaryCondition = SimulationBoundaryCondition::REFLECTING,
		.rightBoundaryCondition = SimulationBoundaryCondition::REFLECTING,
		.dampingEnabled = false,
		.dampingPerSecond = 0.95f,
		.speedDampingEnabled = false,
		.speedDampingPerSecond = 0.95f,
	};
}