#include "GameRenderer.hpp"
#include <game/Shaders/waveData.hpp>
#include <game/EditorEntities.hpp>
#include <game/Constants.hpp>
#include <game/Shaders/gridData.hpp>
#include <game/Shaders/waveDisplayData.hpp>
#include <StructUtils.hpp>
#include <gfx2d/Quad2dPt.hpp>
#include <gfx/ShaderManager.hpp>
#include <gfx/Instancing.hpp>
#include <engine/Math/Rotation.hpp>
#include <engine/Math/Color.hpp>

GameRenderer GameRenderer::make() {
	auto gfx = Gfx2d::make();
	auto waveVao = createInstancingVao<WaveShader>(gfx.quad2dPtVbo, gfx.quad2dPtIbo, gfx.instancesVbo);

	auto gridVao = createInstancingVao<GridShader>(gfx.quad2dPtVbo, gfx.quad2dPtIbo, gfx.instancesVbo);

	auto waveDisplayVao = createInstancingVao<WaveDisplayShader>(gfx.quad2dPtVbo, gfx.quad2dPtIbo, gfx.instancesVbo);

	return GameRenderer{
		.tempVertices = List<Vec2>::empty(),
		MOVE(gfx),
		MOVE(gridVao),
		.gridShader = MAKE_GENERATED_SHADER(GRID),
		MOVE(waveVao),
		.waveShader = MAKE_GENERATED_SHADER(WAVE),
		MOVE(waveDisplayVao),
		.waveDisplayShader = MAKE_GENERATED_SHADER(WAVE_DISPLAY)
	};
}

Vec3 selectedColor = Color3::WHITE;

void GameRenderer::drawBounds(Aabb aabb) {
	gfx.rect(aabb.min, aabb.size(), 0.01f / gfx.camera.zoom, Color3::WHITE);
	gfx.drawLines();
}

void GameRenderer::disk(Vec2 center, f32 radius, f32 angle, Vec4 color, Vec3 outlineColor, bool isStatic) {
	gfx.diskTriangulated(center, radius, insideColor(color, isStatic));
	gfx.circleTriangulated(center, radius, outlineWidth(), outlineColor);
	gfx.lineTriangulated(center, center + Vec2::fromPolar(angle, radius - outlineWidth() / 2.0f), outlineWidth(), outlineColor);
} 

void GameRenderer::polygon(const List<Vec2>& vertices, const List<i32>& boundary, const List<i32>& trianglesVertices, Vec2 translation, f32 rotation, Vec4 color, Vec3 outlineColor, bool isStatic) {
	tempVertices.clear();
	for (const auto& vertex : vertices) {
		tempVertices.add(Rotation(rotation) * vertex + translation);
	}

	gfx.filledTriangles(constView(tempVertices), constView(trianglesVertices), insideColor(color, isStatic));

	if (boundary.size() > 1) {
		i32 startVertex = boundary[0];
		for (i64 i = 0; i < boundary.size(); i++) {
			if (boundary[i + 1] == EditorPolygonShape::PATH_END_INDEX) {
				gfx.lineTriangulated(tempVertices[boundary[i]], tempVertices[startVertex], outlineWidth(), outlineColor);
				startVertex = i + 1;
				if (i == boundary.size() - 2) {
					break;
				}
				continue;
			}
			gfx.lineTriangulated(tempVertices[boundary[i]], tempVertices[boundary[i + 1]], outlineWidth(), outlineColor);
		}
	}
}

void GameRenderer::emitter(Vec2 position, bool isPreview) {
	gfx.diskTriangulated(position, Constants::EMITTER_DISPLAY_RADIUS, Vec4(0.0f, 1.0f, 1.0f, isPreview ? 0.3f : 0.5f));
}

void GameRenderer::emitter(Vec2 positionRelativeToBody, Vec2 bodyTranslation, f32 bodyRotation, bool isPreview) {
	Vec2 pos = positionRelativeToBody;
	pos *= Rotation(bodyRotation);
	pos += bodyTranslation;
	emitter(pos, isPreview);
}

f32 GameRenderer::outlineWidth() const {
	return 0.007f / gfx.camera.zoom;
}

Vec3 GameRenderer::outlineColor(Vec3 mainColor, bool isSelected) const {
	if (isSelected) {
		return selectedColor;
	}

	return mainColor / 2.0f;
}

Vec4 GameRenderer::insideColor(Vec4 color, bool isStatic) const {
	return isStatic ? Vec4(color.xyz() / 1.3f, color.w) : color;
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
