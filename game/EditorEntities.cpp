#include <game/EditorEntities.hpp>

EditorEntityId::EditorEntityId(const EditorReflectingBodyId& id)
	: version(id.version())
	, index(id.index())
	, type(EditorEntityType::REFLECTING_BODY) {}

EditorReflectingBodyId EditorEntityId::reflectingBody() const {
	return EditorReflectingBodyId(index, version);
}

EditorCircleShape::EditorCircleShape(Vec2 center, f32 radius, f32 angle)
	: center(center)
	, radius(radius)
	, angle(angle) {}
