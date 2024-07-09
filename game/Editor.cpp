#include "Editor.hpp"
#include <game/Editor.hpp>
#include <game/Constants.hpp>
#include <engine/Input/Input.hpp>
#include <engine/Math/Color.hpp>
#include <engine/Math/Rotation.hpp>
#include <gfx2d/CameraUtils.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <glad/glad.h>

EditorShape::EditorShape(const EditorCircleShape& circle)
	: circle(circle)
	, type(EditorShapeType::CIRCLE) {}

EditorShape::EditorShape(const EditorPolygonShapeId& polygon)
	: polygon(polygon)
	, type(EditorShapeType::POLYGON) {}

Editor Editor::make() {
	Editor editor{
		.roomBounds = Constants::gridBounds(Constants::DEFAULT_GRID_SIZE),
		.actions = EditorActions::make(),
		.polygonShapes = decltype(polygonShapes)::make(),
		.reflectingBodies = decltype(reflectingBodies)::make(),
	};

	editor.camera.pos = editor.roomBounds.center();
	editor.camera.changeSizeToFitBox(editor.roomBounds.size());

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	return editor;
}

void Editor::update(GameRenderer& renderer) {
	//polygonShapes.update();
	reflectingBodies.update();

	zoomOnCursorPos(camera, dt);

	GameInput input{
		.upButtonHeld = Input::isKeyHeld(KeyCode::W),
		.downButtonHeld = Input::isKeyHeld(KeyCode::S),
		.leftButtonHeld = Input::isKeyHeld(KeyCode::A),
		.rightButtonHeld = Input::isKeyHeld(KeyCode::D),
		.cursorPos = Input::cursorPosClipSpace() * camera.clipSpaceToWorldSpace(),
		.cursorLeftDown = Input::isMouseButtonDown(MouseButton::LEFT),
		.cursorRightDown = Input::isMouseButtonDown(MouseButton::RIGHT),
		.undoDown = false,
		.redoDown = false,
		.ctrlHeld = Input::isKeyHeld(KeyCode::LEFT_CONTROL),
		.deleteDown = Input::isKeyDown(KeyCode::DELETE)
	};

	if (Input::isKeyHeld(KeyCode::LEFT_CONTROL) && Input::isKeyDownWithAutoRepeat(KeyCode::Z)) {
		input.undoDown = true;
	}

	if (Input::isKeyHeld(KeyCode::LEFT_CONTROL) && Input::isKeyDownWithAutoRepeat(KeyCode::Y)) {
		input.redoDown = true;
	}

	{
		Vec2 movement(0.0f);

		if (input.rightButtonHeld) {
			movement.x += 1.0f;
		}
		if (input.leftButtonHeld) {
			movement.x -= 1.0f;
		}
		if (input.upButtonHeld) {
			movement.y += 1.0f;
		}
		if (input.downButtonHeld) {
			movement.y -= 1.0f;
		}

		const auto speed = 10.0f;
		camera.pos += movement.normalized() * speed * dt;
	}

	if (input.undoDown) {
		if (actions.lastDoneAction >= 0 && actions.lastDoneAction < actions.actions.size()) {
			const auto offset = actions.actionIndexToStackStartOffset(actions.lastDoneAction);
			for (i64 i = offset - 1 + actions.actions[actions.lastDoneAction].subActionCount; i >= offset; i--) {
				undoAction(actions.actionStack[i]);
			}
			actions.lastDoneAction--;
		}
	}

	if (input.redoDown) {
		if (actions.lastDoneAction >= -1 && actions.lastDoneAction < actions.actions.size() - 1) {
			actions.lastDoneAction++;
			const auto offset = actions.actionIndexToStackStartOffset(actions.lastDoneAction);
			for (i64 i = offset - 1 + actions.actions[actions.lastDoneAction].subActionCount; i >= offset; i--) {
				redoAction(actions.actionStack[i]);
			}
		}
	}

	switch (selectedTool) {
		using enum ToolType;

	case SELECT: {
		hoveredOverEntities.clear();
		for (auto body : reflectingBodies) {
			if (isPointInEditorShape(body->shape, input.cursorPos)) {
				hoveredOverEntities.insert(EditorEntityId(body.id));
			}
		}

		if (input.cursorLeftDown && selectedEntities != hoveredOverEntities) {


			actions.addSelectionChange(*this, selectedEntities, hoveredOverEntities);
			selectedEntities = hoveredOverEntities;
		}
		//if (input.cursorRightDown) {
		//	selectedEntities.clear();
		//}

		/*if (input.deleteDown) {
			for (auto& entity : sel)
		}*/

		break;
	}

	case CIRCLE: {
		auto circleShape = circleTool.update(input.cursorPos, input.cursorLeftDown, input.cursorRightDown);
		if (circleShape.has_value()) {
			createObject(EditorShape(std::move(*circleShape)));
		}
		break;
	}

	case RECTANGLE: {
		break;
	}
		
	}

	render(renderer, input);
	gui();
}

void Editor::gui() {
	auto id = ImGui::DockSpaceOverViewport(
		ImGui::GetMainViewport(), 
		ImGuiDockNodeFlags_NoDockingOverCentralNode | ImGuiDockNodeFlags_PassthruCentralNode);

	const auto settingsWindowName = "settings";
	if (firstFrame) {
		ImGui::DockBuilderRemoveNode(id);
		ImGui::DockBuilderAddNode(id, ImGuiDockNodeFlags_DockSpace);

		const auto leftId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 0.5f, nullptr, &id);
		ImGui::DockBuilderSetNodeSize(leftId, ImVec2(300.0f, 1.0f));

		ImGui::DockBuilderDockWindow(settingsWindowName, leftId);

		ImGui::DockBuilderFinish(id);
		firstFrame = false;
	}

	bool openHelpWindow = false;
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("project")) {
			if (ImGui::MenuItem("new")) {

			}
			ImGui::EndMenu();
		}
		
		ImGui::EndMainMenuBar();
	}

	if (ImGui::Begin(settingsWindowName)) {
	
		const std::pair<ToolType, const char*> tools[]{
			{ ToolType::SELECT, "select" },
			{ ToolType::CIRCLE, "circle" },
			{ ToolType::RECTANGLE, "rectangle" },
		};

		ImGui::SeparatorText("tool");

		const auto oldSelection = selectedTool;

		for (const auto item : tools) {
			const auto type = item.first;
			const auto name = item.second;

			if (ImGui::Selectable(name, type == selectedTool)) {
				selectedTool = type;
			}
		}

		if (selectedTool == ToolType::SELECT && oldSelection != ToolType::SELECT) {
			selectedEntities.clear();
		}

		//ImGui::Button("");
		ImGui::End();
	}
}

void Editor::render(GameRenderer& renderer, const GameInput& input) {
	renderer.gfx.camera = camera;

	renderer.drawGrid();

	renderer.gfx.rect(roomBounds.min, roomBounds.size(), 0.01f / camera.zoom, Color3::WHITE);

	switch (selectedTool) {
		using enum ToolType;

	case CIRCLE:
		circleTool.render(renderer, input.cursorPos);
		break;
	case RECTANGLE:
		break;
	default:
		break;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (const auto& body : reflectingBodies) {
		const auto isSelected = selectedTool == ToolType::SELECT && selectedEntities.contains(EditorEntityId(body.id));

		switch (body->shape.type)
		{
		case CIRCLE: {
			const auto circle = body->shape.circle;
			renderer.disk(circle.center, circle.radius, circle.angle, Color3::WHITE / 2.0f, isSelected);
			break;
		}

		case POLYGON: {
			//polygonShapes.destroy(shape.polygon);
			break;
		}
		}
	}

	renderer.gfx.drawDisks();
	renderer.gfx.drawCircles();
	renderer.gfx.drawLines();
	glDisable(GL_BLEND);
}

void Editor::destoryAction(EditorAction& action) {
	switch (action.type) {
		using enum EditorActionType;
	case CREATE_ENTITY: {
		fullyDeleteEntity(action.createEntity.id);
		break;
	}

	case SELECTION_CHANGE: {
		actions.freeSelectionChange(action.selectionChange);
		break;
	}
	}
}

void Editor::redoAction(const EditorAction& action) {
	switch (action.type) {
		using enum EditorActionType;
	case CREATE_ENTITY: {
		redoCreateEntity(action.createEntity);
		break;
	}
	
	case SELECTION_CHANGE: {
		const auto& a = action.selectionChange;
		selectedEntities.clear();
		for (const auto& id : a.newSelection) {
			selectedEntities.insert(id);
		}
		break;
	}

	}
}

void Editor::redoCreateEntity(const EditorActionCreateEntity& a) {
	switch (a.id.type) {
		using enum EditorEntityType;
	case REFLECTING_BODY:
		reflectingBodies.activate(a.id.reflectingBody());
		break;
	
	}
}

void Editor::undoAction(const EditorAction& action) {
	switch (action.type) {
		using enum EditorActionType;
	case CREATE_ENTITY: {
		undoCreateEntity(action.createEntity);
		break;
	}

	case SELECTION_CHANGE: {
		const auto& a = action.selectionChange;
		selectedEntities.clear();
		for (const auto& id : a.oldSelection) {
			selectedEntities.insert(id);
		}
		break;
	}
	}
}

void Editor::undoCreateEntity(const EditorActionCreateEntity& a) {
	switch (a.id.type) {
		using enum EditorEntityType;
	case REFLECTING_BODY:
		reflectingBodies.deactivate(a.id.reflectingBody());
		break;
	}
}

void Editor::deleteShape(const EditorShape& shape) {
	switch (shape.type) {
		using enum EditorShapeType;

	case CIRCLE: break;
	case POLYGON: {
		polygonShapes.destroy(shape.polygon);
		break;
	}

	}
}

void Editor::fullyDeleteEntity(const EditorEntityId& id) {
	switch (id.type) {
		using enum EditorEntityType;
	case REFLECTING_BODY: {
		const auto entity = reflectingBodies.get(id.reflectingBody());
		if (!entity.has_value()) {
			CHECK_NOT_REACHED();
			break;
		}
		deleteShape(entity->shape);
		break;
	}
		
	}
}

EditorShape Editor::makeRectangleShape(Vec2 center, Vec2 halfSize) {
	const auto rot = Rotation(0.0f);
	Vec2 vertices[] = {
		halfSize,
		Vec2(halfSize.x, -halfSize.y),
		-halfSize,
		Vec2(-halfSize.x, halfSize.y)
	};
	auto shape = createPolygonShape();
	for (auto& vertex : vertices) {
		shape->vertices.add(center + rot * vertex);
	}
	return EditorShape(shape.id);
}

EntityArrayPair<EditorPolygonShape> Editor::createPolygonShape() {
	auto shape = polygonShapes.create();
	shape->vertices.clear();
	return shape;
}

EntityArrayPair<EditorReflectingBody> Editor::createReflectingBody() {
	auto body = reflectingBodies.create();
	#ifdef DEBUG
	body->shape = EditorShape(EditorCircleShape(Vec2(0.0f), 0.0f));
	#endif
	return body;
}

void Editor::createObject(EditorShape&& shape) {	
 	auto body = createReflectingBody();
	body->shape = std::move(shape);
	auto action = EditorActionCreateEntity(EditorEntityId(body.id));
	actions.add(*this, std::move(action));
}

bool Editor::firstFrame = true;

EditorPolygonShape EditorPolygonShape::DefaultInitialize::operator()() {
	return EditorPolygonShape{
		.vertices = List<Vec2>::empty()
	};
}

EditorReflectingBody EditorReflectingBody::DefaultInitialize::operator()() {
	return EditorReflectingBody{
		.shape = EditorShape(EditorCircleShape(Vec2(0.0f), 0.0f, 0.0f))
	};
}

std::optional<EditorCircleShape> Editor::CircleTool::update(Vec2 cursorPos, bool cursorLeftDown, bool cursorRightDown) {
	if (cursorRightDown) {
		center = std::nullopt;
		return std::nullopt;
	}

	if (!cursorLeftDown) {
		return std::nullopt;
	}

	if (!center.has_value()) {
		center = cursorPos;
		return std::nullopt;
	}

	auto result = EditorCircleShape(*center, center->distanceTo(cursorPos), (cursorPos - *center).angle());
	center = std::nullopt;
	return result;
}

void Editor::CircleTool::render(GameRenderer& renderer, Vec2 cursorPos) {
	if (center.has_value()) {
		renderer.disk(*center, center->distanceTo(cursorPos), (cursorPos - *center).angle(), Color3::WHITE / 2.0f, false);
	}
}

bool isPointInEditorShape(const EditorShape& shape, Vec2 point) {
	switch (shape.type) {
		using enum EditorShapeType;
	case CIRCLE:
		return isPointInCircle(shape.circle.center, shape.circle.radius, point);
	case POLYGON:
		ASSERT_NOT_REACHED();
		return false;
	}
}

bool isPointInCircle(Vec2 center, f32 radius, Vec2 point) {
	return center.distanceSquaredTo(point) <= radius * radius;
}
