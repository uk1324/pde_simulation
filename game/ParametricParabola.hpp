#pragma once

#include <engine/Math/Rotation.hpp>

struct ParametricParabola {
	//static ParametricEllipse fromFociAndPointOnEllipse(Vec2 focus0, Vec2 focus1, Vec2 pointOnEllipse);
	Vec2 sample(i32 index, i32 samplePoints) const;

	Vec2 vertex;
	f32 a;
	Rotation rotation;
	f32 xBound;
};