#include "SimulationDisplay3d.hpp"
#include <StructUtils.hpp>
#include <engine/Window.hpp>
#include <engine/Math/Constants.hpp>
#include <vector>
#include <gfx/Instancing.hpp>
#include <gfx/ShaderManager.hpp>
#include <game/Shaders/gridDisplayData.hpp>

SimulationDisplay3d SimulationDisplay3d::make(Vbo& instancesVbo) {
	std::vector<Vertex2dP> vertices;
	for (i32 yi = 0; yi < VERTICES_PER_SIDE; yi++) {
		for (i32 xi = 0; xi < VERTICES_PER_SIDE; xi++) {
			const auto xt = f32(xi) / f32(VERTICES_PER_SIDE - 1);
			const auto yt = f32(yi) / f32(VERTICES_PER_SIDE - 1);
			vertices.push_back(Vertex2dP{ .pos = Vec2(xt, yt) });
		}
	}
	auto gridVbo = Vbo(vertices.data(), vertices.size() * sizeof(Vertex2dP));

	auto getVertexIndex = [](i32 xi, i32 yi) -> u32 {
		return yi * VERTICES_PER_SIDE + xi;
	};
	std::vector<u32> indices;
	for (i32 yi = 0; yi < VERTICES_PER_SIDE - 1; yi++) {
		for (i32 xi = 0; xi < VERTICES_PER_SIDE - 1; xi++) {
			indices.push_back(getVertexIndex(xi, yi));
			indices.push_back(getVertexIndex(xi + 1, yi));
			indices.push_back(getVertexIndex(xi + 1, yi + 1));

			indices.push_back(getVertexIndex(xi, yi));
			indices.push_back(getVertexIndex(xi + 1, yi + 1));
			indices.push_back(getVertexIndex(xi, yi + 1));
		}
	}
	auto gridIbo = Ibo(indices.data(), indices.size() * sizeof(u32));
	
	auto gridVao = createInstancingVao<GridDisplayShader>(gridVbo, gridIbo, instancesVbo);

	auto trianglesVbo = Vbo::generate();
	auto trianglesIbo = Ibo::generate();
	auto trianglesVao = createInstancingVao<Triangle3dShader>(trianglesVbo, trianglesIbo, instancesVbo);

	return SimulationDisplay3d{
		.trianglesShader = MAKE_GENERATED_SHADER(TRIANGLE_3D),
		MOVE(trianglesVbo),
		MOVE(trianglesIbo),
		MOVE(trianglesVao),
		.gridShader = MAKE_GENERATED_SHADER(GRID_DISPLAY),
		MOVE(gridVbo),
		MOVE(gridIbo),
		.indexCount = i32(indices.size()),
		MOVE(gridVao),
	};
}

void SimulationDisplay3d::renderGrid(Vbo& instancesVbo, Texture& texture, Vec3 scale) {
	GridDisplayInstance instance{
		.transform = viewProjection() * Mat4(Mat3::scale(scale)),
		.scale = scale
	};

	gridShader.use();
	gridShader.setTexture("heightMap", 0, texture);
	drawInstances(gridVao, instancesVbo, constView(instance), [&](i32 instanceCount) {
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
	});
}

void SimulationDisplay3d::addTriangle(i32 i0, i32 i1, i32 i2) {
	trianglesIndices.push_back(i0);
	trianglesIndices.push_back(i1);
	trianglesIndices.push_back(i2);
}

void SimulationDisplay3d::addQuad(i32 i0, i32 i1, i32 i2, i32 i3) {
	/*
	i3-i2
	| / |
	i0-i1
	*/
	trianglesIndices.push_back(i0);
	trianglesIndices.push_back(i1);
	trianglesIndices.push_back(i2);

	trianglesIndices.push_back(i0);
	trianglesIndices.push_back(i2);
	trianglesIndices.push_back(i3);
}

i32 SimulationDisplay3d::addVertex(Vertex3Pnc vertex) {
	const auto index = trianglesVertices.size();
	trianglesVertices.push_back(vertex);
	return i32(index);
}

void SimulationDisplay3d::renderTriangles() {
	trianglesVbo.allocateData(trianglesVertices.data(), trianglesVertices.size() * sizeof(Vertex3Pnc));
	trianglesIbo.allocateData(trianglesIndices.data(), trianglesIndices.size() * sizeof(u32));

	shaderSetUniforms(trianglesShader, Triangle3dVertUniforms{ 
		.transform = viewProjection() // @Performance
	});
	trianglesShader.use();
	trianglesVao.bind();
	glDrawElements(GL_TRIANGLES, trianglesIndices.size(), GL_UNSIGNED_INT, nullptr);

	trianglesVertices.clear();
	trianglesIndices.clear();
}

Mat4 SimulationDisplay3d::viewProjection() {
	const auto view = camera.viewMatrix();
	const auto aspectRatio = Window::aspectRatio();
	const auto projection = Mat4::perspective(PI<f32> / 2.0f, aspectRatio, 0.1f, 1000.0f);
	return projection * view;
}
