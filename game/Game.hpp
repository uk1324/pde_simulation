#pragma once

#include <gfx2d/Gfx2d.hpp>
#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/ShaderProgram.hpp>
#include <game/Array2d.hpp>
#include <List.hpp>
#include <game/Box2d.hpp>

enum class CellType : u8 {
	EMPTY,
	REFLECTING_WALL
};

struct Game {
	Game();

	void update();
	void waveSimulationUpdate();
	void render();

	Aabb displayGridBounds() const;
	Aabb simulationGridBounds() const;

	Vec2T<i64> simulationGridSize;
	f32 cellSize;

	b2WorldId world;

	struct Object {
		b2BodyId id;
	};
	List<Object> objects;

	b2JointId mouseJoint;

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