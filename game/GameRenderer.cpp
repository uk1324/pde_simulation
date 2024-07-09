#include "GameRenderer.hpp"
#include <game/Shaders/waveData.hpp>
#include <game/Shaders/gridData.hpp>
#include <StructUtils.hpp>
#include <gfx2d/Quad2dPt.hpp>
#include <gfx/ShaderManager.hpp>
#include <gfx/Instancing.hpp>
#include <engine/Math/Color.hpp>

GameRenderer GameRenderer::make() {
	auto gfx = Gfx2d::make();
	auto waveVao = createInstancingVao<WaveShader>(gfx.quad2dPtVbo, gfx.quad2dPtIbo, gfx.instancesVbo);

	auto gridVao = createInstancingVao<GridShader>(gfx.quad2dPtVbo, gfx.quad2dPtIbo, gfx.instancesVbo);

	return GameRenderer{
		MOVE(gfx),
		MOVE(gridVao),
		.gridShader = MAKE_GENERATED_SHADER(GRID),
		MOVE(waveVao),
		.waveShader = MAKE_GENERATED_SHADER(WAVE)
	};
}

const f32 WIDTH = 0.15f;
Vec3 selectedColor = Color3::WHITE;

void GameRenderer::disk(Vec2 center, f32 radius, f32 angle, Vec3 color, bool isSelected) {
	gfx.disk(center, radius, color);
	const auto outlineColor = isSelected ? selectedColor : color / 2.0f;
	gfx.circle(center, radius + 0.01f, WIDTH, outlineColor);
	gfx.line(center, center + Vec2::fromPolar(angle, radius - WIDTH / 2.0f), 0.15, outlineColor);
}

void GameRenderer::drawGrid() {
	gridShader.use();
	GridInstance instance{
		.clipToWorld = gfx.camera.clipSpaceToWorldSpace(),
		.cameraZoom = gfx.camera.zoom,
		.smallCellSize = 1.0f
	};
	drawInstances(gridVao, gfx.instancesVbo, constView(instance), quad2dPtDrawInstances);
}
