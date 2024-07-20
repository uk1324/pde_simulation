#include "GameRenderer.hpp"
#include <game/Shaders/waveData.hpp>
#include <game/RelativePositions.hpp>
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
		i64 startVertexI = 0;
		for (i64 i = 0; i < boundary.size(); i++) {
			if (boundary[i + 1] == EditorPolygonShape::PATH_END_INDEX) {
				gfx.lineTriangulated(tempVertices[boundary[i]], tempVertices[boundary[startVertexI]], outlineWidth(), outlineColor);
				// boundary index after PATH_END_INDEX
				startVertexI = i + 2;
				if (i == boundary.size() - 2) {
					break;
				}

				continue;
			}
			gfx.lineTriangulated(tempVertices[boundary[i]], tempVertices[boundary[i + 1]], outlineWidth(), outlineColor);
		}
	}
}

void GameRenderer::emitter(Vec2 position, bool isPreview, bool isSelected) {
	Vec3 color = isSelected ? Vec3(0.0f, 1.0f, 1.0f) : Vec3(0.0f, 0.7f, 0.7f);
	gfx.diskTriangulated(position, Constants::EMITTER_DISPLAY_RADIUS, Vec4(color, isPreview ? 0.3f : 0.5f));
}

void GameRenderer::emitter(Vec2 positionRelativeToBody, Vec2 bodyTranslation, f32 bodyRotation, bool isPreview, bool isSelected) {
	Vec2 pos = calculatePositionFromRelativePosition(positionRelativeToBody, bodyTranslation, bodyRotation);;
	emitter(pos, isPreview, isSelected);
}

const Vec3 revoluteJointColor = Color3::RED;

void GameRenderer::revoluteJoint(Vec2 position, bool isPreview) {
	//Vec3 color = isSelected ? Vec3(0.0f, 1.0f, 1.0f) : Vec3(0.0f, 0.7f, 0.7f);
	gfx.diskTriangulated(position, Constants::REVOLUTE_JOINT_DISPLAY_RADIUS, Vec4(revoluteJointColor, isPreview ? 0.5f : 0.8f));
}

void GameRenderer::revoluteJoint(Vec2 relativePosition0, Vec2 pos0, f32 rotation0, Vec2 relativePosition1, Vec2 pos1, f32 rotation1) {
	Vec2 aboslutePos0 = calculatePositionFromRelativePosition(relativePosition0, pos0, rotation0);
	Vec2 aboslutePos1 = calculatePositionFromRelativePosition(relativePosition1, pos1, rotation1);
	revoluteJoint(aboslutePos0, aboslutePos1);
}

void GameRenderer::revoluteJoint(Vec2 absolutePos0, Vec2 absolutePos1) {
	revoluteJoint(absolutePos0, false);
	if (absolutePos0.distanceTo(absolutePos1) > Constants::REVOLUTE_JOINT_DISPLAY_RADIUS) {
		gfx.lineTriangulated(absolutePos0, absolutePos1, 0.2f, revoluteJointColor);
	}
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
		.smallCellSize = gridSmallCellSize
	};
	drawInstances(gridVao, gfx.instancesVbo, constView(instance), quad2dPtDrawInstances);
}
