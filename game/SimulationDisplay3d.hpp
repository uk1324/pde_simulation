#pragma once

#include <engine/Graphics/Texture.hpp>
#include <engine/Graphics/ShaderProgram.hpp>
#include <engine/Graphics/Vao.hpp>
#include <engine/Graphics/Ibo.hpp>
#include <game/Shaders/triangle3dData.hpp>
#include <game/Camera3d.hpp>

struct SimulationDisplay3d {
	static SimulationDisplay3d make(Vbo& instancesVbo);

	void renderGrid(Vbo& instancesVbo, Texture& texture, Vec3 scale);
	void addTriangle(i32 i0, i32 i1, i32 i2);
	void addQuad(i32 i0, i32 i1, i32 i2, i32 i3);
	i32 addVertex(Vertex3Pnc vertex);
	void renderTriangles();

	Mat4 viewProjection();

	std::vector<i32> trianglesIndices;
	std::vector<Vertex3Pnc> trianglesVertices;
	ShaderProgram& trianglesShader;
	Vbo trianglesVbo;
	Ibo trianglesIbo;
	Vao trianglesVao;

	Camera3d camera;

	static constexpr i32 VERTICES_PER_SIDE = 100;

	ShaderProgram& gridShader;
	Vbo gridVbo;
	Ibo gridIbo;
	i32 indexCount;
	Vao gridVao;
};