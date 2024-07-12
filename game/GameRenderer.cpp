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

Vec3 selectedColor = Color3::WHITE;

void GameRenderer::drawBounds(Aabb aabb) {
	gfx.rect(aabb.min, aabb.size(), 0.01f / gfx.camera.zoom, Color3::WHITE);
	gfx.drawLines();
}

void GameRenderer::disk(Vec2 center, f32 radius, f32 angle, Vec4 color, bool isSelected) {
	gfx.diskTriangulated(center, radius, color);
	const auto outlineColor = this->outlineColor(color.xyz(), isSelected);
	gfx.circleTriangulated(center, radius + 0.01f, outlineWidth(), outlineColor);
	gfx.lineTriangulated(center, center + Vec2::fromPolar(angle, radius - outlineWidth() / 2.0f), 0.15, outlineColor);
}

f32 GameRenderer::outlineWidth() const {
	return 0.005f / gfx.camera.zoom;
}

Vec3 GameRenderer::outlineColor(Vec3 mainColor, bool isSelected) const {
	return isSelected ? selectedColor : mainColor / 2.0f;
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
