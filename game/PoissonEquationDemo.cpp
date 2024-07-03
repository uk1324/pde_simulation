#include "PoissonEquationDemo.hpp"
#include "imgui/implot.h"
#include "imgui/imgui_internal.h"
#include <engine/Input/Input.hpp>
#include <engine/Math/Aabb.hpp>
#include "PoissonEquationSolver.hpp"

const auto INITIAL_SIZE = Vec2T<i64>(50, 50);

PoissonEquationDemo::PoissonEquationDemo() 
	: inputU(Matrix<f32>::zero(INITIAL_SIZE))
	, outputU(Matrix<f32>::zero(INITIAL_SIZE)) 
	, f(Matrix<f32>::zero(INITIAL_SIZE))
	, size(INITIAL_SIZE) {

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

std::optional<Vec2T<i64>> positionToGridPosition(Vec2 pos, const Aabb& gridBounds, Vec2T<i64> gridSize) {
	pos -= gridBounds.min;
	pos /= gridBounds.size();
	pos *= Vec2(gridSize);
	pos = pos.applied(floor);
	auto gridPos = Vec2T<i64>(pos);
	gridPos.y = gridSize.y - 1 - gridPos.y;
	if (gridPos.x >= 0 && gridPos.y >= 0 && gridPos.x < gridSize.x && gridPos.y < gridSize.y) {
		return gridPos;
	}
	return std::nullopt;
}

void PoissonEquationDemo::update() {
	const auto dt = 1.0f / 60.0f;

	auto id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

	const auto plotWindowName = "plot";
	const auto settingsWindowName = "settings";
	if (firstFrame) {
		ImGui::DockBuilderRemoveNode(id);
		ImGui::DockBuilderAddNode(id);

		const auto leftId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 0.2f, nullptr, &id);
		const auto rightId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Right, 0.5f, nullptr, &id);

		ImGui::DockBuilderDockWindow(plotWindowName, rightId);
		ImGui::DockBuilderDockWindow(settingsWindowName, leftId);

		ImGui::DockBuilderFinish(id);
		firstFrame = false;
	}

	ImGui::Begin(plotWindowName);
	const Aabb inputUGridBounds(Vec2(0.0f), Vec2(1.0f));
	const auto gridBoundsSize = inputUGridBounds.size();
	const auto fGridBounds = inputUGridBounds.translated(Vec2(gridBoundsSize.x, 0.0f));
	const auto outputUGridBounds = fGridBounds.translated(Vec2(gridBoundsSize.x, 0.0f));

	Vec2 cursorPlotPos(0.0f);
	//ImPlot::ShowDemoWindow();

	if (ImPlot::BeginPlot("##p", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		auto heatmap = [this](const Matrix<f32>& m, const Aabb& bounds, f32 min, f32 max) {
			//const char* format = nullptr; // "%.1f"
			const char* format = displayNumbers ? "%.1f" : "";
			ImPlot::PushColormap(ImPlotColormap_Jet);
			ImPlot::PlotHeatmap(
				"",
				m.data(), m.sizeY(), m.sizeX(),
				min, max, format,
				ImVec2(bounds.min), ImVec2(bounds.max));
			ImPlot::PopColormap();

			ImPlot::PushPlotClipRect();
			ImDrawList& DrawList = *ImPlot::GetPlotDrawList();
			DrawList.AddRect(
				ImPlot::PlotToPixels(bounds.min.x, bounds.min.y), 
				ImPlot::PlotToPixels(bounds.max.x, bounds.max.y),
				IM_COL32(0, 255, 0, 255));
			ImPlot::PopPlotClipRect();
		};
		heatmap(inputU, inputUGridBounds, minValue, maxValue);
		heatmap(f, fGridBounds, fInputMin, fInputMax);
		heatmap(outputU, outputUGridBounds, outputMin, outputMax);

		cursorPlotPos = Vec2(ImPlot::GetPlotMousePos().x, ImPlot::GetPlotMousePos().y);
		ImPlot::EndPlot();
	}
	
	Input::ignoreImGuiWantCapture = true;
	const auto inputHeld = Input::isMouseButtonHeld(MouseButton::MIDDLE);
	Input::ignoreImGuiWantCapture = false;
	if (inputHeld) {
		int x = 5;
	}

	auto fillCircle = [](Matrix<f32>& mat, Vec2T<i64> center, i64 radius, f32 value) {
		const auto minX = std::clamp(center.x - radius, 0ll, mat.size().x - 1);
		const auto maxX = std::clamp(center.x + radius, 0ll, mat.size().x - 1);
		const auto minY = std::clamp(center.y - radius, 0ll, mat.size().y - 1);
		const auto maxY = std::clamp(center.y + radius, 0ll, mat.size().y - 1);

		Vec2 centerPos = Vec2(center) + Vec2(0.5f);
		for (i64 y = minY; y <= maxY; y++) {
			for (i64 x = minX; x <= maxX; x++) {
				Vec2 pixelPos = Vec2(x, y) + Vec2(0.5f);
				if (centerPos.distanceSquaredTo(pixelPos) <= radius * radius) {
					mat(x, y) = value;
				}
			}
		}
	};

	{
		auto gridPos = positionToGridPosition(cursorPlotPos, inputUGridBounds, inputU.size());
		if (gridPos.has_value()) {
			const auto onBoundary = gridPos->x == 0 || gridPos->y == 0 || gridPos->x == inputU.sizeX() - 1 || gridPos->y == inputU.sizeY() - 1;
			if (inputHeld && onBoundary) {
				auto& value = inputU(gridPos->x, gridPos->y);
				value = valueToWrite;
				/*value += dt * 2.0f;
				value = std::clamp(value, minValue, maxValue);*/
			}
		}
	}
	{
		auto gridPos = positionToGridPosition(cursorPlotPos, fGridBounds, f.size());
		if (gridPos.has_value() && inputHeld) {
			auto& value = f(gridPos->x, gridPos->y);
			value = valueToWrite;

			fillCircle(f, *gridPos, radius, valueToWrite);
			/*value += dt * 2.0f;
			value = std::clamp(value, minValue, maxValue);*/
		}
	}

	ImGui::End();

	ImGui::Begin(settingsWindowName);
	if (ImGui::Button("solve")) {
		for (i64 x = 0; x < size.x; x++) {
			outputU(x, 0) = inputU(x, 0);
			outputU(x, size.y - 1) = inputU(x, size.y - 1);
		}

		for (i64 y = 0; y < size.y; y++) {
			outputU(0, y) = inputU(0, y);
			outputU(size.y - 1, y) = inputU(size.y - 1, y);
		}

		solvePoissonEquation(matrixViewFromConstMatrix(f), matrixViewFromMatrix(outputU), inputUGridBounds);
	}

	ImGui::InputFloat("min value", &minValue);
	ImGui::InputFloat("max value", &maxValue);
	ImGui::SliderFloat("value to write", &valueToWrite, minValue, maxValue);
	ImGui::InputScalar("radius", ImGuiDataType_S64, &radius);
	ImGui::Checkbox("display numbers", &displayNumbers);

	ImGui::InputFloat("output min", &outputMin);
	ImGui::InputFloat("output max", &outputMax);
	ImGui::Checkbox("default output min max", &defaultOutputMinMax);
	if (defaultOutputMinMax) {
		outputMin = minValue;
		outputMax = maxValue;
	}

	ImGui::InputFloat("input min", &fInputMin);
	ImGui::InputFloat("input max", &fInputMax);
	ImGui::Checkbox("default input min max", &defaultFInputMinMax);
	if (defaultFInputMinMax) {
		fInputMin = minValue;
		fInputMax = maxValue;
	}

	ImGui::End();
}

bool PoissonEquationDemo::firstFrame = true;