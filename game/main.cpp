#include <engine/Engine.hpp>
#include <engine/EngineUpdateLoop.hpp>
#include <game/MainLoop.hpp>

int main() {
 	Engine::initAll(Window::Settings{
		.maximized = true,
		.multisamplingSamplesPerPixel = 16
	});

	EngineUpdateLoop updateLoop(60.0f);
	MainLoop mainLoop;

	while (updateLoop.isRunning()) {
		mainLoop.update();
	}

	Engine::terminateAll();
}