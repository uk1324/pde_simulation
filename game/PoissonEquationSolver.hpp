#pragma once

#include <engine/Math/Aabb.hpp>
#include "View2d.hpp"


void solvePoissonEquation(const View2d<const f32>& f, View2d<f32> u, const Aabb& region);