#include <engine/Engine.hpp>
#include <engine/EngineUpdateLoop.hpp>

int main() {
 	Engine::initAll(Window::Settings{
		.maximized = true
	});

	EngineUpdateLoop updateLoop(60.0f);

	while (updateLoop.isRunning()) {

	}

	Engine::terminateAll();
}