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

	void initializeFromSimplePolygon(View<const Vec2> inputVertices);
	void cloneFrom(const EditorPolygonShape& other);

	// Could store the boundary directly, because the triangulation doesn't add any new vertices.
	// The index list is just repeating the vertex list.
	// Technically without it there would need to be some other separator of the paths. Which could cause issues with serialization.
	List<Vec2> vertices;
	List<i32> trianglesVertices;

	static constexpr i32 PATH_END_INDEX = -1;
	// Paths are separted using PATH_END_INDEX
	// The first path of vertices is the main shape the later ones are holes.
	// The first path is counter clockwise the other ones are clockwise.
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

const char* editorMaterialTypeName(EditorMaterialType material);
bool materialTypeComboGui(EditorMaterialType& selectedType);

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
	EditorRigidBody(const EditorShape& shape, const EditorMaterial& material, bool isStatic, u32 collisionCategories, u32 collisionMask);

	EditorShape shape;
	EditorMaterial material;
	bool isStatic;
	u32 collisionCategories;
	u32 collisionMask;

	bool operator==(const EditorRigidBody&) const = default;
};

using EditorRigidBodyId = EntityArrayId<EditorRigidBody>;

struct EditorEmitter {
	struct DefaultInitialize {
		EditorEmitter operator()();
	};

	EditorEmitter(std::optional<EditorRigidBodyId> rigidBody, Vec2 position, f32 strength, bool oscillate, f32 period, f32 phaseOffset, std::optional<InputButton>);
	void initialize(std::optional<EditorRigidBodyId> rigidBody, Vec2 position, f32 strength, bool oscillate, f32 period, f32 phaseOffset, std::optional<InputButton> button);

	std::optional<EditorRigidBodyId> rigidBody;
	// If rigidBody != nullopt then this position is relative.
	Vec2 position;

	f32 strength;

	bool oscillate;
	f32 period;
	f32 phaseOffset;

	std::optional<InputButton> activateOn;

	bool operator==(const EditorEmitter&) const = default;
};

using EditorEmitterId = EntityArrayId<EditorEmitter>;

struct EditorRevoluteJoint {
	struct DefaultInitialize {
		EditorRevoluteJoint operator()();
	};

	std::optional<EditorRigidBodyId> body0;
	Vec2 position0;
	EditorRigidBodyId body1;
	Vec2 position1;

	f32 motorSpeed;
	f32 motorMaxTorque;
	bool motorAlwaysEnabled;
	std::optional<InputButton> clockwiseKey;
	std::optional<InputButton> counterclockwiseKey;

	EditorRevoluteJoint(std::optional<EditorRigidBodyId> body0, Vec2 position0, EditorRigidBodyId body1, Vec2 position1, f32 motorSpeed, f32 motorMaxTorque, bool motorAlwaysEnabled, std::optional<InputButton> clockwiseKey, std::optional<InputButton> counterclockwiseKey);
	void intialize(std::optional<EditorRigidBodyId> body0, Vec2 position0, EditorRigidBodyId body1, Vec2 position1, f32 motorSpeed, f32 motorMaxTorque, bool motorAlwaysEnabled, std::optional<InputButton> clockwiseKey, std::optional<InputButton> counterclockwiseKey);

	bool operator==(const EditorRevoluteJoint&) const = default;
};

using EditorRevoluteJointId = EntityArrayId<EditorRevoluteJoint>;

enum class EditorEntityType {
	RIGID_BODY,
	EMITTER,
	REVOLUTE_JOINT,
};

struct EditorEntityId {
	EditorEntityType type;
	u32 index;
	u32 version;

	explicit EditorEntityId(const EditorRigidBodyId& id);
	explicit EditorEntityId(const EditorEmitterId& id);
	explicit EditorEntityId(const EditorRevoluteJointId& id);

	EditorRigidBodyId rigidBody() const;
	EditorEmitterId emitter() const;
	EditorRevoluteJointId revoluteJoint() const;

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