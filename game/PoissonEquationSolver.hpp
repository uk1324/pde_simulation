#pragma once

#include <engine/Math/Aabb.hpp>
#include "MatrixView.hpp"

struct PoissonEquationSolver {
	void solve(const MatrixView<const f32>& f, MatrixView<f32> u, const Aabb& region);
};