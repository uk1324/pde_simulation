#pragma once

#include <Array2d.hpp>
#include <List.hpp>
#include <game/GameInput.hpp>
#include <game/Box2d.hpp>
#include <game/GameRenderer.hpp>

enum class CellType : u8 {
	EMPTY,
	REFLECTING_WALL
};

struct Simulation {
	Simulation();

	f32 dt;

	void update(GameRenderer& renderer, const GameInput& input);
	void waveSimulationUpdate();
	void render(GameRenderer& renderer);

	void reset();

	Aabb displayGridBounds() const;
	Aabb simulationGridBounds() const;

	Vec2T<i64> simulationGridSize;

	b2WorldId world;

	enum class ObjectShapeType {
		CIRCLE,
		POLYGON,
	};

	struct Object {
		b2BodyId id;
		ObjectShapeType shapeType;
		List<Vec2> simplifiedOutline;
		f32 radius;
	};
	List<Object> objects;
	struct TransparentObject {
		b2BodyId b2Id;
	};
	List<TransparentObject> transparentObjects;

	void getShapes(b2BodyId body);
	List<b2ShapeId> getShapesResult;

	void updateMouseJoint(Vec2 cursorPos, bool inputIsUp, bool inputIsDown);
	b2JointId mouseJoint;
	b2BodyId mouseJointBody0;

	Camera camera;

	Array2d<f32> u;
	Array2d<f32> u_t;
	Array2d<CellType> cellType;
	Array2d<f32> speedSquared;

	Array2d<Pixel32> displayGrid;
	Texture displayTexture;
};