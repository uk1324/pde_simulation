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

MainLoop::MainLoop() 
	: renderer(GameRenderer::make())
	, editor(Editor::make()) {}

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
					position = emitter->position - toVec2(b2Body_GetPosition(simulation.boundariesBodyId));
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
