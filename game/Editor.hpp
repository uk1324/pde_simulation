#pragma once

#include <engine/Math/Aabb.hpp>
#include <variant>
#include <game/EditorActions.hpp>
#include <game/GameRenderer.hpp>

struct GameInput {
	bool upButtonHeld;
	bool downButtonHeld;
	bool leftButtonHeld;
	bool rightButtonHeld;
	Vec2 cursorPos;
	bool cursorLeftDown;
	bool cursorRightDown;
	bool undoDown;
	bool redoDown;
	bool ctrlHeld;
	bool deleteDown;
};

bool isPointInCircle(Vec2 center, f32 radius, Vec2 point);

bool isPointInEditorShape(const EditorShape& shape, Vec2 point);

struct Editor {
	static Editor make();

	void update(GameRenderer& renderer);
	void gui();
	void render(GameRenderer& renderer, const GameInput& input);

	static bool firstFrame;

	void createObject(EditorShape&& shape);

	EditorShape makeRectangleShape(Vec2 center, Vec2 halfSize);

	const f32 dt = 1.0f / 60.0f;

	enum class ToolType {
		SELECT,
		CIRCLE,
		RECTANGLE,
	};
	ToolType selectedTool = ToolType::CIRCLE;

	struct CircleTool {
		std::optional<Vec2> center;
		std::optional<EditorCircleShape> update(Vec2 cursorPos, bool cursorLeftDown, bool cursorRightDown);
		void render(GameRenderer& gfx, Vec2 cursorPos);
	} circleTool;

	Camera camera;

	Aabb roomBounds;

	std::unordered_set<EditorEntityId> hoveredOverEntities;
	std::unordered_set<EditorEntityId> selectedEntities;

	void destoryEntity(const EditorEntityId& id);

	void activateEntity(const EditorEntityId& id);
	void deactivateEntity(const EditorEntityId& id);

	void destoryAction(EditorAction& action);
	void redoAction(const EditorAction& action);
	void undoAction(const EditorAction& action);
	EditorActions actions;

	void fullyDeleteEntity(const EditorEntityId& id);
	void deleteShape(const EditorShape& shape);

	EntityArray<EditorPolygonShape, EditorPolygonShape::DefaultInitialize> polygonShapes;
	EntityArrayPair<EditorPolygonShape> createPolygonShape();
	EntityArray<EditorReflectingBody, EditorReflectingBody::DefaultInitialize> reflectingBodies;
	EntityArrayPair<EditorReflectingBody> createReflectingBody();
};

