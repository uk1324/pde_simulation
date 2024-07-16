#pragma once

#include <gfx2d/Gfx2d.hpp>
#include <gfx2d/Camera.hpp>
#include <engine/Graphics/Vao.hpp>
#include <gfx/ShaderManager.hpp>
#include <game/Demos/PoissonEquationSolver.hpp>
#include <game/Demos/PoissonEquationDemo.hpp>
#include <game/Demos/HeatEquationDemo.hpp>
#include <game/Simulation.hpp>
#include <game/Editor.hpp>

struct MainLoop {
	MainLoop();

	void update();

	GameRenderer renderer;
	Editor editor;
	Simulation simulation;

	enum class State {
		EDITOR,
		SIMULATION,
	};
	State currentState = State::EDITOR;

	std::unordered_map<EditorRigidBodyId, b2BodyId> editorRigidBodyIdToPhysicsId;
};