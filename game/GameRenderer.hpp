#pragma once

#include <gfx2d/Gfx2d.hpp>

struct GameRenderer {
	static GameRenderer make();

	//void update();
	//void drawDebugDisplay();
	void drawGrid();

	Gfx2d gfx;

	Vao gridVao;
	ShaderProgram& gridShader;

	Vao waveVao;
	ShaderProgram& waveShader;
};