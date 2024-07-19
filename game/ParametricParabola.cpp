#include "ParametricParabola.hpp"

Vec2 ParametricParabola::sample(i32 index, i32 samplePoints) const {
	const auto t = f32(index) / f32(samplePoints - 1);
	const auto x = (t - 0.5) * 2.0f * xBound;
	const auto y = a * x * x;
	Vec2 p(x, y);
	p *= rotation;
	p += vertex;
	return p;
}
