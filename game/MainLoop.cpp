#include <game/MainLoop.hpp>
#include <glad/glad.h>
#include <game/Shared.hpp>
#include <engine/Window.hpp>
#include <game/Shaders/waveData.hpp>
#include <gfx/Instancing.hpp>
#include <gfx2d/Quad2dPt.hpp>
#include "MatrixUtils.hpp"
#include <engine/Input/Input.hpp>
#include <engine/Math/DouglassPecker.hpp>
#include <dependencies/earcut/earcut.hpp>
#include <imgui/imgui_internal.h>
#include <imgui/imgui.h>

MainLoop::MainLoop() 
	: renderer(GameRenderer::make())
	, editor(Editor::make())
	, simulation(renderer.gfx) {

	ImGui::GetStyle().FrameRounding = 5;
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

	if (Input::isKeyDown(KeyCode::F3)) {
		hideGui = !hideGui;
	}

	if (Input::isKeyHeld(KeyCode::LEFT_CONTROL) && Input::isKeyDownWithAutoRepeat(KeyCode::Z)) {
		input.undoDown = true;
	}

	if (Input::isKeyHeld(KeyCode::LEFT_CONTROL) && Input::isKeyDownWithAutoRepeat(KeyCode::Y)) {
		input.redoDown = true;
	}

	if (input.switchModeDown) {
		if (currentState == State::EDITOR) {
			switchFromEditorToSimulation();
		} else if (currentState == State::SIMULATION) {
			switchFromSimulationToEditor();
		}
	}

	{
		auto id = ImGui::DockSpaceOverViewport(
			ImGui::GetMainViewport(),
			ImGuiDockNodeFlags_NoDockingOverCentralNode | ImGuiDockNodeFlags_PassthruCentralNode);

		// Putting this here, because using the dock builder twice (once in simulation once in editor), breaks it (the windows get undocked). 
		// Just using a single id created here doesn't fix anything.
		static bool firstFrame = true;
		if (firstFrame) {
			ImGui::DockBuilderRemoveNode(id);
			ImGui::DockBuilderAddNode(id, ImGuiDockNodeFlags_DockSpace);

			const auto leftId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 0.5f, nullptr, &id);
			ImGui::DockBuilderSetNodeSize(leftId, ImVec2(0.2f * ImGui::GetIO().DisplaySize.x, 1.0f));

			ImGui::DockBuilderDockWindow(simulationSimulationSettingsWindowName, leftId);
			ImGui::DockBuilderDockWindow(editorSimulationSettingsWindowName, leftId);
			ImGui::DockBuilderDockWindow(editorEditorSettingsWindowName, leftId);

			ImGui::DockBuilderFinish(id);

			firstFrame = false;
		}
	}

	switch (currentState) {
		using enum State;

	case EDITOR: {
		const auto result = editor.update(renderer, input);
		if (result.switchToSimulation) {
			switchFromEditorToSimulation();
		}
		break;
	}
		
	case SIMULATION: {
		const auto result = simulation.update(renderer, input, hideGui);
		if (result.switchToEditor) {
			switchFromSimulationToEditor();
		}
		break;
	}
	}
}

void MainLoop::switchFromEditorToSimulation() {
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
		auto simplifiedTriangleVertices = List<Vec2>::empty();
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
					const auto vertex = getTriangulationVertex(index);
					simplifiedTriangleVertices.add(vertex);
					hull.points[j] = fromVec2(vertex);
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
			.simplifiedTriangleVertices = std::move(simplifiedTriangleVertices),
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
}

void MainLoop::switchFromSimulationToEditor() {
	currentState = State::EDITOR;
	editor.camera = simulation.camera;
	Window::enableCursor();
	ImGui::GetIO().ConfigFlags &= ~FLAGS_ENABLED_WHEN_CURSOR_DISABLED;
}
