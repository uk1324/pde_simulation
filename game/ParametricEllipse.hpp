#include <engine/Math/Rotation.hpp>

struct ParametricEllipse {
	static ParametricEllipse fromFociAndPointOnEllipse(Vec2 focus0, Vec2 focus1, Vec2 pointOnEllipse);

	Vec2 sample(i32 index, i32 samplePoints) const;

	Vec2 center;
	Rotation rotation;
	f32 radius0;
	f32 radius1;
};