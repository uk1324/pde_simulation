#pragma once

#include <engine/Math/Vec2.hpp>
#include <game/EntityArray.hpp>
#include <List.hpp>

struct EditorCircleShape {
	EditorCircleShape(Vec2 center, f32 radius, f32 angle);

	Vec2 center;
	f32 radius;
	f32 angle;
};

struct EditorPolygonShape {
	struct DefaultInitialize {
		EditorPolygonShape operator()();
	};

	List<Vec2> vertices;
};

using EditorPolygonShapeId = EntityArrayId<EditorPolygonShape>;

enum EditorShapeType {
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
};

struct EditorReflectingBody {
	struct DefaultInitialize {
		EditorReflectingBody operator()();
	};

	EditorShape shape;
};

using EditorReflectingBodyId = EntityArrayId<EditorReflectingBody>;

enum class EditorEntityType {
	REFLECTING_BODY
};

struct EditorEntityId {
	EditorEntityType type;
	u32 index;
	u32 version;

	explicit EditorEntityId(const EditorReflectingBodyId& id);

	EditorReflectingBodyId reflectingBody() const;

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