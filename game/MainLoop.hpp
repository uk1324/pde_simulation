#pragma once

#include <gfx2d/Gfx2d.hpp>
#include <gfx2d/Camera.hpp>
#include <engine/Graphics/Vao.hpp>
#include <gfx/ShaderManager.hpp>
#include <game/PoissonEquationSolver.hpp>
#include <game/PoissonEquationDemo.hpp>
#include <game/HeatEquationDemo.hpp>
#include <game/Game.hpp>

struct MainLoop {
	MainLoop();

	void update();
	void render();

	Game game;

	/*PoissonEquationDemo demo;
	HeatEquationDemo heatEquation;*/
	//Array2d<f32> data;

	Camera camera;
};