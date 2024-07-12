#include "Editor.hpp"
#include <game/Editor.hpp>
#include <game/Constants.hpp>
#include <engine/Math/PointInShape.hpp>
#include <engine/Input/Input.hpp>
#include <game/Shared.hpp>
#include <engine/Math/Color.hpp>
#include <engine/Math/Rotation.hpp>
#include <imgui/imgui.h>
#include <Gui.hpp>
#include <imgui/imgui_internal.h>
#include <glad/glad.h>

EditorShape::EditorShape(const EditorCircleShape& circle)
	: circle(circle)
	, type(EditorShapeType::CIRCLE) {}

EditorShape::EditorShape(const EditorPolygonShapeId& polygon)
	: polygon(polygon)
	, type(EditorShapeType::POLYGON) {}

bool EditorShape::operator==(const EditorShape& other) const {
	if (other.type != type) {
		return false;
	}

	switch (type) {
		using enum EditorShapeType;
	case CIRCLE: return circle == other.circle;
	case POLYGON: return polygon == other.polygon;
	}

	CHECK_NOT_REACHED();
	return false;
}

Editor Editor::make() {
	Editor editor{
		.polygonTool = PolygonTool::make(),
		.gizmoSelectedShapesAtGrabStart = List<EditorShape>::empty(),
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

void Editor::update(GameRenderer& renderer, const GameInput& input) {
	reflectingBodies.update();

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
		selectToolUpdate(input);
		break;
	}

	case CIRCLE: {
		auto circleShape = circleTool.update(input.cursorPos, input.cursorLeftDown, input.cursorRightDown);
		if (circleShape.has_value()) {
			createObject(EditorShape(std::move(*circleShape)));
		}
		break;
	}
			   
	case POLYGON:
		const auto finished = polygonTool.update(input.cursorPos, input.cursorLeftDown, input.cursorLeftHeld, Input::isKeyDown(KeyCode::LEFT_SHIFT));
		if (finished) {
			auto shape = createPolygonShape();
			shape->initializeFromVertices(constView(polygonTool.vertices));
			polygonTool.vertices.clear();
			createObject(EditorShape(shape.id));
		}
		break;
		
	}

	render(renderer, input);
	gui();

	// Apply the camera movement after the frame so the cursor pos isn't delayed. Another option would be to update the cursor pos after the movement. Or just query the cursor pos each time.
	cameraMovement(camera, input, dt);

}

void Editor::selectToolUpdate(const GameInput& input) {
	bool cursorCaptured = false;

	if (selectedEntities.size() > 0 && !cursorCaptured) {
		const auto center = selectedEntitiesCenter();

		const auto result = gizmo.update(camera, center, input.cursorPos, input.cursorLeftDown, input.cursorLeftUp, input.cursorLeftHeld);
		switch (result.state) {
			using enum Gizmo::State;

		case INACTIVE:
			break;

		case GRAB_START:
			cursorCaptured = true;
			gizmoSelectedShapesAtGrabStart.clear();
			for (auto& entity : selectedEntities) {
				switch (entity.type) {
					using enum EditorEntityType;
				case REFLECTING_BODY: {
					auto body = reflectingBodies.get(entity.reflectingBody());
					if (!body.has_value()) {
						CHECK_NOT_REACHED();
						continue;
					}
					gizmoSelectedShapesAtGrabStart.add(cloneShape(body->shape));
					break;

				}
				}
			}
			break;

		case GRAB_END: {
			cursorCaptured = true;

			if (selectedEntities.size() != gizmoSelectedShapesAtGrabStart.size()) {
				CHECK_NOT_REACHED();
				break;
			}

			i64 i = 0;
			actions.beginMulticommand();
			for (auto& entity : selectedEntities) {
				switch (entity.type) {
					using enum EditorEntityType;
				case REFLECTING_BODY: {
					auto body = reflectingBodies.get(entity.reflectingBody());
					if (!body.has_value()) {
						CHECK_NOT_REACHED();
						continue;
					}
					const auto old = EditorReflectingBody(gizmoSelectedShapesAtGrabStart[i]);
					auto newShape = cloneShape(old.shape);
					shapeSetPosition(newShape, shapeGetPosition(gizmoSelectedShapesAtGrabStart[i]) + result.translation);
					actions.add(*this, EditorAction(EditorActionModifyReflectingBody(entity.reflectingBody(), old, EditorReflectingBody(newShape))));
					break;

				}

				}
				i++;
			}
			actions.endMulticommand();

			break;
		}

		case GRABBED: {
			cursorCaptured = true;

			if (selectedEntities.size() != gizmoSelectedShapesAtGrabStart.size()) {
				CHECK_NOT_REACHED();
				break;
			}
			i64 i = 0;
			for (auto& entity : selectedEntities) {
				entitySetPosition(entity, shapeGetPosition(gizmoSelectedShapesAtGrabStart[i]) + result.translation);
				i++;
			}
		}


		}
	}

	if (!cursorCaptured) {
		if (input.cursorLeftDown) {
			selectTool.grabStartPos = input.cursorPos;
			cursorCaptured = true;
		}

		if (selectTool.grabStartPos.has_value() && input.cursorLeftUp) {
			const auto selectionBox = selectTool.selectionBox(camera, input.cursorPos);
			selectTool.grabStartPos = std::nullopt;
			cursorCaptured = true;

			// @Performance
			const auto selectedEntitiesBefore = selectedEntities;

			if (selectionBox.has_value()) {
				if (!input.ctrlHeld) {
					selectedEntities.clear();
				}

				for (auto body : reflectingBodies) {
					if (isEditorShapeContainedInAabb(body->shape, *selectionBox)) {
						selectedEntities.insert(EditorEntityId(body.id));
					}
				}

			} else {
				// TODO: Make an option to enable a modal menu that would allow the user to choose which entity to select if there are multiple entities under the cursor.
				std::optional<EditorEntityId> entityUnderCursor;

				// Place the joints on top. Iterate over them first.

				if (!entityUnderCursor.has_value()) {
					for (auto body : reflectingBodies) {
						if (isPointInEditorShape(body->shape, input.cursorPos)) {
							entityUnderCursor = EditorEntityId(body.id);
							break;
						}
					}
				}

				if (entityUnderCursor.has_value()) {
					if (input.ctrlHeld) {
						if (selectedEntities.contains(*entityUnderCursor)) {
							selectedEntities.erase(*entityUnderCursor);
						} else {
							selectedEntities.insert(*entityUnderCursor);
						}
					} else {
						if (selectedEntities.contains(*entityUnderCursor)) {
							// do nothing
						} else {
							selectedEntities.clear();
							selectedEntities.insert(*entityUnderCursor);
						}
					}
				} else {
					selectedEntities.clear();
				}
			}

			if (selectedEntities != selectedEntitiesBefore) {
				actions.addSelectionChange(*this, selectedEntitiesBefore, selectedEntities);
			}
		}
	}
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
	
		struct ToolDisplay {
			ToolType type;
			const char* name;
			const char* tooltip;
		};

		const ToolDisplay tools[]{
			{ ToolType::SELECT, "select", "" },
			{ ToolType::CIRCLE, "circle", "" },
			{ ToolType::POLYGON, "polygon", "Press shift to finish drawing." },
		};
		
		ImGui::SeparatorText("tool");

		const auto oldSelection = selectedTool;

		for (const auto& tool : tools) {
			if (ImGui::Selectable(tool.name, tool.type == selectedTool)) {
				selectedTool = tool.type;
			}
			ImGui::SetItemTooltip(tool.tooltip);
		}

		if (selectedTool == ToolType::SELECT && oldSelection != ToolType::SELECT) {
			selectedEntities.clear();
		}

		switch (selectedTool) {
			using enum ToolType;

		case SELECT:
			selectToolGui();
			break;

		case POLYGON:
			break;

		case CIRCLE:
			break;
		}

		ImGui::End();
	}
}

void Editor::selectToolGui() {
	ImGui::SeparatorText("selection");
	if (selectedEntities.size() == 0) {
		ImGui::TextWrapped("no entities selected");
	} else if (selectedEntities.size() == 1) {
		auto& id = *selectedEntities.begin();
		entityGui(id);
	} else {
		//actions.beginMulticommand();
	}
}

void Editor::entityGui(EditorEntityId id) {
	if (Gui::beginPropertyEditor()) {
		switch (id.type) {
			using enum EditorEntityType;

		case REFLECTING_BODY: {
			auto body = reflectingBodies.get(id.reflectingBody());
			if (!body.has_value()) {
				CHECK_NOT_REACHED();
				break;
			}
			// @Performance:
			auto old = *body;
			ImGui::TextWrapped("shape");
			shapeGui(body->shape);
			// @Performance: could make the gui return if modified and only then add.
			if (old != *body) {
				actions.add(*this, EditorAction(EditorActionModifyReflectingBody(id.reflectingBody(), old, *body)));
			}
		}

		}

		Gui::endPropertyEditor();
	}
	Gui::popPropertyEditor();
}

void Editor::shapeGui(EditorShape& shape) {
	switch (shape.type) {
		using enum EditorShapeType;
	case CIRCLE: {
		auto& circle = shape.circle;
		Gui::inputVec2("position", circle.center);
		Gui::inputFloat("radius", circle.radius);
		break;
	}

	case POLYGON:
		ASSERT_NOT_REACHED();

	}
}

void Editor::render(GameRenderer& renderer, const GameInput& input) {
	renderer.gfx.camera = camera;

	renderer.drawGrid();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	renderer.drawBounds(roomBounds);

	for (const auto& body : reflectingBodies) {
		const auto isSelected = selectedTool == ToolType::SELECT && selectedEntities.contains(EditorEntityId(body.id));

		switch (body->shape.type) {
			using enum EditorShapeType;
		case CIRCLE: {
			const auto circle = body->shape.circle;
			renderer.disk(circle.center, circle.radius, circle.angle, Color3::WHITE / 2.0f, isSelected);
			break;
		}

		case POLYGON: {
			const auto& polygon = polygonShapes.get(body->shape.polygon);
			if (!polygon.has_value()) {
				CHECK_NOT_REACHED();
				break;
			}
			const auto outlineColor = renderer.outlineColor(Color3::WHITE / 2.0f, isSelected);
			for (i64 i = 0; i < polygon->boundaryEdges.size(); i += 2) {
				const auto startIndex = polygon->boundaryEdges[i];
				const auto endIndex = polygon->boundaryEdges[i + 1];
				renderer.gfx.line(polygon->vertices[startIndex], polygon->vertices[endIndex], GameRenderer::outlineWidth, outlineColor);
			}
 			renderer.gfx.filledTriangles(constView(polygon->vertices), constView(polygon->trianglesVertices), Color3::WHITE / 2.0f);
			break;
		}

		}
	}

	renderer.gfx.drawFilledTriangles();
	renderer.gfx.drawDisks();
	renderer.gfx.drawCircles();
	renderer.gfx.drawLines();

	switch (selectedTool) {
		using enum ToolType;

	case CIRCLE:
		circleTool.render(renderer, input.cursorPos);
		renderer.gfx.drawDisks();
		renderer.gfx.drawCircles();
		renderer.gfx.drawLines();
		break;

	case POLYGON: {
		//renderer.gfx.polyline(constView(polygonTool.vertices), GameRenderer::outlineWidth, Color3::WHITE);
		const auto outlineColor = renderer.outlineColor(Color3::WHITE / 2.0f, true);
		if (polygonTool.vertices.size() > 1) {
			renderer.gfx.polyline(constView(polygonTool.vertices), GameRenderer::outlineWidth, outlineColor);
			//renderer.gfx.polyline(constView(polygonTool.vertices), GameRenderer::outlineWidth / 1.3f, outlineColor);
		} else if (polygonTool.vertices.size() == 1) {
			renderer.gfx.disk(polygonTool.vertices[0], GameRenderer::outlineWidth / 2.0f, outlineColor);
		}
		break;
	}
		
	
	case SELECT: {
		if (selectedEntities.size() > 0) {
			const auto center = selectedEntitiesCenter();
			const auto arrows = gizmo.getArrows(camera);
			f32 width = gizmo.arrowWidth(camera);
			f32 tipLength = 0.06f / camera.zoom;
			renderer.gfx.arrow(center, center + arrows.xArrow, tipLength, 0.4f, width, Color3::RED);
			renderer.gfx.arrow(center, center + arrows.yArrow, tipLength, 0.4f, width, Color3::GREEN);
			renderer.gfx.disk(center, width * 1.5f, Color3::BLUE * 0.7f);
			renderer.gfx.drawLines();
			renderer.gfx.drawDisks();
			break;
		}

		const auto selectionBox = selectTool.selectionBox(camera, input.cursorPos);
		if (selectionBox.has_value()) {
			renderer.gfx.rect(selectionBox->min, selectionBox->size(), 0.008f / renderer.gfx.camera.zoom, Color3::WHITE);
			renderer.gfx.drawLines();
		}
		break;
	}


	}

	glDisable(GL_BLEND);
}

void Editor::destoryAction(EditorAction& action) {
	switch (action.type) {
		using enum EditorActionType;
	case CREATE_ENTITY: {
		fullyDeleteEntity(action.createEntity.id);
		break;
	}

	case DESTROY_ENTITY: {
		fullyDeleteEntity(action.destoryEntity.id);
		break;
	}

	case SELECTION_CHANGE: {
		actions.freeSelectionChange(action.selectionChange);
		break;
	}

	case MODIFY_REFLECTING_BODY:
		break;

	}
}

void Editor::redoAction(const EditorAction& action) {
	switch (action.type) {
		using enum EditorActionType;
	case CREATE_ENTITY: {
		activateEntity(action.createEntity.id);
		break;
	}
	
	case DESTROY_ENTITY: {
		deactivateEntity(action.destoryEntity.id);
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

	case MODIFY_REFLECTING_BODY: {
		const auto& a = action.modifyReflectingBody;
		auto body = reflectingBodies.get(a.id);
		if (!body.has_value()) {
			CHECK_NOT_REACHED();
			break;
		}
		*body = a.newEntity;
		break;
	}

	}
}

void Editor::undoAction(const EditorAction& action) {
	switch (action.type) {
		using enum EditorActionType;
	case CREATE_ENTITY: {
		deactivateEntity(action.createEntity.id);
		break;
	}

	case DESTROY_ENTITY: {
		activateEntity(action.destoryEntity.id);
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

	case MODIFY_REFLECTING_BODY: {
		const auto& a = action.modifyReflectingBody;
		auto body = reflectingBodies.get(a.id);
		if (!body.has_value()) {
			CHECK_NOT_REACHED();
			break;
		}
		*body = a.oldEntity;
		break;
	}
		
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

Aabb Editor::editorShapeAabb(const EditorShape& shape) const {
	switch (shape.type) {
		using enum EditorShapeType;

	case CIRCLE: 
		return Aabb(shape.circle.center - Vec2(shape.circle.radius), shape.circle.center + Vec2(shape.circle.radius));

	case POLYGON: {
		ASSERT_NOT_REACHED();
		//polygonShapes.destroy(shape.polygon);
		break;
	}

	}

	CHECK_NOT_REACHED();

	return Aabb(Vec2(0.0f), Vec2(0.0f));
}

bool Editor::isEditorShapeContainedInAabb(const EditorShape& shape, const Aabb& aabb) const {
	// shape contained in aabb <=> shape's aabb contained in aabb.
	// <= because shape is a subset of shape's aabb which is a subset of aabb
	// => because if a shape is contained in an aabb all of it's coordinate extream along the axes are also contained. The shape's aabb extrema are the same as the shape's extrema so it's also contained.
	return aabb.contains(editorShapeAabb(shape));
}

void Editor::fullyDeleteEntity(const EditorEntityId& id) {
	switch (id.type) {
		using enum EditorEntityType;
	case REFLECTING_BODY: {
		const auto entity = reflectingBodies.getEvenIfInactive(id.reflectingBody());
		if (!entity.has_value()) {
			CHECK_NOT_REACHED();
			break;
		}
		deleteShape(entity->shape);
		break;
	}
		
	}
}

void Editor::activateEntity(const EditorEntityId& id) {
	switch (id.type) {
		using enum EditorEntityType;

	case REFLECTING_BODY:
		reflectingBodies.activate(id.reflectingBody());
		break;
	}
}

void Editor::deactivateEntity(const EditorEntityId& id) {
	switch (id.type) {
		using enum EditorEntityType;
	case REFLECTING_BODY:
		reflectingBodies.deactivate(id.reflectingBody());
		break;
	}
}

Vec2 Editor::selectedEntitiesCenter() {
	Vec2 center(0.0f);

	for (const auto& entity : selectedEntities) {
		center += entityGetPosition(entity);
	}
	center /= selectedEntities.size();

	return center;
}

void Editor::destoryEntity(const EditorEntityId& id) {
	actions.add(*this, EditorAction(EditorActionDestroyEntity(id)));
	switch (id.type) {
		using enum EditorEntityType;
	case REFLECTING_BODY: {
		deactivateEntity(id);
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

Vec2 Editor::entityGetPosition(const EditorEntityId& id) {
	switch (id.type) {
		using enum EditorEntityType;
	case REFLECTING_BODY: {
		const auto body = reflectingBodies.get(id.reflectingBody());
		if (!body.has_value()) {
			CHECK_NOT_REACHED();
			break;
		}
		return shapeGetPosition(body->shape);
	}
	}

	CHECK_NOT_REACHED();
	return Vec2(0.0f);
}

void Editor::entitySetPosition(const EditorEntityId& id, Vec2 position) {
	switch (id.type) {
		using enum EditorEntityType;
	case REFLECTING_BODY: {
		auto body = reflectingBodies.get(id.reflectingBody());
		if (!body.has_value()) {
			CHECK_NOT_REACHED();
			break;
		}
		shapeSetPosition(body->shape, position);
		break;
	}
		 

	default:
		break;
	}
}

Vec2 Editor::shapeGetPosition(const EditorShape& shape) const {
	switch (shape.type) {
		using enum EditorShapeType;
	case CIRCLE: return shape.circle.center;
	case POLYGON:
		ASSERT_NOT_REACHED();
		break;
	}

	CHECK_NOT_REACHED();
	return Vec2(0.0f);
}

void Editor::shapeSetPosition(EditorShape& shape, Vec2 position) {
	switch (shape.type) {
		using enum EditorShapeType;
	case CIRCLE: shape.circle.center = position; break;
	case POLYGON:
		ASSERT_NOT_REACHED();
		break;
	}
}

EditorShape Editor::cloneShape(const EditorShape& shape) {
	switch (shape.type) {
		using enum EditorShapeType;
	case CIRCLE:
		return shape;
		break;

	case POLYGON:
		ASSERT_NOT_REACHED();
		break;
	}

	CHECK_NOT_REACHED();
	return EditorShape(EditorCircleShape(Vec2(0.0f), 0.0f, 0.0f));
}


EntityArrayPair<EditorPolygonShape> Editor::createPolygonShape() {
	auto shape = polygonShapes.create();
	shape->vertices.clear();
	return shape;
}

EntityArrayPair<EditorReflectingBody> Editor::createReflectingBody(const EditorShape& shape) {
	auto body = reflectingBodies.create();
	body->shape = shape;
	return body;
}

void Editor::createObject(EditorShape&& shape) {	
 	auto body = createReflectingBody(shape);
	auto action = EditorActionCreateEntity(EditorEntityId(body.id));
	actions.add(*this, EditorAction(std::move(action)));
}

bool Editor::firstFrame = true;

EditorPolygonShape EditorPolygonShape::DefaultInitialize::operator()() {
	return EditorPolygonShape::make();
}

EditorReflectingBody EditorReflectingBody::DefaultInitialize::operator()() {
	return EditorReflectingBody(EditorShape(EditorCircleShape(Vec2(0.0f), 0.0f, 0.0f)));
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

Editor::PolygonTool Editor::PolygonTool::make() {
	return PolygonTool{
		.drawing = false,
		.vertices = List<Vec2>::empty(),
	};
}

bool Editor::PolygonTool::update(Vec2 cursorPos, bool drawDown, bool drawHeld, bool closeCurveDown) {
	/*
	Not sure what is the best way to make the polygon drawing work. 
	It's nice to be able to swicht between straight lines and normal drawing and it's also nice to be able to just draw a shape and have the line connect to the endpoint without having to move the cursor there.

	1. Could do what algodoo does.
	2. Could have a button that finishes the curve.
	Number 1 makes drawing straight lines less comfortable and number 2 makes finishing the curve less comfortable.
	Number 2 probably should have an undo redo.
	*/

	if (drawDown) {
		drawing = true;
	}


	if (closeCurveDown) {
		drawing = false;
		return true;
	} else if (drawing && drawHeld) {
		if (vertices.size() == 0) {
			vertices.add(cursorPos);
		} else if (vertices.back().distanceTo(cursorPos) > 0.05f) {
			vertices.add(cursorPos);
		}
	}
	return false;
}

std::optional<Aabb> Editor::SelectTool::selectionBox(const Camera& camera, Vec2 cursorPos) const {
	if (!grabStartPos.has_value()) {
		return std::nullopt;
	}

	// If the box is too small treat it as a click selection not a box selection.
	const auto box = Aabb::fromCorners(*grabStartPos, cursorPos);
	if (box.area() < 0.03f / camera.zoom) {
		return std::nullopt;
	}

	return box;
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
	CHECK_NOT_REACHED();
	return false;
}

bool isCircleInsideAabb(Vec2 center, f32 radius, const Aabb& aabb)
{
	return false;
}
