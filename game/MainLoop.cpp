#include <game/MainLoop.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <game/Shaders/waveData.hpp>
#include <gfx/Instancing.hpp>
#include <gfx2d/Quad2dPt.hpp>
#include "MatrixUtils.hpp"

Vec2 waveSize(500, 500);
f32 cellSize = 0.01f;

MainLoop::MainLoop() 
	: gfx(Gfx2d::make())
	, waveVao(createInstancingVao<WaveShader>(gfx.quad2dPtVbo, gfx.quad2dPtIbo, gfx.instancesVbo))
	, waveShader(MAKE_GENERATED_SHADER(WAVE))
	, waveTexture(Texture::generate())
	, data(waveSize.x, waveSize.y) {

}

void MainLoop::update() {
	ShaderManager::update();
	glViewport(0, 0, Window::size().x, Window::size().y);
	glClear(GL_COLOR_BUFFER_BIT);

	demo.update();

	//waveShader.use();
	//std::vector<WaveInstance> instances;
	//const auto size = Vec2(waveSize * cellSize);
	//instances.push_back(WaveInstance{
	//	.transform = camera.makeTransform(Vec2(0.0f), 0.0f, size / 2.0f)
	//});
	//camera.changeSizeToFitBox(size);
	//waveShader.setTexture("waveTexture", 0, waveTexture);

	//for (i32 yi = 0; yi < waveSize.y; yi++) {
	//	for (i32 xi = 0; xi < waveSize.x; xi++) {
	//		const auto x = xi / (waveSize.x - 1.0f);
	//		const auto y = yi / (waveSize.y - 1.0f);
	//		data(xi, yi) = sin(50.0 * x) * sin(30.0f * y);
	//	}
	//}
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, waveSize.x, waveSize.y, GL_RED, GL_FLOAT, data.data());

	//drawInstances(waveVao, gfx.instancesVbo, instances, quad2dPtDrawInstances);
}
