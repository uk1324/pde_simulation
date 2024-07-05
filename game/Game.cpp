#include "Game.hpp"
#include <game/Shaders/waveData.hpp>
#include <gfx/ShaderManager.hpp>
#include <game/View2dUtils.hpp>
#include <engine/Math/Color.hpp>
#include <gfx/Instancing.hpp>
#include <gfx2d/Quad2dPt.hpp>
#include <engine/Input/Input.hpp>
#include <game/Textures.hpp>
#include <engine/Window.hpp>
#include <game/GridUtils.hpp>
#include <game/Array2dDrawingUtils.hpp>

Game::Game()
	: gfx(Gfx2d::make())
	, waveVao(createInstancingVao<WaveShader>(gfx.quad2dPtVbo, gfx.quad2dPtIbo, gfx.instancesVbo))
	, waveShader(MAKE_GENERATED_SHADER(WAVE))
	, displayTexture(makePixelTexture(gridSize.x, gridSize.y))
	/*, gridSize(300, 200)*/
	, gridSize(600, 400)
	, cellSize(0.1f)
	, u(Array2d<f32>::filled(gridSize.x, gridSize.y, 0.0f))
	, u_t(Array2d<f32>::filled(gridSize.x, gridSize.y, 0.0f))
	, cellType(Array2d<CellType>::filled(gridSize.x, gridSize.y, CellType::EMPTY))
	, displayGrid(Array2d<Pixel32>::filled(gridSize, Pixel32(0, 0, 0))) {

	for (i64 yi = 0; yi < gridSize.y; yi++) {
		cellType(0, yi) = CellType::REFLECTING_WALL;
		cellType(gridSize.x - 1, yi) = CellType::REFLECTING_WALL;
	}

	for (i64 xi = 1; xi < gridSize.x - 1; xi++) {
		cellType(xi, 0) = CellType::REFLECTING_WALL;
		cellType(xi, gridSize.y - 1) = CellType::REFLECTING_WALL;
	}

	//const auto size = 100;
	//for (i64 i = 0; i < size; i++) {
	//	const auto j = i - size / 2;
	//	const auto k = i32((j * j) / 24.0f);
	//	Vec2T<i64> p(i - size / 2 + gridSize.x / 2, k);
	//	//cellType(i - 25 + gridSize.x / 2, k) = CellType::REFLECTING_WALL;
	//	fillCircle(cellType, p, 3, CellType::REFLECTING_WALL);
	//}

	const auto size = 100;
	for (i64 i = 0; i < size; i++) {
		const auto j = i - size / 2;
		const auto k = i32((j * j) / 100.0f);
		Vec2T<i64> p(i - size / 2 + gridSize.x / 2, k);
		//cellType(i - 25 + gridSize.x / 2, k) = CellType::REFLECTING_WALL;
		fillCircle(cellType, p, 3, CellType::REFLECTING_WALL);
	}

	for (i64 i = 0; i < size; i++) {
		const auto j = i - size / 2;
		const auto k = i32((j * j) / 100.0f);
		Vec2T<i64> p(i - size / 2 + gridSize.x / 2, gridSize.y - 1 - k);
		//cellType(i - 25 + gridSize.x / 2, k) = CellType::REFLECTING_WALL;
		fillCircle(cellType, p, 3, CellType::REFLECTING_WALL);
	}

}

void Game::update() {
	/*
	There are 2 versions of the wave equation with variable speed.
	1. Dtt u = c(x)^2 div grad u = c(x)^2 (Dxx u + Dyy u)
	2. Dtt u = div (c(x)^2 grad u)

	The first one is used for example in maxwells equation and the second one is used for surface water waves. https://www.sciencedirect.com/science/article/abs/pii/S0165212510000338

	Discretizing Dxx u + Dyy u, assuming dx = dy
	Dxx u + Dyy u =
	(u(x + dl, y, 0) - 2u(x, y, 0) + u(x - dl, y, 0)) / dl + (u(x, y + dl, 0) - 2u(x, y, 0) + u(x, y - dl, 0)) / dl =
	(u(x + dl, y, 0) + u(x - dl, y, 0) + (u(x, y + dl, 0) + u(x, y - dl, 0) - 4u(x, y, 0)) / dl

	For the time discretization I will use forward euler twice. I am not using a central difference approximation even though it has less error, because it requires values from the past frame. This might cause issues when the boundary conditions change between frames.

	*/
	const auto viewSize = Vec2(gridSize) * cellSize;
	const auto halfViewSize = viewSize / 2.0f;
	auto gridBounds = Aabb(-halfViewSize, halfViewSize);

	const auto cursorPos = Input::cursorPosClipSpace() * camera.clipSpaceToWorldSpace();
	const auto cursorGridPos = positionToGridPosition(cursorPos, gridBounds, gridSize);

	if (cursorGridPos.has_value() && Input::isMouseButtonHeld(MouseButton::LEFT)) {
		//u(cursorGridPos->x, gridSize.y - 1 - cursorGridPos->y) = 100.0f;
		fillCircle(u, *cursorGridPos, 3, 100.0f);
	}

	if (cursorGridPos.has_value() && Input::isMouseButtonHeld(MouseButton::MIDDLE)) {
		fillCircle(cellType, *cursorGridPos, 3, CellType::REFLECTING_WALL);
	}
	if (cursorGridPos.has_value() && Input::isMouseButtonHeld(MouseButton::RIGHT)) {
		fillCircle(cellType, *cursorGridPos, 3, CellType::EMPTY);
	}


	for (i32 yi = 0; yi < gridSize.y; yi++) {
		for (i32 xi = 0; xi < gridSize.x; xi++) {
			switch (cellType(xi, yi)) {
			case CellType::EMPTY:
				break;

			case CellType::REFLECTING_WALL:
				u(xi, yi) = 0.0f;
				//u_t(xi, yi) = 0.0f;
			}
		}
	}

	const auto dt = 0.01f;
	for (i32 yi = 1; yi < gridSize.y - 1; yi++) {
		for (i32 xi = 1; xi < gridSize.x - 1; xi++) {
			const auto laplacianU = u(xi + 1, yi) + u(xi - 1, yi) + u(xi, yi + 1) + u(xi, yi - 1) - 4.0f * u(xi, yi);
			const auto c = 50.0f;
			const auto c2 = c * c;
			u_t(xi, yi) += (laplacianU * c2) * dt;
		}
	}
	for (i32 yi = 1; yi < gridSize.y - 1; yi++) {
		for (i32 xi = 1; xi < gridSize.x - 1; xi++) {
			u(xi, yi) += dt * u_t(xi, yi);
		}
	}
	for (i32 yi = 1; yi < gridSize.y - 1; yi++) {
		for (i32 xi = 1; xi < gridSize.x - 1; xi++) {
			u_t(xi, yi) *= 0.99f;
		}
	}

	//const auto maxValue = max(view2d(u))
	for (i32 yi = 0; yi < gridSize.y; yi++) {
		for (i32 xi = 0; xi < gridSize.x; xi++) {
			switch (cellType(xi, yi)) {
			case CellType::EMPTY: {
				const auto color = Color3::scientificColoring(u(xi, yi), -5.0f, 5.0f);
				displayGrid(xi, yi) = Pixel32(color);
				break;
			}

			case CellType::REFLECTING_WALL:
				displayGrid(xi, yi) = Pixel32(Vec3(0.5f));
				break;
			}
		}
	}
	updatePixelTexture(displayGrid.data(), gridSize.x, gridSize.y);
	camera.aspectRatio = Window::aspectRatio();
	gfx.camera = camera;

	glViewport(0, 0, Window::size().x, Window::size().y);
	glClear(GL_COLOR_BUFFER_BIT);

	{
		waveShader.use();
		std::vector<WaveInstance> instances;
		instances.push_back(WaveInstance{
			.transform = camera.makeTransform(Vec2(0.0f), 0.0f, halfViewSize)
		});
		camera.changeSizeToFitBox(viewSize);
		waveShader.setTexture("waveTexture", 0, displayTexture);
		drawInstances(waveVao, gfx.instancesVbo, instances, quad2dPtDrawInstances);
	}
	const auto width = 0.1;
	const auto radius = 4.0f;
	gfx.addCircle(cursorPos, radius - width, width, Color3::WHITE);
	gfx.addLine(Vec2(0.0f), cursorPos, width, Color3::WHITE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gfx.drawCircles();
	gfx.drawLines();
	glDisable(GL_BLEND);
}
