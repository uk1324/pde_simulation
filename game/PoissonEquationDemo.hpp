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