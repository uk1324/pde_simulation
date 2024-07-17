#pragma once

#include <engine/Math/Vec2.hpp>
#include <game/EntityArray.hpp>
#include <game/InputButton.hpp>
#include <List.hpp>

struct EditorCircleShape {
	EditorCircleShape(Vec2 center, f32 radius, f32 angle);

	Vec2 center;
	f32 radius;
	f32 angle;

	bool operator==(const EditorCircleShape&) const = default;
};

struct EditorPolygonShape {
	struct DefaultInitialize {
		EditorPolygonShape operator()();
	};

	static EditorPolygonShape make();

	void initializeFromVertices(View<const Vec2> inputVertices);
	void cloneFrom(const EditorPolygonShape& other);

	List<Vec2> vertices;
	List<i32> trianglesVertices;
	static constexpr i32 PATH_END_INDEX = -1;
	List<i32> boundary;
	Vec2 translation;
	f32 rotation;
};

using EditorPolygonShapeId = EntityArrayId<EditorPolygonShape>;

enum class EditorShapeType {
	CIRCLE,
	POLYGON,
};

struct EditorShape {
	union {
		EditorCircleShape circle;
		EditorPolygonShapeId polygon;
	};
	explicit EditorShape(const EditorCircleShape& circle);
	explicit EditorShape(const EditorPolygonShapeId& polygon);

	EditorShapeType type;
	// TODO: Could have the position and rotation inside here instead of being inside the union.

	bool operator==(const EditorShape& other) const;
};

enum class EditorMaterialType {
	RELFECTING,
	TRANSIMISIVE
};

struct EditorMaterialTransimisive {
	bool matchBackgroundSpeedOfTransmission;
	f32 speedOfTransmition;

	bool operator==(const EditorMaterialTransimisive&) const = default;
};

struct EditorMaterial {
	union {
		EditorMaterialTransimisive transimisive;
	};
	EditorMaterialType type;

	EditorMaterial(const EditorMaterialTransimisive& material);
	static EditorMaterial makeReflecting();

	bool operator==(const EditorMaterial& other) const;

private:
	EditorMaterial(EditorMaterialType type);
};

struct EditorRigidBody {
	struct DefaultInitialize {
		EditorRigidBody operator()();
	};
	EditorRigidBody(const EditorShape& shape, const EditorMaterial& material, bool isStatic);

	EditorShape shape;
	EditorMaterial material;
	bool isStatic;


	bool operator==(const EditorRigidBody&) const = default;
};

using EditorRigidBodyId = EntityArrayId<EditorRigidBody>;

const EditorRigidBodyId BACKGROUND_EDITOR_RIGID_BODY_ID = EditorRigidBodyId(0xFFEEDDCC, 0xFFEEDDCC);

struct EditorEmitter {
	struct DefaultInitialize {
		EditorEmitter operator()();
	};

	EditorEmitter(EditorRigidBodyId rigidbody, Vec2 positionRelativeToRigidBody, f32 strength, bool oscillate, f32 period, f32 phaseOffset, std::optional<InputButton>);
	void initialize(EditorRigidBodyId rigidbody, Vec2 positionRelativeToRigidBody, f32 strength, bool oscillate, f32 period, f32 phaseOffset, std::optional<InputButton> button);

	EditorRigidBodyId rigidbody;
	Vec2 positionRelativeToRigidBody;

	f32 strength;

	bool oscillate;
	f32 period;
	f32 phaseOffset;

	std::optional<InputButton> activateOn;
};

using EditorEmitterId = EntityArrayId<EditorEmitter>;

enum class EditorEntityType {
	RIGID_BODY,
	EMITTER,
};

struct EditorEntityId {
	EditorEntityType type;
	u32 index;
	u32 version;

	explicit EditorEntityId(const EditorRigidBodyId& id);
	explicit EditorEntityId(const EditorEmitterId& id);

	EditorRigidBodyId rigidBody() const;
	EditorEmitterId emitter() const;

	bool operator==(const EditorEntityId&) const = default;
};

namespace std {
	template<>
	struct hash<EditorEntityId> {
		using argument_type = typename EditorEntityId;
		using result_type = size_t;

		result_type operator ()(const argument_type& key) const {
			return std::hash<i32>()(key.index) * std::hash<i32>()(key.version);
		}
	};
}