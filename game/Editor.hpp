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
#include <clipper2/clipper.h>

struct Editor {
	static Editor make();

	void update(GameRenderer& renderer, const GameInput& input);
	void selectToolUpdate(const GameInput& input);

	void gui();
	bool beginPropertyEditor(const char* id);
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
		POLYGON,
		RECTANGLE,
		ELLIPSE,
		PARABOLA,
		EMMITER,
		BOOLEAN_SHAPE_OPERATIONS
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

	EditorMaterial materialSetting() const;
	void materialSettingGui();
	bool transmissiveMaterialGui(EditorMaterialTransimisive& material);
	void rigidBodyGui();

	f32 emitterStrengthSetting = 5.0f;
	bool emitterOscillateSetting = false;
	f32 emitterPeriodSetting = 1.0f;
	f32 emitterPhaseOffsetSetting = 0.0f;
	std::optional<InputButton> emitterActivateOnSetting;

	bool emitterGui(f32& strength, bool& oscillate, f32& period, f32& phaseOffset, std::optional<InputButton>& activateOn);

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
	Vec2 getEmitterPosition(const EditorEmitter& emitter) const;

	EditorShape cloneShape(const EditorShape& shape);

	Aabb editorShapeAabb(const EditorShape& shape) const;
	bool isEditorShapeContainedInAabb(const EditorShape& shape, const Aabb& aabb) const;
	bool isPointInEditorShape(const EditorShape& shape, Vec2 point) const;

	Clipper2Lib::PathsD getShapePath(const EditorShape& shape) const;
	void createRigidBodiesFromPaths(const Clipper2Lib::PathsD& paths, const EditorMaterial& material, bool isStatic);

	EntityArray<EditorPolygonShape, EditorPolygonShape::DefaultInitialize> polygonShapes;
	EntityArrayPair<EditorPolygonShape> createPolygonShape();
	EntityArray<EditorRigidBody, EditorRigidBody::DefaultInitialize> rigidBodies;
	EntityArrayPair<EditorRigidBody> createRigidBody(const EditorShape& shape, const EditorMaterial& material, bool isStatic);
	EntityArray<EditorEmitter, EditorEmitter::DefaultInitialize> emitters;
};

