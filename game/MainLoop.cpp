#include <game/MainLoop.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <game/Shaders/waveData.hpp>
#include <gfx/Instancing.hpp>
#include <gfx2d/Quad2dPt.hpp>

MainLoop::MainLoop() 
	: gfx(Gfx2d::make())
	, waveVao(createInstancingVao<WaveShader>(gfx.quad2dPtVbo, gfx.quad2dPtIbo, gfx.instancesVbo))
	, waveShader(MAKE_GENERATED_SHADER(WAVE)) {
}

void MainLoop::update() {
	ShaderManager::update();
	glViewport(0, 0, Window::size().x, Window::size().y);
	glClear(GL_COLOR_BUFFER_BIT);
	
	waveShader.use();
	std::vector<WaveInstance> instances;
	instances.push_back(WaveInstance{
		.transform = camera.makeTransform(Vec2(0.0f), 0.0f, Vec2(1.0f))
	});
	drawInstances(waveVao, gfx.instancesVbo, instances, quad2dPtDrawInstances);
}
