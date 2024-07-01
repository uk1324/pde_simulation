#pragma once

#include <gfx2d/Gfx2d.hpp>
#include <gfx2d/Camera.hpp>
#include <engine/Graphics/Vao.hpp>
#include <gfx/ShaderManager.hpp>

struct MainLoop {
	MainLoop();

	void update();

	Gfx2d gfx;

	Vao waveVao;
	ShaderProgram& waveShader;

	Camera camera;
};