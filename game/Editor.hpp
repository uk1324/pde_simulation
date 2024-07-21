#pragma once

#include <engine/Math/Aabb.hpp>
#include <variant>
#include <game/SimulationSettings.hpp>
#include <game/EditorActions.hpp>
#include <game/GameRenderer.hpp>
#include <game/GameInput.hpp>
#include <game/ParametricEllipse.hpp>
#include <game/ParametricParabola.hpp>
#include <game/Gizmo.hpp>
#include <game/Shared.hpp>
#include <clipper2/clipper.h>

struct Editor {
	struct Result {
		bool switchToSimulation = false;
	};

	static Editor make();

	Result update(GameRenderer& renderer, const GameInput& input);
	void selectToolUpdate(const GameInput& input);

	Result gui();

	void selectToolGui();
	void entityGui(EditorEntityId id);
	struct EntityGuiRigidBody {
		EditorRigidBodyId id;
		EditorRigidBody old;
	};
	std::optional<EntityGuiRigidBody> entityGuiRigidBody;
	struct EntityGuiEmitter {
		EditorEmitterId id;
		EditorEmitter old;
	};
	std::optional<EntityGuiEmitter> entityGuiEmitter;
	struct EntityGuiRevoluteJoint {
		EditorRevoluteJointId id;
		EditorRevoluteJoint old;
	};
	std::optional<EntityGuiRevoluteJoint> entityGuiRevoluteJoint;

	bool shapeGui(EditorShape& shape);

	void render(GameRenderer& renderer, const GameInput& input);

	static bool firstFrame;

	void createObject(EditorShape&& shape);

	EditorShape makeRectangleShape(Vec2 center, Vec2 halfSize);

	const f32 dt = 1.0f / 60.0f;

	bool isCursorSnappingEnabled(const GameInput& input) const;

	enum class ToolType {
		SELECT,
		CIRCLE,
		ELLIPSE,
		PARABOLA,
		POLYGON,
		RECTANGLE,
		REGULAR_POLYGON,
		LINE,
		BOOLEAN_SHAPE_OPERATIONS,
		EMMITER,
		REVOLUTE_JOINT,
	};
	ToolType selectedTool = ToolType::CIRCLE;

	struct CircleTool {
		std::optional<Vec2> center;
		std::optional<EditorCircleShape> update(Vec2 cursorPos, bool cursorLeftDown, bool cursorRightDown);
		void render(GameRenderer& gfx, Vec2 cursorPos, Vec4 color, bool isStaticSetting);
	} circleTool;

	struct PolygonTool {
		static PolygonTool make();

		bool update(Vec2 cursorPos, bool drawDown, bool drawHeld, bool closeCurveDown, bool cancelDrawingDown);

		static constexpr const char* invalidShapeModalName = "invalid shape";
		void openInvalidShapeModal();
		void invalidShapeModalGui();

		bool drawing;
		List<Vec2> vertices;
	} polygonTool;

	struct RectangleTool {
		std::optional<Vec2> corner;

		std::optional<Aabb> update(Vec2 cursorPos, bool cursorLeftDown, bool cursorRightDown);
		void render(GameRenderer& renderer, Vec2 cursorPos, Vec4 color, bool isStaticSetting);
	} rectangleTool;

	struct RegularPolygonTool {
		std::optional<Vec2> center;
		i32 vertexCount = 6;

		static f32 firstVertexAngle(Vec2 center, Vec2 cursorPos);
		struct Result {
			Vec2 center;
			f32 radius;
			f32 firstVertexAngle;
		};

		std::optional<Result> update(Vec2 cursorPos, bool cursorLeftDown, bool cursorRightDown);
		void render(GameRenderer& renderer, Vec2 cursorPos, Vec4 color, bool isStaticSetting);
	} regularPolygonTool;

	struct LineTool {
		f32 width = 1.0f;
		std::optional<Vec2> endpoint0;

		std::optional<std::pair<Vec2, Vec2>> update(Vec2 cursorPos, bool cursorLeftDown, bool cursorRightDown);
		void render(GameRenderer& renderer, Vec2 cursorPos, Vec4 color, bool isStaticSetting);
	} lineTool;

	struct EllipseTool {
		std::optional<Vec2> focus0;
		std::optional<Vec2> focus1;
		std::optional<ParametricEllipse> update(Vec2 cursorPos, bool cursorLeftDown, bool cursorRightDown);
		void render(GameRenderer& renderer, Vec2 cursorPos, Vec4 color, bool isStaticSetting);
	} ellipseTool;

	struct ParabolaTool {
		std::optional<Vec2> focus;
		std::optional<Vec2> vertex;

		std::optional<ParametricParabola> update(Vec2 cursorPos, bool cursorLeftDown, bool cursorRightDown);
		void render(GameRenderer& renderer, Vec2 cursorPos, Vec4 color, bool isStaticSetting);
		void reset();

		// TODO: maxYOverwrite is hacky
		static std::optional<ParametricParabola> makeParabola(Vec2 focus, Vec2 vertex, Vec2 yBoundPoint, std::optional<f32> maxYOverwrite = std::nullopt);
	} parabolaTool;

	struct EmitterTool {
		void render(Vec2 cursorPos);
	} emitterTool;

	EditorMaterialType materialTypeSetting;
	EditorMaterialTransimisive materialTransimisiveSetting = EditorMaterialTransimisive{ 
		.matchBackgroundSpeedOfTransmission = false,
		.speedOfTransmition = 1.0f, 
	};
	bool isStaticSetting = false;

	// Not sure if this is the best way to achieve this, but I want everything to collide with the ground regardless of category.
	// https://box2d.org/documentation_v3/md_simulation.html#filtering
	// If a rigidbody has no category it collides has collision disabled.
	// the default category is 0b1 and this is what the ground has.
	// The user created bodies can't have a zero category, because then they would pass through the ground.
	// This is why I set the 0b10 bit and make it inaccesible in the editor.
	// I also have the disable the collision with this, because if it were enabled then it would be impossible to disable collision.
	// This is, because if any bit matches in the masks then the collision will occur.
	u32 rigidBodyCollisionCategoriesSetting = 0b110;
	u32 rigidBodyCollisionMaskSetting = ~(0b10u);

	EditorMaterial materialSetting() const;
	void materialSettingGui();
	bool transmissiveMaterialGui(EditorMaterialTransimisive& material);
	static bool collisionGui(u32& collisionCategories, u32& collisionMask);
	void rigidBodyGui();

	f32 emitterStrengthSetting = EMITTER_STRENGTH_DEFAULT;
	bool emitterOscillateSetting = false;
	f32 emitterPeriodSetting = 1.0f;
	f32 emitterPhaseOffsetSetting = 0.0f;
	std::optional<InputButton> emitterActivateOnSetting;

	bool emitterGui(f32& strength, bool& oscillate, f32& period, f32& phaseOffset, std::optional<InputButton>& activateOn);

	struct RevoluteJointTool {
		bool showPreview = false;
		void render(GameRenderer& renderer, Vec2 cursorPos);

		f32 motorSpeedSetting = 1.0f;
		f32 motorMaxTorqueSetting = 10.0f;
		bool motorEnabledSetting = false;
		std::optional<InputButton> clockwiseKeySetting;
		std::optional<InputButton> counterclockwiseKeySetting;

		bool clockwiseKeyWatingForKey = false;
		bool counterclockwiseKeyWatingForKey = false;

	} revoluteJointTool;

	void revoluteJointToolUpdate(Vec2 cursorPos, bool cursorLeftDown);
	static bool revoluteJointGui(f32& motorSpeed, f32& motorMaxTorque, bool& motorAlwaysEnabled, std::optional<InputButton>& clockwiseKey, std::optional<InputButton>& counterclockwiseKey, bool& clockwiseKeyWatingForKey, bool& counterclockwiseKeyWatingForKey);

	struct ShapeBooleanOperationsTool {
		std::optional<EditorRigidBodyId> selectedRhs;
		std::optional<EditorRigidBodyId> selectedLhs;

		List<EditorRigidBodyId> bodiesUnderCursorOnLastLeftClick;
		i32 leftClickSelectionCycle;
		List<EditorRigidBodyId> bodiesUnderCursorOnLastRightClick;
		i32 rightClickSelectionCycle;
		static ShapeBooleanOperationsTool make();

		enum class BooleanOp {
			DIFFERENCE,
			INTERSECTION,
			UNION,
			XOR,
		};

		void gui();

		BooleanOp selectedBooleanOp;

	} shapeBooleanOperationsTool;
	void shapeBooleanOperationsToolUpdate(Vec2 cursorPos, bool cursorLeftDown, bool cursorRightDown, bool applyDown);

	Gizmo gizmo;
	List<EditorShape> gizmoSelectedShapesAtGrabStart;
	static bool canBeMovedByGizmo(EditorEntityType type);

	bool emitterWatingForKey = false;

	Camera camera;

	struct SelectTool {
		std::optional<Vec2> grabStartPos;
		std::optional<Aabb> selectionBox(const Camera& camera, Vec2 cursorPos) const;
	} selectTool;

	Aabb roomBounds;

	SimulationSettings simulationSettings;

	std::unordered_set<EditorEntityId> selectedEntities;
	Vec2 selectedEntitiesCenter();

	void destoryEntity(const EditorEntityId& id);

	void activateEntity(const EditorEntityId& id);
	void deactivateEntity(const EditorEntityId& id);

	void destoryAction(EditorAction& action);
	void redoAction(const EditorAction& action);
	void undoAction(const EditorAction& action);
	EditorActions actions;

	void fullyDeleteEntity(const EditorEntityId& id);
	void deleteShape(const EditorShape& shape);

	Vec2 entityGetPosition(const EditorEntityId& id) const;
	void entitySetPosition(const EditorEntityId& id, Vec2 position);
	Vec2 shapeGetPosition(const EditorShape& shape) const;
	void shapeSetPosition(EditorShape& shape, Vec2 position);
	Vec2 entityGetCenter(const EditorEntityId& id) const;
	Vec2 shapeGetCenter(const EditorShape& shape) const;

	struct RigidBodyTransform {
		RigidBodyTransform(Vec2 translation, f32 rotation);

		Vec2 translation;
		f32 rotation;
	};
	std::optional<RigidBodyTransform> tryGetRigidBodyTransform(EditorRigidBodyId id) const;
	std::optional<RigidBodyTransform> tryGetShapeTransform(const EditorShape& shape) const;
	RigidBodyTransform getShapeTransform(const EditorShape& shape) const;
	Vec2 getEmitterPosition(const EditorEmitter& emitter) const;
	Vec2 getRevoluteJointAbsolutePosition0(const EditorRevoluteJoint& joint) const;

	EditorShape cloneShape(const EditorShape& shape);

	Aabb editorShapeAabb(const EditorShape& shape) const;
	bool isEditorShapeContainedInAabb(const EditorShape& shape, const Aabb& aabb) const;
	bool isPointInEditorShape(const EditorShape& shape, Vec2 point) const;

	Clipper2Lib::PathsD getShapePath(const EditorShape& shape) const;
	void createRigidBodiesFromPaths(const Clipper2Lib::PathsD& paths, const EditorMaterial& material, bool isStatic, u32 collisionCategories, u32 collisionMask);

	EntityArray<EditorPolygonShape, EditorPolygonShape::DefaultInitialize> polygonShapes;
	EntityArrayPair<EditorPolygonShape> createPolygonShape();
	EntityArray<EditorRigidBody, EditorRigidBody::DefaultInitialize> rigidBodies;
	EntityArrayPair<EditorRigidBody> createRigidBody(const EditorShape& shape, const EditorMaterial& material, bool isStatic, u32 collisionCategories, u32 collisionMask);
	EntityArray<EditorEmitter, EditorEmitter::DefaultInitialize> emitters;
	EntityArray<EditorRevoluteJoint, EditorRevoluteJoint::DefaultInitialize> revoluteJoints;
};

