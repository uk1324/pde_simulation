#include "ParametricEllipse.hpp"
#include <engine/Math/Constants.hpp>

ParametricEllipse ParametricEllipse::fromFociAndPointOnEllipse(Vec2 focus0, Vec2 focus1, Vec2 pointOnEllipse) {
	const Vec2 center = (focus0 + focus1) / 2.0f;
	const f32 halfSumOfDistances = (focus0.distanceTo(pointOnEllipse) + focus1.distanceTo(pointOnEllipse)) / 2.0f;

	// https://upload.wikimedia.org/wikipedia/commons/thumb/d/d1/Ellipse-reflex.svg/1920px-Ellipse-reflex.svg.png
	const f32 distanceToTopVertex = sqrt(halfSumOfDistances * halfSumOfDistances - focus0.distanceSquaredTo(center));
	const f32 radius0 = distanceToTopVertex;
	const f32 radius1 = halfSumOfDistances;

	const Rotation rotation = Rotation::fromDirection(focus0 - focus1);

	return ParametricEllipse{
		.center = center,
		.rotation = rotation,
		.radius0 = radius0,
		.radius1 = radius1,
	};
}

Vec2 ParametricEllipse::sample(i32 index, i32 samplePoints) const {
	const auto t = f32(index) / f32(samplePoints);
	const auto a = TAU<f32> * t;
	// TODO: Could use the Jacobi elliptic functions, but c++ math.h only implements elliptic integrals which are inversed of elliptic functions.
	auto p = Vec2(radius1 * cos(a), radius0 * sin(a));
	p *= rotation;
	p += center;
	return p;
}
