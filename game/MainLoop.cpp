#include <game/MainLoop.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <game/Shaders/waveData.hpp>
#include <gfx/Instancing.hpp>
#include <gfx2d/Quad2dPt.hpp>
#include "MatrixUtils.hpp"
#include <engine/Input/Input.hpp>
#include <engine/Math/DouglassPecker.hpp>
#include <dependencies/earcut/earcut.hpp>

#include <imgui/imgui.h>

MainLoop::MainLoop() 
	: renderer(GameRenderer::make())
	, editor(Editor::make()) {

		//{
		//	ImVec4* colors = ImGui::GetStyle().Colors;
		//	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		//	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		//	colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		//	colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		//	colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
		//	colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
		//	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
		//	colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		//	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
		//	colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		//	colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		//	colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		//	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		//	colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		//	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		//	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		//	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
		//	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		//	colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		//	colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		//	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		//	colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		//	colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
		//	colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		//	colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		//	colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
		//	colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
		//	colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		//	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		//	colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		//	colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		//	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		//	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		//	colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		//	colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		//	colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
		//	colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		//	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		//	colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		//	colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		//	colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		//	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		//	colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		//	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		//	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		//	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		//	colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		//	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		//	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		//	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		//	colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		//	colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		//	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
		//	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
		//	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

		//	ImGuiStyle& style = ImGui::GetStyle();
		//	style.WindowPadding = ImVec2(8.00f, 8.00f);
		//	style.FramePadding = ImVec2(5.00f, 2.00f);
		//	style.CellPadding = ImVec2(6.00f, 6.00f);
		//	style.ItemSpacing = ImVec2(6.00f, 6.00f);
		//	style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
		//	style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
		//	style.IndentSpacing = 25;
		//	style.ScrollbarSize = 15;
		//	style.GrabMinSize = 10;
		//	style.WindowBorderSize = 1;
		//	style.ChildBorderSize = 1;
		//	style.PopupBorderSize = 1;
		//	style.FrameBorderSize = 1;
		//	style.TabBorderSize = 1;
		//	style.WindowRounding = 7;
		//	style.ChildRounding = 4;
		//	style.FrameRounding = 3;
		//	style.PopupRounding = 4;
		//	style.ScrollbarRounding = 9;
		//	style.GrabRounding = 3;
		//	style.LogSliderDeadzone = 4;
		//	style.TabRounding = 4;
		//}

	/*for (i64 i = 0; i < ImGuiCol_COUNT; i++) {
		std::swap(ImGui::GetStyle().Colors[i].y, ImGui::GetStyle().Colors[i].z);
		ImGui::GetStyle().Colors[i].z *= 2.0f;
	}*/

	//for (i64 i = 0; i < ImGuiCol_COUNT; i++) {
	//	//std::swap(ImGui::GetStyle().Colors[i].y, ImGui::GetStyle().Colors[i].z);
	//	f32 scale = 1.0f;
	//	if (ImGui::GetStyle().Colors[i].y != ImGui::GetStyle().Colors[i].z) {
	//		scale = 0.6f;
	//	}
	//	/*ImGui::GetStyle().Colors[i].y = ImGui::GetStyle().Colors[i].z;
	//	ImGui::GetStyle().Colors[i].z *= scale;*/
	//	ImGui::GetStyle().Colors[i].y = ImGui::GetStyle().Colors[i].z;
	//	ImGui::GetStyle().Colors[i].x *= scale;
	//	ImGui::GetStyle().Colors[i].y *= scale;
	//	ImGui::GetStyle().Colors[i].z *= scale;
	//}
	ImGui::GetStyle().FrameRounding = 5;

	//for (i64 i = 0; i < ImGuiCol_COUNT; i++) {
	//	std::swap(ImGui::GetStyle().Colors[i].y, ImGui::GetStyle().Colors[i].z);
	//	auto avg = (ImGui::GetStyle().Colors[i].x + ImGui::GetStyle().Colors[i].y + ImGui::GetStyle().Colors[i].z) / 3.0f;
	//	avg *= 2.0f;
	//	ImGui::GetStyle().Colors[i].x = avg;
	//	ImGui::GetStyle().Colors[i].y = avg;
	//	ImGui::GetStyle().Colors[i].z = avg;
	//}
	//ImVec4* colors = ImGui::GetStyle().Colors;
	//colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.94f);
	//colors[ImGuiCol_Border] = ImVec4(0.66f, 0.66f, 0.66f, 0.50f);
	//colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.46f, 0.56f, 0.54f);
	//colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.54f, 0.64f, 0.40f);
	//colors[ImGuiCol_FrameBgActive] = ImVec4(0.76f, 0.93f, 0.98f, 0.67f);
	//colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.92f, 0.92f, 1.00f, 1.00f);
	//colors[ImGuiCol_CheckMark] = ImVec4(0.47f, 0.89f, 1.00f, 1.00f);
	//colors[ImGuiCol_ButtonHovered] = ImVec4(0.11f, 0.45f, 0.54f, 1.00f);
	//colors[ImGuiCol_Separator] = ImVec4(0.63f, 0.63f, 0.63f, 0.50f);
}

void MainLoop::update() {
	ShaderManager::update();

	GameInput input{
		.upButtonHeld = Input::isKeyHeld(KeyCode::W),
		.downButtonHeld = Input::isKeyHeld(KeyCode::S),
		.leftButtonHeld = Input::isKeyHeld(KeyCode::A),
		.rightButtonHeld = Input::isKeyHeld(KeyCode::D),
		.cursorPos = Input::cursorPosClipSpace() * (currentState == State::EDITOR ? editor.camera : simulation.camera).clipSpaceToWorldSpace(),
		.cursorLeftDown = Input::isMouseButtonDown(MouseButton::LEFT),
		.cursorLeftUp = Input::isMouseButtonUp(MouseButton::LEFT),
		.cursorLeftHeld = Input::isMouseButtonHeld(MouseButton::LEFT),
		.cursorRightDown = Input::isMouseButtonDown(MouseButton::RIGHT),
		.undoDown = false,
		.redoDown = false,
		.ctrlHeld = Input::isKeyHeld(KeyCode::LEFT_CONTROL),
		.deleteDown = Input::isKeyDown(KeyCode::DELETE),
		.switchModeDown = Input::isKeyDown(KeyCode::TAB)
	};

	if (Input::isKeyHeld(KeyCode::LEFT_CONTROL) && Input::isKeyDownWithAutoRepeat(KeyCode::Z)) {
		input.undoDown = true;
	}

	if (Input::isKeyHeld(KeyCode::LEFT_CONTROL) && Input::isKeyDownWithAutoRepeat(KeyCode::Y)) {
		input.redoDown = true;
	}

	if (input.switchModeDown) {
		if (currentState == State::EDITOR) {
			currentState = State::SIMULATION;
			simulation.camera = editor.camera;
			simulation.reset();
			simulation.simulationSettings = editor.simulationSettings;

			editorRigidBodyIdToPhysicsId.clear();
			for (auto body : editor.rigidBodies) {

				b2BodyDef bodyDef = b2DefaultBodyDef();
				bodyDef.type = body->isStatic ? b2_staticBody : b2_dynamicBody;

				b2BodyId bodyId = b2CreateBody(simulation.world, &bodyDef);
				editorRigidBodyIdToPhysicsId.insert({ body.id, bodyId });

				b2ShapeDef shapeDef = b2DefaultShapeDef();
				shapeDef.density = 1.0f;
				shapeDef.friction = 0.3f;
				shapeDef.filter.categoryBits = body->collisionCategories;
				shapeDef.filter.maskBits = body->collisionMask;

				/*if (body->collisionCategories != 0b1) {
					body->collisionCategories &= ~0b1;
				}*/

				// TODO: Could reuse vertices
				auto simplifiedOutline = List<Vec2>::empty();
				auto vertices = List<Vec2>::empty();
				auto boundary = List<i32>::empty();
				auto triangleVertices = List<i32>::empty();
				auto shapeType = Simulation::ShapeType::CIRCLE;
				f32 radius = 0.0f;

				switch (body->shape.type) {
					using enum EditorShapeType;
				case CIRCLE: {
					const auto& circle = body->shape.circle;
					b2Circle physicsCircle{ .center = b2Vec2(0.0f), .radius = circle.radius };
					radius = circle.radius;
					b2CreateCircleShape(bodyId, &shapeDef, &physicsCircle);
					b2Body_SetTransform(bodyId, fromVec2(circle.center), circle.angle);
					break;
				}

				case POLYGON: {
					shapeType = Simulation::ShapeType::POLYGON;
					const auto& polygon = editor.polygonShapes.get(body->shape.polygon);
					if (!polygon.has_value()) {
						CHECK_NOT_REACHED();
						break;
					}
					// @Performance:
					std::vector <std::vector<Vec2>> polygonToTriangulate;
					std::vector<Vec2> verticesToTriangulate;
					for (i64 i = 0; i < polygon->boundary.size(); i++) {
						if (polygon->boundary[i] == EditorPolygonShape::PATH_END_INDEX) {
							const auto simplifiedVertices = polygonDouglassPeckerSimplify(constView(verticesToTriangulate), 0.1f);
							verticesToTriangulate.clear();
							polygonToTriangulate.push_back(simplifiedVertices);

						} else {
							verticesToTriangulate.push_back(Vec2(polygon->vertices[polygon->boundary[i]]));
						}
					}
					const auto triangulation = mapbox::earcut(polygonToTriangulate);

					for (const auto& path : polygonToTriangulate) {
						for (const auto& vertex : path) {
							simplifiedOutline.add(vertex);
						}
						simplifiedOutline.add(Simulation::ShapeInfo::PATH_END_VERTEX);
					}
					boundary = polygon->boundary.clone();
					triangleVertices = polygon->trianglesVertices.clone();
					vertices = polygon->vertices.clone();

					auto getTriangulationVertex = [&](i64 i) -> Vec2 {
						for (const auto& path : polygonToTriangulate) {
							if (i < path.size()) {
								return path[i];
							}
							i -= path.size();
						}
						CHECK_NOT_REACHED();
						return Vec2(0.0f);
					};

					ASSERT(triangulation.size() % 3 == 0);
					for (i64 i = 0; i < triangulation.size(); i += 3) {
						b2Hull hull;
						for (i64 j = 0; j < 3; j++) {
							const auto index = triangulation[i + j];
							//hull.points[j] = fromVec2(polygonToTriangulate[0][index]);
							hull.points[j] = fromVec2(getTriangulationVertex(index));
						}
						hull.count = 3;
						const auto polygon = b2MakePolygon(&hull, 0.0f);
						b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
					}
					b2Body_SetTransform(bodyId, fromVec2(polygon->translation), polygon->rotation);
					break;
				}

				}

				Simulation::ShapeInfo shapeInfo{
					.type = shapeType,
					.simplifiedOutline = std::move(simplifiedOutline),
					.vertices = std::move(vertices),
					.boundary = std::move(boundary),
					.trianglesVertices = std::move(triangleVertices),
					.radius = radius
				};

				switch (body->material.type) {
					using enum EditorMaterialType;
				case RELFECTING:
					simulation.reflectingObjects.add(Simulation::ReflectingObject{
						.id = bodyId,
						.shape = std::move(shapeInfo)
					});
					break;

				case TRANSIMISIVE:
					simulation.transmissiveObjects.add(Simulation::TransmissiveObject{
						.id = bodyId,
						.shape = std::move(shapeInfo),
						.speedOfTransmition = body->material.transimisive.speedOfTransmition,
						.matchBackgroundSpeedOfTransmission = body->material.transimisive.matchBackgroundSpeedOfTransmission
					});
					break;

				}

			}

			auto positionRelativeToBoundaryBody = [&](Vec2 pos) {
				return pos - toVec2(b2Body_GetPosition(simulation.boundariesBodyId));
			};



			for (const auto& emitter : editor.emitters) {
				b2BodyId bodyId;
				Vec2 position(0.0f);
				if (emitter->rigidBody.has_value()) {
					const auto physicsId = editorRigidBodyIdToPhysicsId.find(*emitter->rigidBody);
					if (physicsId == editorRigidBodyIdToPhysicsId.end()) {
						CHECK_NOT_REACHED();
						continue;
					}
					bodyId = physicsId->second;
					position = emitter->position;
				} else {
					bodyId = simulation.boundariesBodyId;
					position = positionRelativeToBoundaryBody(emitter->position);
				}
				
				simulation.emitters.add(Simulation::Emitter{
					.body = bodyId,
					.positionRelativeToBody = position,
					.strength = emitter->strength,
					.oscillate = emitter->oscillate,
					.period = emitter->period,
					.phaseOffset = emitter->phaseOffset,
					.activateOn = emitter->activateOn,
				});
			}

			for (const auto& joint : editor.revoluteJoints) {
				b2BodyId bodyId0;
				Vec2 position0(0.0f);
				b2BodyId bodyId1;
				Vec2 position1(0.0f);

				if (joint->body0.has_value()) {
					const auto physicsId0 = editorRigidBodyIdToPhysicsId.find(*joint->body0);
					if (physicsId0 == editorRigidBodyIdToPhysicsId.end()) {
						CHECK_NOT_REACHED();
						continue;
					}
					bodyId0 = physicsId0->second;
					position0 = joint->position0;
				} else {
					bodyId0 = simulation.boundariesBodyId;
					position0 = positionRelativeToBoundaryBody(joint->position0);
				}

				{
					const auto physicsId1 = editorRigidBodyIdToPhysicsId.find(joint->body1);
					if (physicsId1 == editorRigidBodyIdToPhysicsId.end()) {
						CHECK_NOT_REACHED();
						continue;
					}
					bodyId1 = physicsId1->second;
					position1 = joint->position1;
				}

				b2RevoluteJointDef jointDef = b2DefaultRevoluteJointDef();
				jointDef.bodyIdA = bodyId0;
				jointDef.localAnchorA = fromVec2(position0);
				jointDef.bodyIdB = bodyId1;
				jointDef.localAnchorB = fromVec2(position1);

				auto jointId = b2CreateRevoluteJoint(simulation.world, &jointDef);
				b2RevoluteJoint_SetMaxMotorTorque(jointId, joint->motorMaxTorque);

				simulation.revoluteJoints.add(Simulation::RevoluteJoint{
					.body0 = bodyId0,
					.positionRelativeToBody0 = position0,
					.body1 = bodyId1,
					.positionRelativeToBody1 = position1,
					.joint = jointId,
					.motorSpeed = joint->motorSpeed,
					.motorAlwaysEnabled = joint->motorAlwaysEnabled,
					.clockwiseKey = joint->clockwiseKey,
					.counterclockwiseKey = joint->counterclockwiseKey,
				});
			}
		} else if (currentState == State::SIMULATION) {
			currentState = State::EDITOR;
			editor.camera = simulation.camera;
		}
	}

	switch (currentState) {
		using enum State;

	case EDITOR:
		editor.update(renderer, input);
		break;

	case SIMULATION:
		simulation.update(renderer, input);
		break;
	}
}
