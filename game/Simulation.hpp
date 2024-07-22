#pragma once

#include <Array2d.hpp>
#include <List.hpp>
#include <game/GameInput.hpp>
#include <game/Box2d.hpp>
#include <game/Shared.hpp>
#include <game/GameRenderer.hpp>
#include <game/SimulationSettings.hpp>
#include <game/InputButton.hpp>
#include <game/SimulationDisplay3d.hpp>

enum class CellType : u8 {
	EMPTY,
	REFLECTING_WALL
};

struct Simulation {
	struct Result {
		bool switchToEditor;
	};

	Simulation(Gfx2d& gfx);

	enum class DisplayMode {
		DISPLAY_2D,
		DISPLAY_3D
	};

	DisplayMode displayMode = DisplayMode::DISPLAY_2D;

	f32 realtimeDt;

	Result update(GameRenderer& renderer, const GameInput& input, bool hideGui);
	bool gui();

	void waveSimulationUpdate(f32 simulationDt);
	void render(GameRenderer& renderer, Vec3 grid3dScale);

	void runEmitter(Vec2 pos, f32 strength, bool oscillate, f32 period, f32 phaseOffset);


	void reset();

	Aabb displayGridBounds() const;
	Aabb simulationGridBounds() const;
	Vec3 grid3dScale();

	f32 simulationElapsed;

	Vec2T<i64> simulationGridSize;

	b2WorldId world;

	b2BodyId boundariesBodyId;

	bool debugDisplay = false;

	enum class ShapeType {
		CIRCLE,
		POLYGON,
	};

	struct ShapeInfo {
		ShapeType type;
		List<Vec2> simplifiedOutline;
		List<Vec2> simplifiedTriangleVertices;
		List<Vec2> vertices;
		List<i32> boundary;
		List<i32> trianglesVertices;
		f32 radius;
		static constexpr Vec2 PATH_END_VERTEX = Vec2(-FLT_MIN, FLT_MAX);
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

		bool oscillate;
		f32 period;
		f32 phaseOffset;
		
		std::optional<InputButton> activateOn;
	};
	List<Emitter> emitters;
	Vec2 getEmitterPos(const Emitter& emitter);

	struct RevoluteJoint {
		b2BodyId body0;
		Vec2 positionRelativeToBody0;
		b2BodyId body1;
		Vec2 positionRelativeToBody1;
		b2JointId joint;

		f32 motorSpeed = 1.0f;
		bool motorAlwaysEnabled = false;
		std::optional<InputButton> clockwiseKey;
		std::optional<InputButton> counterclockwiseKey;
	};
	List<RevoluteJoint> revoluteJoints;

	void getShapes(b2BodyId body);
	List<b2ShapeId> getShapesResult;

	void updateMouseJoint(Vec2 cursorPos, bool inputIsUp, bool inputIsDown);
	b2JointId mouseJoint;
	b2BodyId mouseJointBody0;

	Camera camera;

	SimulationSettings simulationSettings;

	f32 emitterStrengthSetting = 100.0f;
	bool emitterOscillateSetting = false;
	f32 emitterPeriodSetting = 1.0f;
	f32 emitterPhaseOffsetSetting = 0.0f;

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

	SimulationDisplay3d display3d;
};