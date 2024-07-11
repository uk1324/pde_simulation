#include <game/MainLoop.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <game/Shaders/waveData.hpp>
#include <gfx/Instancing.hpp>
#include <gfx2d/Quad2dPt.hpp>
#include "MatrixUtils.hpp"
#include <engine/Input/Input.hpp>

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
			for (auto body : editor.reflectingBodies) {
				switch (body->shape.type) {
					using enum EditorShapeType;
				case CIRCLE: {
					const auto& circle = body->shape.circle;
					b2BodyDef bodyDef = b2DefaultBodyDef();
					bodyDef.type = b2_dynamicBody;
					bodyDef.position = fromVec2(circle.center);
					bodyDef.angle = circle.angle;
					b2BodyId bodyId = b2CreateBody(simulation.world, &bodyDef);
					//b2Polygon dynamicBox = b2MakeBox(0.1f, 10.0f);
					b2Circle physicsCircle{ .center = b2Vec2(0.0f), .radius = circle.radius };

					b2ShapeDef shapeDef = b2DefaultShapeDef();
					shapeDef.density = 1.0f;
					shapeDef.friction = 0.3f;

					b2CreateCircleShape(bodyId, &shapeDef, &physicsCircle);
					simulation.objects.add(Simulation::Object{ .id = bodyId });
					break;
				}
				case POLYGON:
					ASSERT_NOT_REACHED();
					//b2MakePolygon()
					//b2Polygon
// 
					//b2CreatePolygonShape()
					break;

				}
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
