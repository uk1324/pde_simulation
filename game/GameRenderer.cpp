#include "GameRenderer.hpp"
#include <game/Shaders/waveData.hpp>
#include <game/Shaders/gridData.hpp>
#include <StructUtils.hpp>
#include <gfx2d/Quad2dPt.hpp>
#include <gfx/ShaderManager.hpp>
#include <gfx/Instancing.hpp>

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

void GameRenderer::drawGrid() {
	gridShader.use();
	GridInstance instance{
		.clipToWorld = gfx.camera.clipSpaceToWorldSpace(),
		.cameraZoom = gfx.camera.zoom,
		.smallCellSize = 1.0f
	};
	drawInstances(gridVao, gfx.instancesVbo, constView(instance), quad2dPtDrawInstances);
}
