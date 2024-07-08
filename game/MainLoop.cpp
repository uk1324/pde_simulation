#include <game/MainLoop.hpp>
#include <glad/glad.h>
#include <engine/Window.hpp>
#include <game/Shaders/waveData.hpp>
#include <gfx/Instancing.hpp>
#include <gfx2d/Quad2dPt.hpp>
#include "MatrixUtils.hpp"

MainLoop::MainLoop() 
	: renderer(GameRenderer::make())
	, editor(Editor::make()) {}

void MainLoop::update() {
	ShaderManager::update();

	//game.update(renderer);
	editor.update(renderer);
}
