#pragma once

#include <gfx2d/Gfx2d.hpp>

struct GameRenderer {
	static GameRenderer make();

	void drawBounds(Aabb aabb);
	void disk(Vec2 center, f32 radius, f32 angle, Vec4 color, Vec3 outlineColor, bool isStatic);
	void polygon(const List<Vec2>& vertices, const List<i32>& boundary, const List<i32>& trianglesVertices, Vec2 translation, f32 rotation, Vec4 color, Vec3 outlineColor, bool isStatic);
	void emitter(Vec2 position, bool isPreview, bool isSelected);
	void emitter(Vec2 positionRelativeToBody, Vec2 bodyTranslation, f32 bodyRotation, bool isPreview, bool isSelected);
	void revoluteJoint(Vec2 position, bool isPreview);
	void revoluteJoint(Vec2 relativePosition0, Vec2 pos0, f32 rotation0, Vec2 relativePosition1, Vec2 pos1, f32 rotation1);
	void revoluteJoint(Vec2 absolutePos0, Vec2 absolutePos1);

	List<Vec2> tempVertices;

	f32 outlineWidth() const;
	Vec3 outlineColor(Vec3 mainColor, bool isSelected) const;
	Vec4 insideColor(Vec4 color, bool isStatic) const;
	static constexpr Vec3 defaultColor = Vec3(0.5f);
	static constexpr f32 transimittingShapeTransparency = 0.3f;

	f32 gridSmallCellSize = 1.0f;

	void drawGrid();

	Gfx2d gfx;

	Vao gridVao;
	ShaderProgram& gridShader;

	Vao waveVao;
	ShaderProgram& waveShader;

	Vao waveDisplayVao;
	ShaderProgram& waveDisplayShader;
};