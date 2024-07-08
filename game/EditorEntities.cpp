#include <game/EditorEntities.hpp>

EditorEntityId::EditorEntityId(EditorReflectingBodyId& id)
	: version(id.version())
	, index(id.index())
	, type(EditorEntityType::REFLECTING_BODY) {}

EditorReflectingBodyId EditorEntityId::reflectingBody() const {
	return EditorReflectingBodyId(index, version);
}
