#pragma once

#include <gfx2d/Gfx2d.hpp>
#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/ShaderProgram.hpp>
#include <game/Array2d.hpp>

enum class CellType : u8 {
	EMPTY,
	REFLECTING_WALL
};

struct Game {
	Game();

	void update();

	Vec2T<i64> gridSize;
	Vec2 cellSize;

	Gfx2d gfx;

	Vao waveVao;
	ShaderProgram& waveShader;

	Camera camera;

	Array2d<f32> u;
	Array2d<f32> u_t;
	Array2d<CellType> cellType;

	Array2d<Pixel32> displayGrid;
	Texture displayTexture;
};