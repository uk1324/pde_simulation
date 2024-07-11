#include <game/EditorEntities.hpp>
#include <engine/Math/ComplexPolygonOutline.hpp>
#include <dependencies/earcut/earcut.hpp>
#include <engine/Math/Triangulate.hpp>

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

EditorReflectingBody::EditorReflectingBody(const EditorShape& shape)
 : shape(shape) {}

EditorPolygonShape EditorPolygonShape::make() {
	return EditorPolygonShape{
		.vertices = List<Vec2>::empty(),
		.trianglesVertices = List<i32>::empty(),
		.boundaryEdges = List<i32>::empty(),
	};
}

void EditorPolygonShape::initializeFromVertices(View<const Vec2> inputVertices) {
	const auto& outline = complexPolygonOutline(inputVertices);
	for (const auto& v : outline) {
		vertices.add(v);
	}

	std::vector<std::vector<Vec2>> inputPolygon;
	std::vector<Vec2> triangulationInputVertices;
	for (const auto& vertex : vertices) {
		triangulationInputVertices.push_back(vertex);
	}
	inputPolygon.emplace_back(std::move(triangulationInputVertices));

	std::vector<u32> result = mapbox::earcut<u32>(inputPolygon);

	for (const auto& vertexIndex : result) {
		trianglesVertices.add(vertexIndex);
	}

	if (vertices.size() > 2) {
		i64 previous = vertices.size() - 1;
		for (i64 i = 0; i < vertices.size(); i++) {
			boundaryEdges.add(i);
			boundaryEdges.add(previous);
			previous = i;
		}
	}
}
