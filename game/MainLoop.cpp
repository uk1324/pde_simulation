#include <game/MainLoop.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <game/Shaders/waveData.hpp>
#include <gfx/Instancing.hpp>
#include <gfx2d/Quad2dPt.hpp>
#include "MatrixUtils.hpp"

Vec2 waveSize(500, 500);
f32 cellSize = 0.01f;

void steadyStateHeatEquationTest() {
	PoissonEquationSolver solver;
	auto u = Matrix<f32>::zero(5, 5);
	auto f = Matrix<f32>::zero(5, 5);
	const auto region = Aabb(Vec2(0.0f), Vec2(0.5f));
	const auto regionSize = region.size();
	for (i64 i = 0; i < u.sizeX(); i++) {
		const auto x = region.min.x + (f32(regionSize.x) / (u.sizeX() - 1)) * i;
		u(i, 0) = 200.0f * x;
	}
	for (i64 i = 0; i < u.sizeY(); i++) {
		const auto y = region.min.y + (f32(regionSize.y) / f32(u.sizeY() - 1)) * (u.sizeY() - 1 - i);
		u(u.sizeX() - 1, i) = 200.0f * y;
	}
	solver.solve(matrixViewFromConstMatrix(f), matrixViewFromMatrix(u), region);
	matrixPrint(u);
	/*
	The correct output is
	| 0    25   50    75 100 |
	| 0 18.75 37.5 56.25  75 |
	| 0  12.5   25  37.5  50 |
	| 0  6.25 12.5 18.75  25 |
	| 0     0    0     0   0 |
	*/
	// Technically what the solver does is discretize the problem and then try to solve the discretized problem exactly. The that comes from the discretization of the partial derivatives is based on the 4th partial derivatives of u. In this case the 4th derivatives are zero so if the discretized problem is solved exacly then it's also an exact solution to the original problem.
	// The exact solution is 400xy

	auto exactResult = Matrix<f32>::uninitialized(u.size());
	for (i64 j = 0; j < u.sizeY(); j++) {
		for (i64 i = 0; i < u.sizeX(); i++) {
			const auto x = region.min.x + (f32(regionSize.x) / (u.sizeX() - 1)) * i;
			const auto y = region.min.y + (f32(regionSize.y) / f32(u.sizeY() - 1)) * (u.sizeY() - 1 - j);
			exactResult(i, j) = 400.0f * x * y;
		}
	}
	matrixPrint(exactResult);
}

void poissonEquationTest() {
	PoissonEquationSolver solver;
	auto u = Matrix<f32>::zero(7, 6);
	auto f = Matrix<f32>::zero(u.size());

	const auto region = Aabb(Vec2(0.0f), Vec2(2.0f, 1.0f));
	const auto regionSize = region.size();
	for (i64 i = 0; i < u.sizeX(); i++) {
		const auto x = region.min.x + (f32(regionSize.x) / (u.sizeX() - 1)) * i;
		u(i, 0) = exp(1.0f) * x;
		u(i, u.sizeY() - 1) = x;
	}
	for (i64 i = 0; i < u.sizeY(); i++) {
		const auto y = region.min.y + (f32(regionSize.y) / f32(u.sizeY() - 1)) * (u.sizeY() - 1 - i);
		u(u.sizeX() - 1, i) = 2.0f * exp(y);
		u(0, i) = 0;
	}

	for (i64 j = 0; j < u.sizeY(); j++) {
		for (i64 i = 0; i < u.sizeX(); i++) {
			const auto x = region.min.x + (f32(regionSize.x) / (u.sizeX() - 1)) * i;
			const auto y = region.min.y + (f32(regionSize.y) / f32(u.sizeY() - 1)) * (u.sizeY() - 1 - j);
			f(i, j) = x * exp(y);
		}
	}

	solver.solve(matrixViewFromConstMatrix(f), matrixViewFromMatrix(u), region);
	matrixPrint(u);
	

	auto exactResult = Matrix<f32>::uninitialized(u.size());
	for (i64 j = 0; j < u.sizeY(); j++) {
		for (i64 i = 0; i < u.sizeX(); i++) {
			const auto x = region.min.x + (f32(regionSize.x) / (u.sizeX() - 1)) * i;
			const auto y = region.min.y + (f32(regionSize.y) / f32(u.sizeY() - 1)) * (u.sizeY() - 1 - j);
			exactResult(i, j) = x * exp(y);
		}
	}

	matrixPrint(exactResult);
	matrixPrint(exactResult - u);
}

MainLoop::MainLoop() 
	: gfx(Gfx2d::make())
	, waveVao(createInstancingVao<WaveShader>(gfx.quad2dPtVbo, gfx.quad2dPtIbo, gfx.instancesVbo))
	, waveShader(MAKE_GENERATED_SHADER(WAVE))
	, waveTexture(Texture::generate())
	, data(waveSize.x, waveSize.y) {

	{
		waveTexture.bind();
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_R32F,
			waveSize.x,
			waveSize.y,
			0,
			GL_RED,
			GL_FLOAT,
			nullptr
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	}
	poissonEquationTest();
	//steadyStateHeatEquationTest();
}

void MainLoop::update() {
	ShaderManager::update();
	glViewport(0, 0, Window::size().x, Window::size().y);
	glClear(GL_COLOR_BUFFER_BIT);
	
	waveShader.use();
	std::vector<WaveInstance> instances;
	const auto size = Vec2(waveSize * cellSize);
	instances.push_back(WaveInstance{
		.transform = camera.makeTransform(Vec2(0.0f), 0.0f, size / 2.0f)
	});
	camera.changeSizeToFitBox(size);
	waveShader.setTexture("waveTexture", 0, waveTexture);

	for (i32 yi = 0; yi < waveSize.y; yi++) {
		for (i32 xi = 0; xi < waveSize.x; xi++) {
			const auto x = xi / (waveSize.x - 1.0f);
			const auto y = yi / (waveSize.y - 1.0f);
			data(xi, yi) = sin(50.0 * x) * sin(30.0f * y);
		}
	}
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, waveSize.x, waveSize.y, GL_RED, GL_FLOAT, data.data());

	drawInstances(waveVao, gfx.instancesVbo, instances, quad2dPtDrawInstances);
}
