#pragma once

#include <Array2d.hpp>
#include <List.hpp>
#include <game/GameInput.hpp>
#include <game/Box2d.hpp>
#include <game/GameRenderer.hpp>
#include <game/SimulationSettings.hpp>

enum class CellType : u8 {
	EMPTY,
	REFLECTING_WALL
};

struct Simulation {
	Simulation();

	f32 dt;

	void update(GameRenderer& renderer, const GameInput& input);
	void waveSimulationUpdate(f32 deltaTime);
	void render(GameRenderer& renderer);

	void reset();

	Aabb displayGridBounds() const;
	Aabb simulationGridBounds() const;

	Vec2T<i64> simulationGridSize;

	b2WorldId world;

	bool debugDisplay = false;

	enum class ShapeType {
		CIRCLE,
		POLYGON,
	};

	struct ShapeInfo {
		ShapeType type;
		List<Vec2> simplifiedOutline;
		List<Vec2> vertices;
		List<i32> boundaryEdges;
		List<i32> trianglesVertices;
		f32 radius;
	};

	struct ReflectingObject {
		b2BodyId id;
		ShapeInfo shape;
	};
	List<ReflectingObject> reflectingObjects;

	struct TransmissiveObject {
		b2BodyId id;
		ShapeInfo shape;
		f32 speedOfTransmition;
		// @Performance: Could make a different type of object that always has matchBackgroundSpeedOfTransmission set to true. This might make some code faster, but it would require writing more code.
		bool matchBackgroundSpeedOfTransmission;
	};
	List<TransmissiveObject> transmissiveObjects;

	struct Emitter {
		b2BodyId body;
		Vec2 positionRelativeToBody;
		f32 strength;
	};
	List<Emitter> emitters;
	Vec2 getEmitterPos(const Emitter& emitter);

	void getShapes(b2BodyId body);
	List<b2ShapeId> getShapesResult;

	void updateMouseJoint(Vec2 cursorPos, bool inputIsUp, bool inputIsDown);
	b2JointId mouseJoint;
	b2BodyId mouseJointBody0;

	Camera camera;

	SimulationSettings simulationSettings;

	Array2d<f32> u;
	Array2d<f32> u_t;
	Array2d<CellType> cellType;
	Array2d<f32> speedSquared;

	Array2d<Pixel32> debugDisplayGrid;
	Texture debugDisplayTexture;

	Array2d<f32> displayGrid;
	Array2d<f32> displayGridTemp;
	bool applyBlurToDisplayGrid = true;
	Texture displayTexture;
};