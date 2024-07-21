#include <game/SimulationSettings.hpp>
#include <Gui.hpp>
#include <game/Shared.hpp>

SimulationSettings SimulationSettings::makeDefault() {
	return SimulationSettings{
		.paused = false,
		.timeScale = 1.0f,
		.rigidbodySimulationSubStepCount = 4,
		.waveEquationSimulationSubStepCount = 1,
		.topBoundaryCondition = SimulationBoundaryCondition::REFLECTING,
		.bottomBoundaryCondition = SimulationBoundaryCondition::REFLECTING,
		.leftBoundaryCondition = SimulationBoundaryCondition::REFLECTING,
		.rightBoundaryCondition = SimulationBoundaryCondition::REFLECTING,
		.dampingPerSecond = 0.90f,
		.speedDampingPerSecond = 0.90f,
	};
}

void simulationSettingsGui(SimulationSettings& settings) {
	auto boundaryConditionCombo = [](const char* text, SimulationBoundaryCondition& value) {
		struct Entry {
			SimulationBoundaryCondition type;
			// Could add tooltip.
		};

		auto boundaryConditionName = [](SimulationBoundaryCondition s) {
			switch (s) {
				using enum SimulationBoundaryCondition;
			case REFLECTING: return "reflecting";
			case ABSORBING: return "absorbing";
			}

			CHECK_NOT_REACHED();
			return "";
		};

		Entry entries[]{
			{ SimulationBoundaryCondition::REFLECTING },
			{ SimulationBoundaryCondition::ABSORBING }
		};
		const char* preview = boundaryConditionName(value);

		Gui::leafNodeBegin(text);
		if (ImGui::BeginCombo(Gui::prependWithHashHash(text), preview)) {
			for (auto& entry : entries) {
				const auto isSelected = entry.type == value;
				if (ImGui::Selectable(boundaryConditionName(entry.type), isSelected)) {
					value = entry.type;
				}

				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}

			}
			ImGui::EndCombo();
		}
	};

	if (gameBeginPropertyEditor("simulation settings")) {
		Gui::inputI32("wave simulation substeps", settings.waveEquationSimulationSubStepCount);
		settings.waveEquationSimulationSubStepCount = std::clamp(settings.waveEquationSimulationSubStepCount, 1, 20);

		Gui::inputI32("rigidbody simulation substeps", settings.rigidbodySimulationSubStepCount);
		settings.rigidbodySimulationSubStepCount = std::clamp(settings.rigidbodySimulationSubStepCount, 1, 20);

		// TODO: undo redo
		Gui::sliderFloat("time scale", settings.timeScale, 0.0f, 1.0f);
		settings.timeScale = std::clamp(settings.timeScale, 0.0f, 1.0f);

		Gui::sliderFloat("damping per second", settings.dampingPerSecond, 0.0f, 1.0f);
		settings.dampingPerSecond = std::clamp(settings.dampingPerSecond, 0.0f, 1.0f);

		Gui::sliderFloat("speed damping per second", settings.speedDampingPerSecond, 0.0f, 1.0f);
		settings.speedDampingPerSecond = std::clamp(settings.speedDampingPerSecond, 0.0f, 1.0f);

		Gui::endPropertyEditor();
	}
	Gui::popPropertyEditor();

	ImGui::SeparatorText("boundary conditions");
	if (gameBeginPropertyEditor("boundary conditions")) {
		boundaryConditionCombo("top", settings.topBoundaryCondition);
		boundaryConditionCombo("bottom", settings.bottomBoundaryCondition);
		boundaryConditionCombo("left", settings.leftBoundaryCondition);
		boundaryConditionCombo("right", settings.rightBoundaryCondition);
		Gui::endPropertyEditor();
	}
	Gui::popPropertyEditor();
	auto setAllTo = [&settings](SimulationBoundaryCondition condition) {
		settings.topBoundaryCondition = condition;
		settings.bottomBoundaryCondition = condition;
		settings.leftBoundaryCondition = condition;
		settings.rightBoundaryCondition = condition;
	};

	if (ImGui::Button("set all to reflecting")) {
		setAllTo(SimulationBoundaryCondition::REFLECTING);
	}
	if (ImGui::Button("set all to absorbing")) {
		setAllTo(SimulationBoundaryCondition::ABSORBING);
	}
}
