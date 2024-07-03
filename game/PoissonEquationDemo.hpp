#pragma once

#include "Matrix.hpp"

struct PoissonEquationDemo {
	PoissonEquationDemo();

	void update();

	static bool firstFrame;

	Matrix<f32> inputU;
	Matrix<f32> outputU;
	Matrix<f32> f;
	Vec2T<i64> size;

	f32 maxValue = 1.0f;
	f32 minValue = 0.0f;

	f32 valueToWrite = 1.0f;
	i64 radius = 3;
	bool displayNumbers = false;

	f32 fInputMin = 0.0f;
	f32 fInputMax = 0.0f;
	bool defaultFInputMinMax = true;

	f32 outputMin = 0.0f;
	f32 outputMax = 1.0f;
	bool defaultOutputMinMax = true;
};