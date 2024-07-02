#pragma once

#include <gfx2d/Gfx2d.hpp>
#include <gfx2d/Camera.hpp>
#include <engine/Graphics/Vao.hpp>
#include <gfx/ShaderManager.hpp>
#include <Array2d.hpp>
#include <game/PoissonEquationSolver.hpp>

struct MainLoop {
	MainLoop();

	void update();

	Gfx2d gfx;

	PoissonEquationSolver solver;

	Vao waveVao;
	ShaderProgram& waveShader;

	Texture waveTexture;
	Array2d<f32> data;

	Camera camera;
};