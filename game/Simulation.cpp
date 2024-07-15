#include <game/Simulation.hpp>
#include <game/View2dUtils.hpp>
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
#include <gfx/Instancing.hpp>
#include <game/Shared.hpp>
#include <glad/glad.h>
#include <game/Constants.hpp>

Simulation::Simulation()
	: simulationGridSize(Constants::DEFAULT_GRID_SIZE.x + 2, Constants::DEFAULT_GRID_SIZE.y + 2)
	, u(Array2d<f32>::filled(simulationGridSize.x, simulationGridSize.y, 0.0f))
	, u_t(Array2d<f32>::filled(simulationGridSize.x, simulationGridSize.y, 0.0f))
	, speedSquared(Array2d<f32>::filled(simulationGridSize.x, simulationGridSize.y, 0.0f))
	, cellType(Array2d<CellType>::filled(simulationGridSize.x, simulationGridSize.y, CellType::EMPTY))
	, displayGrid(Array2d<Pixel32>::filled(simulationGridSize.x - 2, simulationGridSize.y - 2, Pixel32(0, 0, 0))) 
	, displayTexture(makePixelTexture(displayGrid.sizeX(), displayGrid.sizeY()))
	, reflectingObjects(List<ReflectingObject>::empty())
	, transmissiveObjects(List<TransmissiveObject>::empty())
	, mouseJoint(b2_nullJointId)
	, getShapesResult(List<b2ShapeId>::empty())
	, dt(1.0f / 60.0f) {

	for (i64 yi = 0; yi < simulationGridSize.y; yi++) {
		cellType(0, yi) = CellType::REFLECTING_WALL;
		cellType(simulationGridSize.x - 1, yi) = CellType::REFLECTING_WALL;
	}

	for (i64 xi = 1; xi < simulationGridSize.x - 1; xi++) {
		cellType(xi, 0) = CellType::REFLECTING_WALL;
		cellType(xi, simulationGridSize.y - 1) = CellType::REFLECTING_WALL;
	}

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
		b2BodyId boundaries = b2CreateBody(world, &bodyDef);
		b2ShapeDef shapeDef = b2DefaultShapeDef();

		b2Polygon bottom = b2MakeOffsetBox(boundsSize.x / 2.0f, halfWidth, b2Vec2{.x = 0.0f, .y = -(boundsSize.y / 2.0f + halfWidth) }, 0.0f);
		b2CreatePolygonShape(boundaries, &shapeDef, &bottom);

		b2Polygon top = b2MakeOffsetBox(boundsSize.x / 2.0f, halfWidth, b2Vec2{ .x = 0.0f, .y = (boundsSize.y / 2.0f + halfWidth) }, 0.0f);
		b2CreatePolygonShape(boundaries, &shapeDef, &top);


		b2Polygon left = b2MakeOffsetBox(halfWidth, boundsSize.y / 2.0f, b2Vec2{ .x = -(boundsSize.x / 2.0f + halfWidth), .y = 0.0f }, 0.0f);
		b2CreatePolygonShape(boundaries, &shapeDef, &left);

		b2Polygon right = b2MakeOffsetBox(halfWidth, boundsSize.y / 2.0f, b2Vec2{ .x = (boundsSize.x / 2.0f + halfWidth), .y = 0.0f }, 0.0f);
		b2CreatePolygonShape(boundaries, &shapeDef, &right);
	}
}

void Simulation::update(GameRenderer& renderer, const GameInput& input) {
	auto gridBounds = displayGridBounds();

	const auto cursorPos = Input::cursorPosClipSpace() * camera.clipSpaceToWorldSpace();
	const auto cursorDisplayGridPos = positionToGridPositionInGrid(cursorPos, gridBounds, simulationGridSize);
	std::optional<Vec2T<i64>> cursorSimulationGridPos;
	if (cursorDisplayGridPos.has_value()) {
		cursorSimulationGridPos = *cursorDisplayGridPos + Vec2T<i64>(1);
	}
	if (cursorSimulationGridPos.has_value() && Input::isMouseButtonHeld(MouseButton::RIGHT)) {
		fillCircle(u, *cursorSimulationGridPos, 3, 100.0f);
	}

	cameraMovement(camera, input, dt);
	
	updateMouseJoint(cursorPos, Input::isMouseButtonDown(MouseButton::LEFT), Input::isMouseButtonDown(MouseButton::LEFT));
	

	{
		i32 subStepCount = 4;
		b2World_Step(world, dt, subStepCount);
	}

	fill(cellType, CellType::EMPTY);

	#define FILL_SHAPE(TO_EXECUTE) \
		const auto position = toVec2(b2Body_GetPosition(object.id)); \
		const auto rotation = b2Body_GetAngle(object.id); \
		switch (object.shape.type) { \
			using enum ShapeType; \
			\
		case CIRCLE: { \
			const auto shapeAabb = circleAabb(position, object.shape.radius); \
			const auto shapeGridAabb = aabbToClampedGridAabb(shapeAabb, simulationGridBounds, simulationGridSize); \
			for (i64 yi = shapeGridAabb.min.y; yi <= shapeGridAabb.max.y; yi++) { \
				for (i64 xi = shapeGridAabb.min.x; xi <= shapeGridAabb.max.x; xi++) { \
					const auto cellCenter = calculateCellCenter(xi, yi); \
					if (isPointInCircle(position, object.shape.radius, cellCenter)) { \
						TO_EXECUTE \
					} \
				} \
			} \
			break; \
		} \
		case POLYGON: { \
			const auto shapeAabb = transformedPolygonAabb(constView(object.shape.simplifiedOutline), position, rotation); \
			const auto shapeGridAabb = aabbToClampedGridAabb(shapeAabb, simulationGridBounds, simulationGridSize); \
			const auto rotate = Rotation(-rotation); \
			for (i64 yi = shapeGridAabb.min.y; yi <= shapeGridAabb.max.y; yi++) { \
				for (i64 xi = shapeGridAabb.min.x; xi <= shapeGridAabb.max.x; xi++) { \
					auto cellCenter = calculateCellCenter(xi, yi); \
					cellCenter -= position; \
					cellCenter *= rotate; \
					if (isPointInPolygon(constView(object.shape.simplifiedOutline), cellCenter)) { \
						TO_EXECUTE \
					} \
				} \
			} \
			break; \
		} \
		\
		}

	const auto simulationGridBounds = this->simulationGridBounds();
	auto calculateCellCenter = [&](i64 x, i64 y) -> Vec2 {
		return Vec2(x - 0.5f, y - 0.5f) * Constants::CELL_SIZE + simulationGridBounds.min;
	};
	for (const auto& object : reflectingObjects) {
		FILL_SHAPE(cellType(xi, yi) = CellType::REFLECTING_WALL;)
	}
	{
		const auto defaultSpeed = 30.0f * Constants::CELL_SIZE;
		fill(speedSquared, pow(defaultSpeed, 2.0f));
		for (const auto& object : transmissiveObjects) {
			const auto speedOfTransmitionSquared = pow(object.speedOfTransmition, 2.0f);
			FILL_SHAPE(speedSquared(xi, yi) = speedOfTransmitionSquared;)
		}
	}

	/*for (i64 i = 0; i < 4; i++) {
		waveSimulationUpdate(dt / 4.0f);
	}*/
	waveSimulationUpdate(dt);

	render(renderer);
}

/*
There are 2 versions of the wave equation with variable speed.
1. Dtt u = c(x)^2 div grad u = c(x)^2 (Dxx u + Dyy u)
2. Dtt u = div (c(x)^2 grad u)

The first one is used for example in maxwells equation and the second one is used for surface water waves. https://www.sciencedirect.com/science/article/abs/pii/S0165212510000338

Discretizing Dxx u + Dyy u, assuming dx = dy
Dxx u + Dyy u =
(u(x + dl, y, 0) - 2u(x, y, 0) + u(x - dl, y, 0)) / dl^2 + (u(x, y + dl, 0) - 2u(x, y, 0) + u(x, y - dl, 0)) / dl^2 =
(u(x + dl, y, 0) + u(x - dl, y, 0) + (u(x, y + dl, 0) + u(x, y - dl, 0) - 4u(x, y, 0)) / dl^2

Could use an implicit method for the rhs. If 1. is used then you could calculate the inverse of the matrix once and reuse it. This wouldn't work for 2., because it has varying speed inside the operator.

For the time discretization I will use forward euler twice. I am not using a central difference approximation even though it has less error, because it requires values from the past frame. This might cause issues when the boundary conditions change between frames.
*/
void Simulation::waveSimulationUpdate(f32 deltaTime) {
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

			/*const auto laplacianU = (
				u(xi + 1, yi) * speedSquared(xi + 1, yi) + 
				u(xi - 1, yi) * speedSquared(xi - 1, yi) + 
				u(xi, yi + 1) * speedSquared(xi, yi + 1) + 
				u(xi, yi - 1) * speedSquared(xi, yi - 1) - 
				4.0f * u(xi, yi) * speedSquared(xi, yi)) / (Constants::CELL_SIZE * Constants::CELL_SIZE);*/
			/*const auto laplacianU = (
				u(xi + 1, yi) * speedSquared(xi + 1, yi) +
				u(xi - 1, yi) * speedSquared(xi - 1, yi) +
				u(xi, yi + 1) * speedSquared(xi, yi + 1) +
				u(xi, yi - 1) * speedSquared(xi, yi - 1) -
				4.0f * u(xi, yi) * speedSquared(xi, yi)) / (Constants::CELL_SIZE * Constants::CELL_SIZE);*/

			u_t(xi, yi) += laplacianU * speedSquared(xi, yi) * deltaTime;
		}
	}

	for (i32 yi = 1; yi < simulationGridSize.y - 1; yi++) {
		for (i32 xi = 1; xi < simulationGridSize.x - 1; xi++) {
			u(xi, yi) += deltaTime * u_t(xi, yi);
		}
	}
	for (i32 yi = 1; yi < simulationGridSize.y - 1; yi++) {
		for (i32 xi = 1; xi < simulationGridSize.x - 1; xi++) {
			//u_t(xi, yi) *= 0.9f;
			u_t(xi, yi) *= 0.99f;
			//u_t(xi, yi) *= 0.999f;
		}
	}
}

void Simulation::render(GameRenderer& renderer) {
	camera.aspectRatio = Window::aspectRatio();
	renderer.gfx.camera = camera;

	glViewport(0, 0, Window::size().x, Window::size().y);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	renderer.drawGrid();
	{
		for (i32 displayYi = 0; displayYi < displayGrid.sizeY(); displayYi++) {
			for (i32 displayXi = 0; displayXi < displayGrid.sizeX(); displayXi++) {
				const auto simulationXi = displayXi + 1;
				const auto simulationYi = displayYi + 1;

				auto& pixel = displayGrid(displayXi, displayYi);
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
		updatePixelTexture(displayGrid.data(), displayGrid.sizeX(), displayGrid.sizeY());

		const auto displayGridBounds = this->displayGridBounds();
		const auto displayGridBoundsSize = displayGridBounds.size();
		renderer.waveShader.use();
		const WaveInstance display{
			.transform = camera.makeTransform(displayGridBounds.center(), 0.0f, displayGridBounds.size() / 2.0f)
		};
		//camera.changeSizeToFitBox(displayGridBoundsSize);
		renderer.waveShader.setTexture("waveTexture", 0, displayTexture);
		drawInstances(renderer.waveVao, renderer.gfx.instancesVbo, View<const WaveInstance>(&display, 1), quad2dPtDrawInstances);
	}

	renderer.drawBounds(displayGridBounds());

	auto renderShape = [this, &renderer](b2BodyId id, const ShapeInfo& shape, bool isTransmissive) {
		const auto color = Vec4(GameRenderer::defaultColor, isTransmissive ? GameRenderer::transimittingShapeTransparency : 1.0f);
		const auto position = toVec2(b2Body_GetPosition(id));
		f32 rotation = b2Body_GetAngle(id);

		switch (shape.type) {
			using enum ShapeType;
		case CIRCLE:
			renderer.disk(position, shape.radius, rotation, color, false);
			break;

		case POLYGON:
			renderer.polygon(shape.vertices, shape.boundaryEdges, shape.trianglesVertices, position, rotation, color, false);
			break;
		}
	};

	for (const auto& object : reflectingObjects) {
		renderShape(object.id, object.shape, false);
	}

	for (const auto& object : transmissiveObjects) {
		renderShape(object.id, object.shape, true);
	}

	renderer.gfx.drawDisks();
	renderer.gfx.drawCircles();
	renderer.gfx.drawLines();
	renderer.gfx.drawFilledTriangles();
	glDisable(GL_BLEND);
}

void Simulation::reset() {
	for (auto& object : reflectingObjects) {
		b2DestroyBody(object.id);
	}
	reflectingObjects.clear();

	for (auto& object : transmissiveObjects) {
		b2DestroyBody(object.id);
	}
	transmissiveObjects.clear();

	fill(u, 0.0f);
	fill(u_t, 0.0f);
}

Aabb Simulation::displayGridBounds() const {
	return Constants::gridBounds(displayGrid.size());
}

Aabb Simulation::simulationGridBounds() const {
	auto bounds = displayGridBounds();
	bounds.min -= Vec2(Constants::CELL_SIZE);
	bounds.min += Vec2(Constants::CELL_SIZE);
	return bounds;
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
