#pragma once

#include <gfx2d/Gfx2d.hpp>

struct GameRenderer {
	static GameRenderer make();

	void drawBounds(Aabb aabb);
	void disk(Vec2 center, f32 radius, f32 angle, Vec4 color, bool isSelected, bool isStatic);
	void polygon(const List<Vec2>& vertices, const List<i32>& boundaryEdges, const List<i32>& trianglesVertices, Vec2 translation, f32 rotation, Vec4 color, bool isSelected, bool isStatic);

	List<Vec2> tempVertices;
	//void update();
	//void drawDebugDisplay();

	//static constexpr f32 outlineWidth = 0.15f;
	//static constexpr f32 outlineWidth = 0.05f;
	f32 outlineWidth() const;
	Vec3 outlineColor(Vec3 mainColor, bool isSelected) const;
	static constexpr Vec3 defaultColor = Vec3(0.5f);
	static constexpr f32 transimittingShapeTransparency = 0.3f;

	void drawGrid();

	Gfx2d gfx;

	Vao gridVao;
	ShaderProgram& gridShader;

	Vao waveVao;
	ShaderProgram& waveShader;

	Vao waveDisplayVao;
	ShaderProgram& waveDisplayShader;
};