#pragma once

#include <List.hpp>
#include <engine/Math/Vec2.hpp>

struct HeatEquationDemo {
	HeatEquationDemo();

	static bool firstFrame;

	void update();

	List<f32> simulationU;
	List<Vec2> controlPoints;
	List<f32> controlPointsDerivatives;

	f32 alpha = 0.1f;
	f32 dt = 1.0f / 600.0f;
};