#include <game/Simulation.hpp>
#include <game/View2dUtils.hpp>
#include <engine/Math/Interpolation.hpp>
#include <game/EditorEntities.hpp>
#include <Timer.hpp>
#include <Gui.hpp>
#include <engine/Math/Constants.hpp>
#include <game/RelativePositions.hpp>
#include <engine/Math/ShapeAabb.hpp>
#include <engine/Math/Rotation.hpp>
#include <engine/Math/PointInShape.hpp>
#include <engine/Math/Color.hpp>
#include <gfx2d/Quad2dPt.hpp>
#include <engine/Input/Input.hpp>
#include <game/Textures.hpp>
#include <engine/Window.hpp>
#include <game/GridUtils.hpp>
#include <game/Array2dDrawingUtils.hpp>
#include <game/Shaders/waveData.hpp>
#include <game/Shaders/waveDisplayData.hpp>
#include <gfx/Instancing.hpp>
#include <glad/glad.h>
#include <game/Constants.hpp>

i32 clamp(i32 i, i32 max) {
	if (i < 0) {
		return 0;
	}
	if (i >= max) {
		return max - 1;
	}
	return i;
}

Aabb transformedTriangleAabb(Vec2 v0, Vec2 v1, Vec2 v2, Vec2 position, Rotation rotation) {
	Vec2 points[]{ 
		rotation * v0 + position, 
		rotation * v1 + position, 
		rotation * v2 + position 
	};
	return Aabb::fromPoints(constView(points));
}

template<typename T>
void fillTriangle(View2d<T> a, Vec2 v0, Vec2 v1, Vec2 v2, Rotation rotation, Vec2 translation, T value, Aabb gridBounds, Vec2T<i64> gridSize) {
	const auto aabb = transformedTriangleAabb(v0, v1, v2, translation, rotation);
	const auto gridAabb = aabbToClampedGridAabb(aabb, gridBounds, gridSize);

	const auto a0 = v1 - v0;
	const auto a1 = v2 - v1;
	const auto a2 = v0 - v2;

	const auto rotationInversed = rotation.inversed();
	for (i64 yi = gridAabb.min.y; yi <= gridAabb.max.y; yi++) {
		for (i64 xi = gridAabb.min.x; xi <= gridAabb.max.x; xi++) {
			auto cellCenter = Vec2(xi - 0.5f, yi - 0.5f) * Constants::CELL_SIZE + gridBounds.min;
			cellCenter -= translation;
			cellCenter *= rotationInversed;

			const auto b0 = cellCenter - v0;
			auto area0 = a0.x * b0.y - b0.x * a0.y;
			if (area0 <= 0.0f) {
				continue;
			}
			const auto b1 = cellCenter - v1;
			auto area1 = a1.x * b1.y - b1.x * a1.y;
			if (area1 <= 0.0f) { 
				continue;
			}
			const auto b2 = cellCenter - v2;
			auto area2 = a2.x * b2.y - b2.x * a2.y;
			if (area2 <= 0.0f) {
				continue;
			}
			a(xi, yi) = value;
		}
	}
}

bool isPointInSimulationPolygon(View<const Vec2> verts, Vec2 p) {
	bool result = false;
	
	i64 i = 0;
	for (; i < verts.size(); i++) {
		if (verts[i] == Simulation::ShapeInfo::PATH_END_VERTEX) {
			result = isPointInPolygon(View<const Vec2>(verts.data(), i), p);
			i++;
			break;
		}
	}

	if (!result) {
		return false;
	}

	i64 start = i;
	for (; i < verts.size(); i++) {
		if (verts[i] == Simulation::ShapeInfo::PATH_END_VERTEX) {
			if (isPointInPolygon(View<const Vec2>(verts.data() + start, i - start), p)) {
				return false;
			}
			start = i + 1;
		}
	}

	return result;
}

Aabb simulationPolygonAabb(View<const Vec2> verts, Vec2 translation, f32 rotation) {
	for (i64 i = 0; i < verts.size(); i++) {
		if (verts[i] == Simulation::ShapeInfo::PATH_END_VERTEX) {
			return transformedPolygonAabb(View<const Vec2>(verts.data(), i), translation, rotation);;
		}
	}
	CHECK_NOT_REACHED();
	return Aabb(Vec2(0.0f), Vec2(0.0f));
}

Simulation::Simulation(Gfx2d& gfx)
	: simulationGridSize(Constants::DEFAULT_GRID_SIZE.x + 2, Constants::DEFAULT_GRID_SIZE.y + 2)
	, simulationSettings(SimulationSettings::makeDefault())
	, u(Array2d<f32>::filled(simulationGridSize.x, simulationGridSize.y, 0.0f))
	, u_t(Array2d<f32>::filled(simulationGridSize.x, simulationGridSize.y, 0.0f))
	, speedSquared(Array2d<f32>::filled(simulationGridSize.x, simulationGridSize.y, 0.0f))
	, cellType(Array2d<CellType>::filled(simulationGridSize.x, simulationGridSize.y, CellType::EMPTY))
	, debugDisplayGrid(Array2d<Pixel32>::filled(simulationGridSize.x - 2, simulationGridSize.y - 2, Pixel32(0, 0, 0))) 
	, debugDisplayTexture(makePixelTexture(debugDisplayGrid.sizeX(), debugDisplayGrid.sizeY()))
	, displayGrid(Array2d<f32>::filled(simulationGridSize.x - 2, simulationGridSize.y - 2, 0.0f))
	, displayGridTemp(Array2d<f32>::filled(simulationGridSize.x - 2, simulationGridSize.y - 2, 0.0f))
	, displayTexture(makeFloatTexture(debugDisplayGrid.sizeX(), debugDisplayGrid.sizeY()))
	, reflectingObjects(List<ReflectingObject>::empty())
	, transmissiveObjects(List<TransmissiveObject>::empty())
	, emitters(List<Emitter>::empty())
	, revoluteJoints(List<RevoluteJoint>::empty())
	, mouseJoint(b2_nullJointId)
	, getShapesResult(List<b2ShapeId>::empty())
	, realtimeDt(1.0f / 60.0f)
	, simulationElapsed(0.0f)
	, display3d(SimulationDisplay3d::make(gfx.instancesVbo)) {

	{
		b2WorldDef worldDef = b2DefaultWorldDef();
		worldDef.gravity = b2Vec2{ 0.0f, -10.0f };
		//worldDef.gravity = b2Vec2{ 0.0f, 0.0f };
		world = b2CreateWorld(&worldDef);
	}

	{
		const auto halfWidth = 10.0f;
		const auto bounds = displayGridBounds();
		const auto boundsSize = bounds.size();
		const auto boundsCenter = bounds.center();
		b2BodyDef bodyDef = b2DefaultBodyDef();
		bodyDef.position = fromVec2(boundsCenter);
		boundariesBodyId = b2CreateBody(world, &bodyDef);
		b2ShapeDef shapeDef = b2DefaultShapeDef();

		b2Polygon bottom = b2MakeOffsetBox(boundsSize.x / 2.0f, halfWidth, b2Vec2{.x = 0.0f, .y = -(boundsSize.y / 2.0f + halfWidth) }, 0.0f);
		b2CreatePolygonShape(boundariesBodyId, &shapeDef, &bottom);

		b2Polygon top = b2MakeOffsetBox(boundsSize.x / 2.0f, halfWidth, b2Vec2{ .x = 0.0f, .y = (boundsSize.y / 2.0f + halfWidth) }, 0.0f);
		b2CreatePolygonShape(boundariesBodyId, &shapeDef, &top);

		b2Polygon left = b2MakeOffsetBox(halfWidth, boundsSize.y / 2.0f, b2Vec2{ .x = -(boundsSize.x / 2.0f + halfWidth), .y = 0.0f }, 0.0f);
		b2CreatePolygonShape(boundariesBodyId, &shapeDef, &left);

		b2Polygon right = b2MakeOffsetBox(halfWidth, boundsSize.y / 2.0f, b2Vec2{ .x = (boundsSize.x / 2.0f + halfWidth), .y = 0.0f }, 0.0f);
		b2CreatePolygonShape(boundariesBodyId, &shapeDef, &right);
	}

	const auto grid3dSize = grid3dScale();
	display3d.camera.angleAroundUpAxis = 0.0f;
	display3d.camera.angleAroundRightAxis = PI<f32> / 4.0f;
	const auto gridCenter = grid3dSize / 2.0f;
	display3d.camera.position = gridCenter - display3d.camera.forward() * 4.0f;
}

std::optional<f32> rayPlaneIntersection(Vec3 planeNormal, Vec3 pointOnPlane, Vec3 rayStart, Vec3 rayDirection) {
	f32 denom = dot(planeNormal, rayDirection);
	if (abs(denom) > 1e-6) {
		Vec3 p0l0 = pointOnPlane - rayStart;
		f32 t = dot(p0l0, planeNormal) / denom;
		return t;
	}
	return std::nullopt;
}

template<typename T>
void fillShape(View2d<T> a, T value, Vec2 translation, f32 rotation, const Simulation::ShapeInfo& shape, Aabb gridBounds, Vec2T<i64> gridSize) {
	if (shape.type == Simulation::ShapeType::POLYGON) {
		for (i32 i = 0; i < shape.simplifiedTriangleVertices.size(); i += 3) {
			const auto v0 = shape.simplifiedTriangleVertices[i];
			const auto v1 = shape.simplifiedTriangleVertices[i + 1];
			const auto v2 = shape.simplifiedTriangleVertices[i + 2];
			fillTriangle(a, v0, v1, v2, rotation, translation, value, gridBounds, gridSize);
		}
	} else if (shape.type == Simulation::ShapeType::CIRCLE) {
		const auto shapeAabb = circleAabb(translation, shape.radius);
		const auto shapeGridAabb = aabbToClampedGridAabb(shapeAabb, gridBounds, gridSize);
		for (i64 yi = shapeGridAabb.min.y; yi <= shapeGridAabb.max.y; yi++) {
			for (i64 xi = shapeGridAabb.min.x; xi <= shapeGridAabb.max.x; xi++) {
				const auto cellCenter = Vec2(xi - 0.5f, yi - 0.5f) * Constants::CELL_SIZE + gridBounds.min;
				if (isPointInCircle(translation, shape.radius, cellCenter)) {
					a(xi, yi) = value;
				}
			}
		}
	}
}

Simulation::Result Simulation::update(GameRenderer& renderer, const GameInput& input, bool hideGui) {
	bool switchToEditor = false;
	if (!hideGui) {
		switchToEditor = gui();
	}

	const auto simulationDt = realtimeDt * simulationSettings.timeScale;
	simulationElapsed += simulationDt;

	if (displayMode == DisplayMode::DISPLAY_3D) {
		if (Input::isKeyDown(KeyCode::ESCAPE)) {
			Window::toggleCursor();
		}

		if (Window::isCursorEnabled()) {
			ImGui::GetIO().ConfigFlags &= ~FLAGS_ENABLED_WHEN_CURSOR_DISABLED;
		} else {
			ImGui::GetIO().ConfigFlags |= FLAGS_ENABLED_WHEN_CURSOR_DISABLED;
		}
	}

	if (displayMode == DisplayMode::DISPLAY_2D) {
		if (!Window::isCursorEnabled()) {
			Window::enableCursor();
		}
	}

	const auto grid3dScale = this->grid3dScale();

	// Could add option to change to mouse button down
	std::optional<Vec2> cursorPos;
	if (displayMode == DisplayMode::DISPLAY_2D) {
		cursorPos = input.cursorPos;
	} else if (displayMode == DisplayMode::DISPLAY_3D && !Window::isCursorEnabled()) {
		const auto rayStart = display3d.camera.position;
		const auto rayDirection = display3d.camera.forward();
		const auto intersectionT = rayPlaneIntersection(Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 0.0f), rayStart, rayDirection);
		if (intersectionT.has_value() && (*intersectionT) > 0.0f) {
			const Vec3 pos = rayStart + (*intersectionT) * rayDirection;
			const auto displaySize = displayGridBounds().size();
			cursorPos = Vec2(pos.x, pos.z) / Vec2(grid3dScale.x, grid3dScale.z) * displaySize;
		}
	}

	for (const auto& emitter : emitters) {
		if (emitter.activateOn.has_value() && !inputButtonIsHeld(*emitter.activateOn)) {
			continue;
		}
		const auto pos = getEmitterPos(emitter);
		runEmitter(pos, emitter.strength, emitter.oscillate, emitter.period, emitter.phaseOffset);
	}

	for (const auto& joint : revoluteJoints) { 
		if (joint.motorAlwaysEnabled) {
			b2RevoluteJoint_EnableMotor(joint.joint, true);
			b2RevoluteJoint_SetMotorSpeed(joint.joint, joint.motorSpeed * TAU<f32>);
			b2Body_SetAwake(joint.body0, true);
			b2Body_SetAwake(joint.body1, true);
		} else {
			bool enabled = false;
			f32 speed = 0.0f;

			if (joint.clockwiseKey.has_value() && inputButtonIsHeld(*joint.clockwiseKey)) {
				speed = -joint.motorSpeed * TAU<f32>;
				enabled = true;
			} else if (joint.counterclockwiseKey.has_value() && inputButtonIsHeld(*joint.counterclockwiseKey)) {
				speed = joint.motorSpeed * TAU<f32>;
				enabled = true;
			}

			if (enabled) {
				b2RevoluteJoint_EnableMotor(joint.joint, true);
				b2RevoluteJoint_SetMotorSpeed(joint.joint, speed);
				b2Body_SetAwake(joint.body0, true);
				b2Body_SetAwake(joint.body1, true);
			} else {
				b2RevoluteJoint_EnableMotor(joint.joint, false);
			}
		}
	}

	if (displayMode == DisplayMode::DISPLAY_2D) {
		cameraMovement(camera, input, realtimeDt);
	} else if (displayMode == DisplayMode::DISPLAY_3D) {
		if (!Window::isCursorEnabled()) {
			display3d.camera.update(realtimeDt);
		} else {
			display3d.camera.lastMousePosition = std::nullopt;
		}
	}
	
	if (displayMode == DisplayMode::DISPLAY_3D) {
		Input::ignoreImGuiWantCapture = true;
	}
	if (cursorPos.has_value() && Input::isMouseButtonHeld(MouseButton::RIGHT)) {
		runEmitter(*cursorPos, emitterStrengthSetting, emitterOscillateSetting, emitterPeriodSetting, emitterPhaseOffsetSetting);
	}

	if (cursorPos.has_value()) {
		updateMouseJoint(*cursorPos, Input::isMouseButtonDown(MouseButton::LEFT), Input::isMouseButtonDown(MouseButton::LEFT));
	}
	if (displayMode == DisplayMode::DISPLAY_3D) {
		Input::ignoreImGuiWantCapture = false;
	}

	if (!simulationSettings.paused) {
		b2World_Step(world, simulationDt, simulationSettings.rigidbodySimulationSubStepCount);

		const auto simulationGridBounds = this->simulationGridBounds();
		auto calculateCellCenter = [&](i64 x, i64 y) -> Vec2 {
			return Vec2(x - 0.5f, y - 0.5f) * Constants::CELL_SIZE + simulationGridBounds.min;
		};
		auto transform = [](Vec2 v, Vec2 translation, Rotation rotation) {
			return ((rotation * v) + translation) / Constants::CELL_SIZE;
		};

		{
			fill(cellType, CellType::EMPTY);
			auto cellTypeView = view2d(cellType);
			for (const auto& object : reflectingObjects) {
				const auto rotation = b2Body_GetAngle(object.id);
				const auto translation = toVec2(b2Body_GetPosition(object.id));
				fillShape(cellTypeView, CellType::REFLECTING_WALL, translation, rotation, object.shape, simulationGridBounds, simulationGridSize);
			}
		}

		{
			const auto defaultSpeed = 30.0f * Constants::CELL_SIZE;
			fill(speedSquared, pow(defaultSpeed, 2.0f));
			auto speedSquaredView = view2d(speedSquared);
			for (const auto& object : transmissiveObjects) {
				if (object.matchBackgroundSpeedOfTransmission) {
					continue;
				}
				const auto speedOfTransmitionSquared = pow(object.speedOfTransmition, 2.0f);
				const auto rotation = b2Body_GetAngle(object.id);
				const auto translation = toVec2(b2Body_GetPosition(object.id));
				fillShape(speedSquaredView, speedOfTransmitionSquared, translation, rotation, object.shape, simulationGridBounds, simulationGridSize);
			}
		}

		for (i64 i = 0; i < simulationSettings.waveEquationSimulationSubStepCount; i++) {
			waveSimulationUpdate(simulationDt / simulationSettings.waveEquationSimulationSubStepCount);
		}
	}

	render(renderer, grid3dScale);

	return Result{
		.switchToEditor = switchToEditor
	};
}

bool Simulation::gui() {
	ImGui::Begin(simulationSimulationSettingsWindowName);

	bool switchToEditor = ImGui::Button("switch to editor");
	ImGui::SetItemTooltip("Tab");

	ImGui::SeparatorText("emitter");
	{
		ImGui::TextDisabled("(?)");
		ImGui::SetItemTooltip("Use the right mouse button to activate emitter under cursor");

		if (gameBeginPropertyEditor("simulationEmitterSettings")) {
			emitterSettings(emitterStrengthSetting, emitterOscillateSetting, emitterPeriodSetting, emitterPhaseOffsetSetting);
			Gui::endPropertyEditor();
		}
		Gui::popPropertyEditor();
	}
	
	ImGui::SeparatorText("simulation");
	simulationSettingsGui(simulationSettings);

	ImGui::SeparatorText("display mode");
	ImGui::TextDisabled("(?)");
	ImGui::SetItemTooltip("use Escape to toggle cursor");
	const char* names[]{ "2D", "3D" };
	ImGui::Combo("display", reinterpret_cast<int*>(&displayMode), names, 2);

	ImGui::End();

	return switchToEditor;
}

void Simulation::waveSimulationUpdate(f32 simulationDt) {

	if (simulationSettings.topBoundaryCondition == SimulationBoundaryCondition::REFLECTING) {
		for (i64 xi = 1; xi < simulationGridSize.x - 1; xi++) {
			cellType(xi, simulationGridSize.y - 1) = CellType::REFLECTING_WALL;
		}
	}
	if (simulationSettings.bottomBoundaryCondition == SimulationBoundaryCondition::REFLECTING) {
		for (i64 xi = 1; xi < simulationGridSize.x - 1; xi++) {
			cellType(xi, 0) = CellType::REFLECTING_WALL;
		}
	}
	if (simulationSettings.leftBoundaryCondition == SimulationBoundaryCondition::REFLECTING) {
		for (i64 yi = 0; yi < simulationGridSize.y; yi++) {
			cellType(0, yi) = CellType::REFLECTING_WALL;
		}
	}
	if (simulationSettings.rightBoundaryCondition == SimulationBoundaryCondition::REFLECTING) {
		for (i64 yi = 0; yi < simulationGridSize.y; yi++) {
			cellType(simulationGridSize.x - 1, yi) = CellType::REFLECTING_WALL;
		}
	}
	
	for (i32 yi = 0; yi < simulationGridSize.y; yi++) {
		for (i32 xi = 0; xi < simulationGridSize.x; xi++) {
			switch (cellType(xi, yi)) {
			case CellType::EMPTY:
				break;

			case CellType::REFLECTING_WALL:
				// Dirichlet boundary conditions
				u(xi, yi) = 0.0f;
				// This shouldn't do anything.
				u_t(xi, yi) = 0.0f;
			}
		}
	}

	for (i32 yi = 1; yi < simulationGridSize.y - 1; yi++) {
		for (i32 xi = 1; xi < simulationGridSize.x - 1; xi++) {

			const auto laplacianU = (u(xi + 1, yi) + u(xi - 1, yi) + u(xi, yi + 1) + u(xi, yi - 1) - 4.0f * u(xi, yi)) / (Constants::CELL_SIZE * Constants::CELL_SIZE);

			u_t(xi, yi) += laplacianU * speedSquared(xi, yi) * simulationDt;
		}
	}

#define CALCULATE_U_T(xPos, yPos, normalDifference) \
	u_t(xPos, yPos) = sqrt(speedSquared(xPos, yPos)) * ((normalDifference) / Constants::CELL_SIZE)
	
	if (simulationSettings.bottomBoundaryCondition == SimulationBoundaryCondition::ABSORBING) {
		for (i32 xi = 1; xi < simulationGridSize.x - 1; xi++) {
			CALCULATE_U_T(xi, 1, u(xi, 2) - u(xi, 1));
		}
	}

	if (simulationSettings.topBoundaryCondition == SimulationBoundaryCondition::ABSORBING) {
		for (i32 xi = 1; xi < simulationGridSize.x - 1; xi++) {
			CALCULATE_U_T(xi, simulationGridSize.y - 2, u(xi, simulationGridSize.y - 3) - u(xi, simulationGridSize.y - 2));
		}
	}

	if (simulationSettings.leftBoundaryCondition == SimulationBoundaryCondition::ABSORBING) {
		for (i32 yi = 1; yi < simulationGridSize.y - 1; yi++) {
			CALCULATE_U_T(1, yi, u(2, yi) - u(1, yi));
		}
	}

	if (simulationSettings.rightBoundaryCondition == SimulationBoundaryCondition::ABSORBING) {
		for (i32 yi = 1; yi < simulationGridSize.y - 1; yi++) {
			CALCULATE_U_T(simulationGridSize.x - 2, yi, u(simulationGridSize.x - 3, yi) - u(simulationGridSize.x - 2, yi));
		}
	}
#undef CALCULATE_U_T
	for (i32 yi = 1; yi < simulationGridSize.y - 1; yi++) {
		for (i32 xi = 1; xi < simulationGridSize.x - 1; xi++) {
			u(xi, yi) += simulationDt * u_t(xi, yi);
		}
	}

	if (simulationSettings.dampingPerSecond != 1.0f) {
		const auto scale = exp(simulationDt * log(simulationSettings.dampingPerSecond));
		for (i32 yi = 1; yi < simulationGridSize.y - 1; yi++) {
			for (i32 xi = 1; xi < simulationGridSize.x - 1; xi++) {
				u(xi, yi) *= scale;
			}
		}
	}

	if (simulationSettings.speedDampingPerSecond != 1.0f) {
		const auto scale = exp(simulationDt * log(simulationSettings.speedDampingPerSecond));
		for (i32 yi = 1; yi < simulationGridSize.y - 1; yi++) {
			for (i32 xi = 1; xi < simulationGridSize.x - 1; xi++) {
				u_t(xi, yi) *= scale;
			}
		}
	}
}

void Simulation::render(GameRenderer& renderer, Vec3 grid3dScale) {
	camera.aspectRatio = Window::aspectRatio();
	renderer.gfx.camera = camera;

	glViewport(0, 0, Window::size().x, Window::size().y);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	renderer.drawGrid();
	if (debugDisplay) {
		for (i32 displayYi = 0; displayYi < debugDisplayGrid.sizeY(); displayYi++) {
			for (i32 displayXi = 0; displayXi < debugDisplayGrid.sizeX(); displayXi++) {
				const auto simulationXi = displayXi + 1;
				const auto simulationYi = displayYi + 1;

				auto& pixel = debugDisplayGrid(displayXi, displayYi);
				switch (cellType(simulationXi, simulationYi)) {
				case CellType::EMPTY: {
					// could smooth out the values before displaying
					const auto color = Color3::scientificColoring(u(simulationXi, simulationYi), -5.0f, 5.0f);
					pixel = Pixel32(color);
					break;
				}

				case CellType::REFLECTING_WALL:
					pixel = Pixel32(Vec3(0.5f));
					break;
				}
			}
		}
		debugDisplayTexture.bind();
		updatePixelTexture(debugDisplayGrid.data(), debugDisplayGrid.sizeX(), debugDisplayGrid.sizeY());

		const auto displayGridBounds = this->displayGridBounds();
		const auto displayGridBoundsSize = displayGridBounds.size();
		renderer.waveShader.use();
		const WaveInstance display{
			.transform = camera.makeTransform(displayGridBounds.center(), 0.0f, displayGridBounds.size() / 2.0f)
		};
		renderer.waveShader.setTexture("waveTexture", 0, debugDisplayTexture);
		drawInstances(renderer.waveVao, renderer.gfx.instancesVbo, View<const WaveInstance>(&display, 1), quad2dPtDrawInstances);
	} else {
		for (i32 displayYi = 0; displayYi < debugDisplayGrid.sizeY(); displayYi++) {
			for (i32 displayXi = 0; displayXi < debugDisplayGrid.sizeX(); displayXi++) {
				const auto simulationXi = displayXi + 1;
				const auto simulationYi = displayYi + 1;
				const auto min = -5.0f;
				const auto max = 5.0f;
				displayGridTemp(displayXi, displayYi) = (u(simulationXi, simulationYi) - min) / (max - min);
				//displayGridTemp(displayXi, displayYi) = (u_t(simulationXi, simulationYi) - min) / (max - min);
			}
		}

		//ImGui::Checkbox("applyBlurToDisplayGrid", &applyBlurToDisplayGrid);

		if (applyBlurToDisplayGrid) {
			for (i32 displayYi = 0; displayYi < debugDisplayGrid.sizeY(); displayYi++) {
				for (i32 displayXi = 0; displayXi < debugDisplayGrid.sizeX(); displayXi++) {
					f32 sum = 0.0f;
					f32 kernel[3][3] = {
						{ 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f },
						{ 2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f },
						{ 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f },
					};

					for (i64 j = 0; j < 3; j++) {
						for (i64 i = 0; i < 3; i++) {
							i64 x = displayXi + (i - 1);
							i64 y = displayYi + (j - 1);

							if (x < 0) {
								x = 0;
							}
							if (y < 0) {
								y = 0;
							}
							if (x >= displayGrid.sizeX()) {
								x = displayGrid.sizeX() - 1;
							}
							if (y >= displayGrid.sizeY()) {
								y = displayGrid.sizeY() - 1;
							}

							sum += displayGridTemp(x, y) * kernel[i][j];
						}
					}
					displayGrid(displayXi, displayYi) = sum;
				}
			}
		} else {
			for (i32 displayYi = 0; displayYi < debugDisplayGrid.sizeY(); displayYi++) {
				for (i32 displayXi = 0; displayXi < debugDisplayGrid.sizeX(); displayXi++) {
					displayGrid(displayXi, displayYi) = displayGridTemp(displayXi, displayYi);
				}
			}
		}

		displayTexture.bind();
		updateFloatTexture(displayGrid.data(), displayGrid.sizeX(), displayGrid.sizeY());

		const auto displayGridBounds = this->displayGridBounds();
		const auto displayGridBoundsSize = displayGridBounds.size();
		renderer.waveDisplayShader.use();
		const WaveDisplayInstance display{
			.transform = camera.makeTransform(displayGridBounds.center(), 0.0f, displayGridBounds.size() / 2.0f)
		};
		renderer.waveDisplayShader.setTexture("waveTexture", 0, displayTexture);
		drawInstances(renderer.waveVao, renderer.gfx.instancesVbo, View<const WaveDisplayInstance>(&display, 1), quad2dPtDrawInstances);
	}

	if (displayMode == DisplayMode::DISPLAY_2D) {
		renderer.drawBounds(displayGridBounds());

		auto renderShape = [this, &renderer](b2BodyId id, const ShapeInfo& shape, bool isTransmissive) {
			const auto color = Vec4(GameRenderer::defaultColor, isTransmissive ? GameRenderer::transimittingShapeTransparency : 1.0f);
			const auto position = toVec2(b2Body_GetPosition(id));
			f32 rotation = b2Body_GetAngle(id);
			const auto isStatic = b2Body_GetType(id) == b2_staticBody;

			switch (shape.type) {
				using enum ShapeType;
			case CIRCLE:
				renderer.disk(position, shape.radius, rotation, color, renderer.outlineColor(color.xyz(), false), isStatic);
				break;

			case POLYGON:
				renderer.polygon(shape.vertices, shape.boundary, shape.trianglesVertices, position, rotation, color, renderer.outlineColor(color.xyz(), false), isStatic);
				break;
			}
		};
		renderer.gfx.drawLines();

		/*for (const auto& a : reflectingObjects) {
			const auto position = toVec2(b2Body_GetPosition(a.id));
			f32 rotation = b2Body_GetAngle(a.id);
			std::vector<Vec2> vs;
			for (i64 i = 0; i < a.shape.simplifiedOutline.size(); i++) {
				if (a.shape.simplifiedOutline[i] == Simulation::ShapeInfo::PATH_END_VERTEX) {
					break;
				}
				vs.push_back(Rotation(rotation) * a.shape.simplifiedOutline[i] + position);
			}
			renderer.gfx.polylineTriangulated(constView(vs), 0.1f, Color3::WHITE);
		}*/

		auto debugRenderPolygon = [this, &renderer](b2BodyId id) {
			const auto position = toVec2(b2Body_GetPosition(id));
			f32 rotation = b2Body_GetAngle(id);

			getShapes(id);
			for (auto& shape : getShapesResult) {
				auto type = b2Shape_GetType(shape);
				switch (type) {
				case b2_polygonShape: {
					const auto rotate = Rotation(rotation);
					const auto polygon = b2Shape_GetPolygon(shape);

					auto transform = [&](Vec2 v) -> Vec2 {
						v *= rotate;
						v += position;
						return v;
					};

					i64 previous = polygon.count - 1;
					for (i64 i = 0; i < polygon.count; i++) {
						const auto currentVertex = transform(toVec2(polygon.vertices[i]));
						const auto previousVertex = transform(toVec2(polygon.vertices[previous]));
						renderer.gfx.line(previousVertex, currentVertex, renderer.outlineWidth() / 1.5f, Color3::WHITE / 2.0f);
						previous = i;
					}
					break;
				}

				case b2_circleShape:
				case b2_capsuleShape:
				case b2_segmentShape:
				case b2_smoothSegmentShape:
				case b2_shapeTypeCount:
					ASSERT_NOT_REACHED();
					break;
				}
			}
		};

		for (const auto& object : reflectingObjects) {
			renderShape(object.id, object.shape, false);
			//debugRenderPolygon(object.id);
		}
		renderer.gfx.drawLines();

		for (const auto& object : transmissiveObjects) {
			renderShape(object.id, object.shape, true);
		}

		for (const auto& emitter : emitters) {
			renderer.emitter(getEmitterPos(emitter), false, false);
		}

		auto getAbsolutePosition = [](b2BodyId id, Vec2 relativePosition) -> Vec2 {
			const auto pos = toVec2(b2Body_GetPosition(id));
			const auto rotation = b2Body_GetAngle(id);
			return calculatePositionFromRelativePosition(relativePosition, pos, rotation);
		};

		for (const auto& joint : revoluteJoints) {
			const auto pos0 = getAbsolutePosition(joint.body0, joint.positionRelativeToBody0);
			const auto pos1 = getAbsolutePosition(joint.body1, joint.positionRelativeToBody1);
			renderer.revoluteJoint(pos0, pos1);
		}

		renderer.gfx.drawDisks();
		renderer.gfx.drawCircles();
		renderer.gfx.drawLines();
		renderer.gfx.drawFilledTriangles();
		glDisable(GL_BLEND);
	} else if (displayMode == DisplayMode::DISPLAY_3D) {
		glViewport(0, 0, Window::size().x, Window::size().y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		display3d.renderGrid(renderer.gfx.instancesVbo, displayTexture, grid3dScale);

		const auto topHeight = 0.4f;
		const auto bottomHeight = -topHeight;

		for (const auto& object : reflectingObjects) {
			const auto isStatic = b2Body_GetType(object.id) == b2_staticBody;

			auto addVertex = [&](Vec2 worldPos, f32 y) -> i32 {
				const auto color = renderer.insideColor(Vec4(GameRenderer::defaultColor, 1.0f), isStatic).xyz();
				const auto scale = Vec2(1.0f) / displayGridBounds().size() * Vec2(grid3dScale.x, grid3dScale.z);
				return display3d.addVertex(Vertex3Pnc{
					.position = Vec3(worldPos.x * scale.x, y, worldPos.y * scale.y),
					.normal = Vec3(0.0f, 1.0f, 0.0f),
					.color = color
				});
			};

			const auto& s = object.shape;
			const auto position = toVec2(b2Body_GetPosition(object.id));
			const auto rotation = Rotation(b2Body_GetAngle(object.id));

			if (s.type == ShapeType::POLYGON) {
				const auto topIndicesOffset = display3d.trianglesVertices.size();
				for (i32 i = 0; i < s.vertices.size(); i++) {
					const auto vertex = rotation * s.vertices[i] + position;
					addVertex(vertex, topHeight);
				}
				for (i32 i = 0; i < s.trianglesVertices.size(); i++) {
					display3d.trianglesIndices.push_back(topIndicesOffset + s.trianglesVertices[i]);
				}
				const auto bottomIndicesOffset = display3d.trianglesVertices.size();
				for (i32 i = 0; i < s.vertices.size(); i++) {
					const auto vertex = rotation * s.vertices[i] + position;
					addVertex(vertex, bottomHeight);
				}

				if (s.boundary.size() > 1) {
					i64 startVertexI = 0;
					for (i64 i = 0; i < s.boundary.size(); i++) {
						if (s.boundary[i + 1] == EditorPolygonShape::PATH_END_INDEX) {
							display3d.addQuad(
								bottomIndicesOffset + s.boundary[i],
								bottomIndicesOffset + s.boundary[startVertexI],
								topIndicesOffset + s.boundary[startVertexI],
								topIndicesOffset + s.boundary[i]
							);

							startVertexI = i + 2;
							if (i == s.boundary.size() - 2) {
								break;
							}
							i++;
							continue;
						}
						display3d.addQuad(
							bottomIndicesOffset + s.boundary[i], 
							bottomIndicesOffset + s.boundary[i + 1],
							topIndicesOffset + s.boundary[i + 1],
							topIndicesOffset + s.boundary[i]
						);
					}
				}

				
			} else if (s.type == ShapeType::CIRCLE) {
				const auto center = addVertex(position, topHeight);
				const auto firstPos = position + Vec2(s.radius, 0.0f);
				const auto firstVertexTop = addVertex(firstPos, topHeight);
				const auto firstVertexBottom = addVertex(firstPos, bottomHeight);
				u32 oldVertexTop = firstVertexTop;
				u32 oldVertexBottom = firstVertexBottom;
				const auto vertices = renderer.gfx.calculateCircleVertexCount(s.radius);
				for (i32 i = 1; i < vertices ; i++) {
					const auto t = f32(i) / f32(vertices - 1);
					const auto angle = lerp(0.0f, TAU<f32>, t);
					const auto pos = position + Vec2::fromPolar(angle, s.radius);
					const auto newVertexTop = addVertex(pos, topHeight);
					const auto newVertexBottom = addVertex(pos, bottomHeight);
					display3d.addQuad(oldVertexBottom, newVertexBottom, newVertexTop, oldVertexTop);
					display3d.addTriangle(center, newVertexTop, oldVertexTop);
					oldVertexTop = newVertexTop;
					oldVertexBottom = newVertexBottom;
				}
			}
		}
		display3d.renderTriangles();


		glEnable(GL_BLEND);
		renderer.gfx.diskTriangulated(camera.pos, 0.01f / camera.zoom, Vec4(Color3::WHITE, 0.2f));
		glDisable(GL_BLEND);
		renderer.gfx.drawFilledTriangles();
		glDisable(GL_DEPTH_TEST);
	}	
}

void Simulation::runEmitter(Vec2 pos, f32 strength, bool oscillate, f32 period, f32 phaseOffset) {
	const auto gridBounds = displayGridBounds();
	const auto gridPosition = positionToGridPosition(pos, gridBounds, simulationGridSize);

	f32 finalStrength;
	if (oscillate) {
		finalStrength = sin((simulationElapsed / period + phaseOffset) * TAU<f32>);
		//strength = std::max(0.0f, ) * emitter.strength;
		finalStrength *= strength;
	} else {
		finalStrength = strength;
	}

	fillCircle(u, gridPosition, 3, finalStrength);
}

void Simulation::reset() {
	for (auto& joint : revoluteJoints) {
		b2DestroyJoint(joint.joint);
	}
	revoluteJoints.clear();

	for (auto& object : reflectingObjects) {
		b2DestroyBody(object.id);
	}
	reflectingObjects.clear();

	for (auto& object : transmissiveObjects) {
		b2DestroyBody(object.id);
	}
	transmissiveObjects.clear();

	emitters.clear();

	fill(u, 0.0f);
	fill(u_t, 0.0f);

	simulationElapsed = 0.0f;
}

Aabb Simulation::displayGridBounds() const {
	return Constants::gridBounds(debugDisplayGrid.size());
}

Aabb Simulation::simulationGridBounds() const {
	auto bounds = displayGridBounds();
	bounds.min -= Vec2(Constants::CELL_SIZE);
	bounds.min += Vec2(Constants::CELL_SIZE);
	return bounds;
}

Vec3 Simulation::grid3dScale() {
	const auto displaySize = displayGridBounds().size();
	return Vec3(displaySize.x, 3.0f, displaySize.y) * 0.2f;
}

Vec2 Simulation::getEmitterPos(const Emitter& emitter) {
	const auto translation = toVec2(b2Body_GetPosition(emitter.body));
	const auto rotation = b2Body_GetAngle(emitter.body);
	Vec2 pos = emitter.positionRelativeToBody;
	pos *= Rotation(rotation);
	pos += translation;
	return pos;
}

void Simulation::getShapes(b2BodyId body) {
	const auto shapeCount = b2Body_GetShapeCount(body);
	getShapesResult.resizeWithoutInitialization(shapeCount);
	b2Body_GetShapes(body, getShapesResult.data(), shapeCount);
}

void Simulation::updateMouseJoint(Vec2 cursorPos, bool inputIsUp, bool inputIsDown) {
	struct QueryContext {
		b2Vec2 point;
		b2BodyId bodyId = b2_nullBodyId;
	};

	auto queryCallback = [](b2ShapeId shapeId, void* context) -> bool {
		QueryContext* queryContext = static_cast<QueryContext*>(context);

		b2BodyId bodyId = b2Shape_GetBody(shapeId);
		b2BodyType bodyType = b2Body_GetType(bodyId);
		if (bodyType != b2_dynamicBody) {
			// continue query
			return true;
		}

		bool overlap = b2Shape_TestPoint(shapeId, queryContext->point);
		if (overlap) {
			// found shape
			queryContext->bodyId = bodyId;
			return false;
		}

		return true;
	};

	if (inputIsDown) {
		b2AABB cursorAabb;
		b2Vec2 d = { 0.001f, 0.001f };
		const auto p = fromVec2(cursorPos);
		cursorAabb.lowerBound = b2Sub(p, d);
		cursorAabb.upperBound = b2Add(p, d);

		QueryContext queryContext = { p, b2_nullBodyId };
		b2World_OverlapAABB(world, cursorAabb, b2DefaultQueryFilter(), queryCallback, &queryContext);

		if (B2_IS_NON_NULL(queryContext.bodyId)) {
			b2BodyDef bodyDef = b2DefaultBodyDef();
			mouseJointBody0 = b2CreateBody(world, &bodyDef);

			b2MouseJointDef mouseDef = b2DefaultMouseJointDef();
			mouseDef.bodyIdA = mouseJointBody0;
			mouseDef.bodyIdB = queryContext.bodyId;
			mouseDef.target = p;
			mouseDef.hertz = 5.0f;
			mouseDef.dampingRatio = 0.7f;
			mouseDef.maxForce = 1000.0f * b2Body_GetMass(queryContext.bodyId);
			mouseJoint = b2CreateMouseJoint(world, &mouseDef);

			b2Body_SetAwake(queryContext.bodyId, true);
		}
	}

	if (Input::isMouseButtonUp(MouseButton::LEFT)) {
		if (!b2Joint_IsValid(mouseJoint)) {
			// The world or attached body was destroyed.
			mouseJoint = b2_nullJointId;
		}

		if (B2_IS_NON_NULL(mouseJoint)) {
			b2DestroyJoint(mouseJoint);
			mouseJoint = b2_nullJointId;

			b2DestroyBody(mouseJointBody0);
			mouseJointBody0 = b2_nullBodyId;
		}
	}

	if (!b2Joint_IsValid(mouseJoint)) {
		// The world or attached body was destroyed.
		mouseJoint = b2_nullJointId;
	}

	if (B2_IS_NON_NULL(mouseJoint)) {
		b2MouseJoint_SetTarget(mouseJoint, fromVec2(cursorPos));
		b2BodyId bodyIdB = b2Joint_GetBodyB(mouseJoint);
		b2Body_SetAwake(bodyIdB, true);
	}
}
