#pragma once

#include <engine/Math/Vec2.hpp>
#include <game/EntityArray.hpp>
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

	List<Vec2> vertices;
	List<i32> trianglesVertices;
	List<i32> boundaryEdges;
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

	bool operator==(const EditorShape& other) const;
};

struct EditorReflectingBody {
	struct DefaultInitialize {
		EditorReflectingBody operator()();
	};
	EditorReflectingBody(const EditorShape& shape);

	EditorShape shape;

	bool operator==(const EditorReflectingBody&) const = default;
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