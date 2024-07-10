#pragma once

#include <gfx2d/Gfx2d.hpp>

struct GameRenderer {
	static GameRenderer make();

	void drawBounds(Aabb aabb);
	void disk(Vec2 center, f32 radius, f32 angle, Vec3 color, bool isSelected);
	//void update();
	//void drawDebugDisplay();

	void drawGrid();

	Gfx2d gfx;

	Vao gridVao;
	ShaderProgram& gridShader;

	Vao waveVao;
	ShaderProgram& waveShader;
};