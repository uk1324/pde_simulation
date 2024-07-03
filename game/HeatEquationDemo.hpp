#pragma once

#include <List.hpp>
#include <engine/Math/Vec2.hpp>

struct HeatEquationDemo {
	HeatEquationDemo();

	static bool firstFrame;

	void update();

	//i64 size;
	List<f32> simulationU;
	List<Vec2> controlPoints;
};