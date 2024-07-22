#pragma once

#include <engine/Math/Quat.hpp>
#include <engine/Math/Vec2.hpp>
#include <engine/Math/Mat4.hpp>
#include <optional>

struct Camera3d {
	std::optional<Vec2> lastMousePosition;
	Vec3 position = Vec3(0.0f);
	float angleAroundUpAxis = 0.0f;
	float angleAroundRightAxis = 0.0f;

	// Angle change per pixel.
	float rotationSpeed = 0.1f;
	float movementSpeed = 1.0f;

	void update(float dt);
	Quat cameraForwardRotation() const;
	Mat4 viewMatrix() const;
	Vec3 forward() const;
};