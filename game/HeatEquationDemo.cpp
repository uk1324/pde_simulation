#include "HeatEquationDemo.hpp"
#include <imgui/imgui_internal.h>
#include <imgui/implot.h>
#include <engine/Math/Lerp.hpp>
#include <Span.hpp>

const auto INITIAL_SIZE = 40;
const auto min = 0.0f;
const auto max = 1.0f;

template<typename T>
struct RegularPartition {
	struct Point {
		Point(i64 i, i64 n, T min, T max);

		Point& operator++();
		bool operator==(const Point& other) const;
		Point& operator*();

		i64 i;
		i64 n;
		T min, max;
		T x;
	};

	RegularPartition(i64 n, T min, T max);

	Point begin();
	Point end();

	i64 n;
	T min, max;
};


f32 piecewiseLinearSample(Span<const Vec2> points, f32 x) {
	if (points.size() == 0.0f) {
		return 0.0f;
	}

	if (x < points[0].x) {
		return points[0].y;
	}

	for (i32 i = 0; i < i32(points.size()) - 1; i++) {
		const auto& next = points[i + 1];
		if (x <= next.x) {
			const auto& previous = points[i];
			return lerp(previous.y, next.y, (x - previous.x) / (next.x - previous.x));
		}
	}

	return points.back().y;
}

//struct Cubic {
//	// ax^3 + bx^2 + cx + d
//	f32 a, b, c, d;
//};

//List<Cubic> calculateCubicSplines(Span<const Vec2> points, f32 derivativeAtFirstPoint, f32 derivativeAtLastPoint) {
//	auto output = List<Cubic>::empty();
//
//	const auto n = points.size() - 1;
//
//	auto x = [&](i32 i) -> f32 {
//		return points[i].x;
//	};
//
//	auto h = [&](i32 i) -> f32 {
//		return x(i + 1) - x(i);
//	};
//
//	auto a = [&](i32 i) -> f32 {
//		return points[i].y;
//	};
//
//	auto alpha = [&](i32 i) -> f32 {
//		if (i == 0) {
//			return 3.0f * (a(1) - a(0)) / h(0) - 3.0f * derivativeAtFirstPoint;
//		}
//		if (i == n) {
//			return 3.0f * derivativeAtLastPoint - 3.0f * (a(n) - a(n - 1)) / h(n - 1);
//		}
//		return (3.0f / h(i)) * (a(i + 1) - a(i)) - (3.0f / h(i - 1)) * (a(i) - a(i - 1));
//	};
//
//	auto l = [&](i32 i) -> f32 {
//
//		return 2.0f * (x(i + 1) - x(i - 1)) - h(i - 1)
//	}
//
//	for (i64 i = 1; i < points.size() - 2; i++) {
//		const auto& p = points[i];
//		const auto d = p.y;
//	}
//
//	return output
//}

f32 cubicHermiteInterpolate(f32 startValue, f32 endValue, f32 startDerivative, f32 endDerivative, f32 t) {
	// Could use Horner's method
	const auto t2 = t * t;
	const auto t3 = t2 * t;
	return
		(2.0f * t2 - 3.0f * t3 + 1.0f) * startValue +
		(t3 - 2.0f * t2 + t) * startDerivative +
		(t3 - t2) * endDerivative +
		(-2.0f * t3 + 3.0f * t2) * endValue;
}

f32 cubicHermiteSplineSample(Span<const Vec2> points, f32 x, f32 derivativeAtFirstPoint, f32 derivativeAtLastPoint) {
	if (points.size() == 0.0f) {
		return 0.0f;
	}

	if (x < points[0].x) {
		return points[0].y;
	}

	const auto n = i32(points.size()) - 1;
	for (i32 i = 0; i < n; i++) {
		const auto& next = points[i + 1];
		if (x <= next.x) {
			const auto& previous = points[i];

			f32 previousDerivative;
			if (i == 0) {
				previousDerivative = derivativeAtFirstPoint;
			} else {
				const auto previousPrevious = points[i - 1];
				previousDerivative = (previous.y - previousPrevious.y) / (previous.x - previousPrevious.x);
			}

			f32 nextDerivative;
			if (i == n - 1) {
				nextDerivative = derivativeAtLastPoint;
			} else {
				const auto nextNext = points[i + 2];
				nextDerivative = (nextNext.y - next.y) / (nextNext.x - next.x);
			}

			const auto t = (x - previous.x) / (next.x - previous.x);
			return cubicHermiteInterpolate(previous.y, next.y, previousDerivative, nextDerivative, t);
		}
	}
}

void plotVec2Line(const char* label, const Span<const Vec2>& vs) {
	const auto pointsData = reinterpret_cast<const float*>(vs.data());
	ImPlot::PlotLine(label, pointsData, pointsData + 1, vs.size(), 0, 0, sizeof(float) * 2);
}

void plotVec2Scatter(const char* label, Span<const Vec2> points) {
	const auto pointsData = reinterpret_cast<const float*>(points.data());
	ImPlot::PlotScatter(label, pointsData, pointsData + 1, points.size(), 0, 0, sizeof(Vec2));
}

template<typename T>
Span<const T> listConstSpan(const List<T>& list) {
	return Span<const T>(list.data(), list.size());
}

HeatEquationDemo::HeatEquationDemo() 
	: simulationU(List<f32>::uninitialized(INITIAL_SIZE))
	, controlPoints(List<Vec2>::empty()) {

	for (auto& value : simulationU) {
		value = 0.0f;
	}
	const auto controlPointCount = 7;
	for (i64 i = 0; i < controlPointCount; i++) {
		const auto t = f32(i) / f32(controlPointCount - 1);
		const auto x = lerp(min, max, t);
		controlPoints.add(Vec2(x, 0.0f));
	}
}

void HeatEquationDemo::update() {
	const auto dt = 1.0f / 60.0f;

	auto id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

	const auto initialConditionsWindowName = "initial conditions";
	const auto simulationWindowName = "simulation";
	const auto settingsWindowName = "settings";
	if (firstFrame) {
		ImGui::DockBuilderRemoveNode(id);
		ImGui::DockBuilderAddNode(id);

		const auto leftId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Left, 0.2f, nullptr, &id);
		const auto topId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Up, 0.5f, nullptr, &id);
		const auto bottomId = ImGui::DockBuilderSplitNode(id, ImGuiDir_Down, 0.5f, nullptr, &id);

		ImGui::DockBuilderDockWindow(initialConditionsWindowName, topId);
		ImGui::DockBuilderDockWindow(simulationWindowName, bottomId);
		ImGui::DockBuilderDockWindow(settingsWindowName, leftId);

		ImGui::DockBuilderFinish(id);
		firstFrame = false;
	}

	ImGui::Begin(initialConditionsWindowName);
	
	if (ImPlot::BeginPlot("##p", ImVec2(-1.0f, -1.0f), ImPlotFlags_Equal)) {
		for (i64 i = 1; i < controlPoints.size() - 1; i++) {
			auto& point = controlPoints[i];
			f64 x = point.x;
			f64 y = point.y;
			ImPlot::DragPoint(i, &x, &y, ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 4.0f, ImPlotDragToolFlags_Delayed);
			point.x = std::clamp(f32(x), min, max);
			point.y = y;
		}

		{
			auto points = controlPoints.clone();
			// Not sorting the original controlPoints, because it would mess up the DragPoint ids.
			std::ranges::sort(points, [](Vec2 a, Vec2 b) { return a.x < b.x; });

			auto initialConditionDisplayPoints = List<Vec2>::empty();
			for (i64 i = 0; i < points.size() - 1; i++) {
				initialConditionDisplayPoints.add(points[i]);
				auto& current = points[i];
				auto& next = points[i + 1];
				const auto pointsToPutInBetween = i64(current.distanceTo(next) / 0.005f);
				/*const auto pointsToPutInBetween = i64((abs(next.y - current.y)) / 0.005f);*/
				for (i64 j = 1; j < pointsToPutInBetween; j++) {
					const auto t = f32(j) / f32(pointsToPutInBetween);
					const auto x = lerp(current.x, next.x, t);
					/*initialConditionDisplayPoints.add(Vec2(x, piecewiseLinearSample(listConstSpan(points), x)));*/
					initialConditionDisplayPoints.add(Vec2(x, cubicHermiteSplineSample(listConstSpan(points), x, 0.0f, 0.0f)));
				}
			}
			initialConditionDisplayPoints.add(points[points.size() - 1]);
			/*auto initialConditionDisplayPoints = List<Vec2>::empty();
			for (const auto& point : points) {
				initialConditionDisplayPoints.add(Vec2(points.x, ))
			}*/

			/*for (auto& p : RegularPartition<f32>(initialConditionsDisplayPoints.size(), min, max)) {
				initialConditionsDisplayPoints[p.i] = Vec2(p.x, piecewiseLinearSample(listConstSpan(points), p.x));
			}*/
			plotVec2Line("initial conditions graph", listConstSpan(initialConditionDisplayPoints));
			//plotVec2Scatter("initial conditions graph", listConstSpan(initialConditionDisplayPoints));
		}

		ImPlot::EndPlot();
	}

	ImGui::End();

	ImGui::Begin(simulationWindowName);

	ImGui::End();

	ImGui::Begin(settingsWindowName);

	ImGui::End();
}


bool HeatEquationDemo::firstFrame = true;

template<typename T>
RegularPartition<T>::RegularPartition(i64 n, T min, T max)
	: n(n)
	, min(min)
	, max(max) {}

template<typename T>
RegularPartition<T>::Point RegularPartition<T>::begin() {
	return Point(0, n, min, max);
}

template<typename T>
RegularPartition<T>::Point RegularPartition<T>::end() {
	return Point(n, n, min, max);
}

template<typename T>
RegularPartition<T>::Point::Point(i64 i, i64 n, T min, T max) 
	: i(i)
	, n(n)
	, min(min)
	, max(max)
	, x(min + (max - min) * (f32(i) / f32(n - 1))) {}

template<typename T>
typename RegularPartition<T>::Point& RegularPartition<T>::Point::operator++() {
	i++;
	x = min + (max - min) * (f32(i) / f32(n - 1));
	return *this;
}

template<typename T>
bool RegularPartition<T>::Point::operator==(const Point& other) const {
	return i == other.i;
}

template<typename T>
typename RegularPartition<T>::Point& RegularPartition<T>::Point::operator*() {
	return *this;
}
