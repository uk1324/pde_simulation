#include "PoissonEquationDemo.hpp"
#include "imgui/implot.h"
#include "imgui/imgui_internal.h"
#include <engine/Input/Input.hpp>
#include <engine/Math/Aabb.hpp>
#include "MatrixUtils.hpp"
#include "View2dUtils.hpp"
#include "PoissonEquationSolver.hpp"
#include "GridUtils.hpp"

const auto INITIAL_SIZE = Vec2T<i64>(40, 40);

PoissonEquationDemo::PoissonEquationDemo() 
	: inputU(Array2d<f32>::filled(INITIAL_SIZE, 0.0f))
	, outputU(Array2d<f32>::filled(INITIAL_SIZE, 0.0f))
	, f(Array2d<f32>::filled(INITIAL_SIZE, 0.0f))
	, size(INITIAL_SIZE) {

	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

/*
// https://en.wikipedia.org/wiki/Poisson%27s_equation

Dxx u + Dyy u = f

If u is interpreted as the gravitational potential (the function whose gradient is the gravitational field) then from Gauss's law f = -mass density * some constants. Then finding a solution to Poisson's equation can be interpreted as finding the gravitational potential u gives the distribution of density -f.
Similarly in electrostatics -f is the charge density times some constant and u is the electrostatic potential.

Poisson's equation is also used in surface reconstruction from a point cloud.
Not sure if this is the idea, but if you had a bunch of points the the potential field generated by points charges at those points could be used to approximate the shape.
// https://youtu.be/FjnHE8dWc9w?si=0onjJohpiPWWWQku&t=331
*/

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
	const auto fGridBounds = inputUGridBounds.translated(Vec2(gridBoundsSize.x + 0.05, 0.0f));
	const auto outputUGridBounds = fGridBounds.translated(Vec2(gridBoundsSize.x + 0.05, 0.0f));

	Vec2 cursorPlotPos(0.0f);
	//ImPlot::ShowDemoWindow();
	if (ImPlot::BeginPlot("##p", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		auto heatmap = [this](const char* text, const Array2d<f32>& m, const Aabb& bounds, f32 min, f32 max) {
			ImPlot::PlotText(text, bounds.center().x, bounds.max.y + 0.05f);
			//const char* format = nullptr; // "%.1f"
			const char* format = displayNumbers ? "%.1f" : "";
			ImPlot::PushColormap(ImPlotColormap_Jet);
			ImPlot::PlotHeatmap(
				text,
				m.data(), m.sizeY(), m.sizeX(),
				min, max, format,
				ImVec2(bounds.min), ImVec2(bounds.max));
			ImPlot::PopColormap();

			ImPlot::PushPlotClipRect();
			ImDrawList& DrawList = *ImPlot::GetPlotDrawList();
			DrawList.AddRect(
				ImPlot::PlotToPixels(bounds.min.x, bounds.min.y), 
				ImPlot::PlotToPixels(bounds.max.x, bounds.max.y),
				IM_COL32(255, 255, 255, 255));
			ImPlot::PopPlotClipRect();
		};
		heatmap("boundary conditions", inputU, inputUGridBounds, uInputMin, uInputMax);
		heatmap("f", f, fGridBounds, fInputMin, fInputMax);
		heatmap("output", outputU, outputUGridBounds, calculatedOutputMin, calculatedOutputMax);

		cursorPlotPos = Vec2(ImPlot::GetPlotMousePos().x, ImPlot::GetPlotMousePos().y);
		ImPlot::EndPlot();
	}
	
	Input::ignoreImGuiWantCapture = true;
	const auto inputHeld = Input::isMouseButtonHeld(MouseButton::MIDDLE);
	Input::ignoreImGuiWantCapture = false;
	if (inputHeld) {
		int x = 5;
	}

	auto fillCircle = [](Array2d<f32>& mat, Vec2T<i64> center, i64 radius, f32 value) {
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
				value = uInputValueToWrite;
				/*value += dt * 2.0f;
				value = std::clamp(value, minValue, maxValue);*/
			}
		}
	}
	{
		auto gridPos = positionToGridPosition(cursorPlotPos, fGridBounds, f.size());
		if (gridPos.has_value() && inputHeld) {
			/*auto& value = f(gridPos->x, gridPos->y);
			value = fValueToWrite;*/
			fillCircle(f, *gridPos, radius, fValueToWrite);
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

		solvePoissonEquation(constView2d(f), view2d(outputU), inputUGridBounds);
		calculatedOutputMax = max(outputU);
		calculatedOutputMin = min(outputU);
	}
	ImGui::Checkbox("display numbers", &displayNumbers);

	//if (ImGui::Button("open help")) {
	//	ImGui::OpenPopup("help");
	//}

	//if (ImGui::BeginPopupModal("help")) {
	//	ImGui::Text("use middle mouse button to draw");
	//	ImGui::EndPopup();
	//}
	

	{
		ImGui::SeparatorText("boundary conditions");
		ImGui::PushID(&inputU);
		ImGui::InputFloat("input min", &uInputMin);
		ImGui::InputFloat("input max", &uInputMax);
		ImGui::SliderFloat("input value", &uInputValueToWrite, uInputMin, uInputMax);
		if (ImGui::Button("clear")) {
			fill(inputU, 0.0f);
		}
		ImGui::PopID();
	}

	{
		ImGui::SeparatorText("f");
		ImGui::PushID(&f);
		ImGui::InputFloat("input min", &fInputMin);
		ImGui::InputFloat("input max", &fInputMax);
		ImGui::InputScalar("brush radius", ImGuiDataType_S64, &radius);
		ImGui::SliderFloat("f input value to write", &fValueToWrite, fInputMin, fInputMax);
		if (ImGui::Button("clear")) {
			fill(f, 0.0f);
		}
		ImGui::PopID();
		
	}

	ImGui::End();
}

bool PoissonEquationDemo::firstFrame = true;