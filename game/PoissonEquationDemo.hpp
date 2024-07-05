#pragma once

#include "Array2d.hpp"

struct PoissonEquationDemo {
	PoissonEquationDemo();

	void update();

	static bool firstFrame;

	Array2d<f32> inputU;
	Array2d<f32> outputU;
	Array2d<f32> f;
	Vec2T<i64> size;

	i64 radius = 3;
	bool displayNumbers = false;

	f32 uInputMin = 0.0f;
	f32 uInputMax = 1.0f;
	f32 uInputValueToWrite = 1.0f;

	f32 fInputMin = -100.0f;
	f32 fInputMax = 100.0f;
	f32 fValueToWrite = 100.0f;

	f32 calculatedOutputMin = 0.0f;
	f32 calculatedOutputMax = 1.0f;
};