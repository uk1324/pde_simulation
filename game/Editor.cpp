#include "Editor.hpp"
#include <game/Editor.hpp>
#include <game/Constants.hpp>
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
		bool cursorCaptured = false;


		if (selectedEntities.size() > 0) {
		/*	const auto& entity = *selectedEntities.begin();
			switch (entity.type) {
				using enum EditorEntityType;
			case REFLECTING_BODY: {
				const auto reflectingBodyId = entity.reflectingBody();
				auto reflectingBody = reflectingBodies.get(reflectingBodyId);
				if (!reflectingBody.has_value()) {
					CHECK_NOT_REACHED();
					break;
				}
				switch (reflectingBody->shape.type) {
					using enum EditorShapeType;
				case CIRCLE: {
					auto& circle = reflectingBody->shape.circle;
					const auto gizmoPosition = circle.center;

					const auto arrows = gizmo.getArrows(camera);
					const auto xDirection = arrows.xArrow.normalized();
					const auto yDirection = arrows.yArrow.normalized();

					const f32 xAxisGizmoDistance = distanceLineSegmentToPoint(gizmoPosition, gizmoPosition + arrows.xArrow, input.cursorPos);
					const f32 yAxisGizmoDistance = distanceLineSegmentToPoint(gizmoPosition, gizmoPosition + arrows.yArrow, input.cursorPos);
					const f32 centerDistance = gizmoPosition.distanceTo(input.cursorPos);

					const auto arrowWidth = gizmo.arrowWidth(camera);
					const auto acivationDistance = (arrowWidth / 2.0f) * 4.0f;

					if (input.cursorLeftDown) {
						cursorCaptured = true;
						if (centerDistance < acivationDistance) {
							gizmo.grabbedGizmo = Gizmo::GizmoType::CENTER;
						} else if (xAxisGizmoDistance < acivationDistance) {
							gizmo.grabbedGizmo = Gizmo::GizmoType::X_AXIS;
						} else if (yAxisGizmoDistance < acivationDistance) {
							gizmo.grabbedGizmo = Gizmo::GizmoType::Y_AXIS;
						}
						gizmo.grabStartPosition = input.cursorPos;
						gizmo.pos = circle.center;
					}

					if (input.cursorLeftHeld) {
						cursorCaptured = true;
						const auto cursorChangeSinceGrab = input.cursorPos - gizmo.grabStartPosition;
						switch (gizmo.grabbedGizmo) {
							using enum Gizmo::GizmoType;
							
						case CENTER:
							circle.center = gizmo.pos + cursorChangeSinceGrab;
							break;

						case X_AXIS:
							circle.center = gizmo.pos + xDirection * dot(cursorChangeSinceGrab, xDirection);
							break;

						case Y_AXIS:
							circle.center = gizmo.pos + yDirection * dot(cursorChangeSinceGrab, yDirection);
							break;

						case NONE:
							break;
						}
					}

					break;
				}
				case POLYGON:
					ASSERT_NOT_REACHED();
					break;

				}

				break;
			}

			}*/

		}

		/*if (!input.cursorLeftHeld) {
			gizmo.grabbedGizmo = Gizmo::GizmoType::NONE;
		}*/

		if (selectedEntities.size() > 0 && !cursorCaptured) {
			Vec2 center(0.0f);

			for (const auto& entity : selectedEntities) {
				center += entityGetPosition(entity);
			}
			center /= selectedEntities.size();

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

						//entity.
						//entitySetPosition(entity, shapeGetPosition() + result.translation);
						//i++;
					}

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

			if (input.deleteDown) {
				actions.beginMulticommand();
				for (auto& entity : selectedEntities) {
					destoryEntity(entity);
				}
				actions.endMulticommand();
			}
		}

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

	// Apply the camera movement after the frame so the cursor pos isn't delayed. Another option would be to update the cursor pos after the movement. Or just query the cursor pos each time.
	cameraMovement(camera, input, dt);

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

		switch (selectedTool) {
			using enum ToolType;

		case SELECT:
			selectToolGui();
			break;

		case CIRCLE:
		case RECTANGLE:
			break;
		}

		//ImGui::Button("");
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
			//polygonShapes.destroy(shape.polygon);
			break;
		}
		}
	}

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
	case RECTANGLE:
		break;
	
	case SELECT: {
		if (selectedEntities.size() == 1) {
			const auto& entity = *selectedEntities.begin();
			switch (entity.type) {
				using enum EditorEntityType;
			case REFLECTING_BODY: {
				const auto reflectingBodyId = entity.reflectingBody();
				const auto reflectingBody = reflectingBodies.get(reflectingBodyId);
				if (!reflectingBody.has_value()) {
					CHECK_NOT_REACHED();
					break;
				}
				switch (reflectingBody->shape.type) {
					using enum EditorShapeType;
				case CIRCLE: {
					auto& circle = reflectingBody->shape.circle;
					const auto arrows = gizmo.getArrows(camera);
					f32 width = gizmo.arrowWidth(camera);
					f32 tipLength = 0.06f / camera.zoom;
					renderer.gfx.arrow(circle.center, circle.center + arrows.xArrow, tipLength, 0.4f, width, Color3::RED);
					renderer.gfx.arrow(circle.center, circle.center + arrows.yArrow, tipLength, 0.4f, width, Color3::GREEN);
					renderer.gfx.disk(circle.center, width * 1.5f, Color3::BLUE * 0.7f);
					renderer.gfx.drawLines();
					renderer.gfx.drawDisks();
					break;
				}
				case POLYGON:
					ASSERT_NOT_REACHED();
					break;
					
				}

				break;
			}

			}

		} else if (selectedEntities.size() > 1) {
			
		}
	}


		break;
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
	actions.add(*this, EditorAction(std::move(action)));
}

bool Editor::firstFrame = true;

EditorPolygonShape EditorPolygonShape::DefaultInitialize::operator()() {
	return EditorPolygonShape{
		.vertices = List<Vec2>::empty()
	};
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

bool isPointInCircle(Vec2 center, f32 radius, Vec2 point) {
	return center.distanceSquaredTo(point) <= radius * radius;
}