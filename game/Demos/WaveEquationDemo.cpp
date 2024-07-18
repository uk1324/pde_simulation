#include "WaveEquationDemo..hpp"
#include <iostream>
#include <iomanip>
#include <engine/Math/Vec2.hpp>

void runWaveEquationDemo() {
	const auto SIZE = 10;
	f32 u[SIZE];
	for (auto& item : u) {
		item = 0.0f;
	}
	f32 u_t[SIZE];
	for (auto& item : u_t) {
		item = 0.0f;
	}
	f32 dx = 0.1f;
	f32 dt = 0.05f;

	u[1] = 5.0f;

	const auto stepCount = 50;
	for (i64 stepI = 0; stepI < stepCount; stepI++) {
		u[0] = 0.0f;
		u[SIZE - 1] = 0.0f;

		for (i64 i = 1; i < SIZE - 1; i++) {
			const auto u_tt = (u[i - 1] - 2.0f * u[i] + u[i + 1]) / (dx * dx);
			u_t[i] += u_tt * dt;
		}

		for (i64 i = 1; i < SIZE - 1; i++) {
			u[i] += u_t[i] * dt;
		}

		std::cout << std::setprecision(2);
		for (i64 i = 0; i < SIZE; i++) {
			std::cout << u[i] << ' ';
		}
		std::cout << " |";
		for (i64 i = 0; i < SIZE; i++) {
			std::cout << u_t[i] << ' ';
		}
		std::cout << '\n';
	}
	exit(0);
}