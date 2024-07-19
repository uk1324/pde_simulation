#include <game/EditorEntities.hpp>
#include <imgui/imgui.h>
#include <dependencies/earcut/earcut.hpp>
#include <engine/Math/Triangulate.hpp>

EditorEntityId::EditorEntityId(const EditorRigidBodyId& id)
	: version(id.version())
	, index(id.index())
	, type(EditorEntityType::RIGID_BODY) {}

EditorEntityId::EditorEntityId(const EditorEmitterId& id)
	: version(id.version())
	, index(id.index())
	, type(EditorEntityType::EMITTER) {}

EditorRigidBodyId EditorEntityId::rigidBody() const {
	return EditorRigidBodyId(index, version);
}

EditorEmitterId EditorEntityId::emitter() const {
	return EditorEmitterId(index, version);
}

EditorCircleShape::EditorCircleShape(Vec2 center, f32 radius, f32 angle)
	: center(center)
	, radius(radius)
	, angle(angle) {}

EditorRigidBody::EditorRigidBody(const EditorShape& shape, const EditorMaterial& material, bool isStatic)
	: shape(shape) 
	, material(material)
	, isStatic(isStatic) {}

EditorShape::EditorShape(const EditorCircleShape& circle)
	: circle(circle)
	, type(EditorShapeType::CIRCLE) {}

EditorShape::EditorShape(const EditorPolygonShapeId& polygon)
	: polygon(polygon)
	, type(EditorShapeType::POLYGON) {}

bool EditorShape::operator==(const EditorShape& other) const {
	if (other.type != type) {
		return false;
	}

	switch (type) {
		using enum EditorShapeType;
	case CIRCLE: return circle == other.circle;
	case POLYGON: return polygon == other.polygon;
	}

	CHECK_NOT_REACHED();
	return false;
}

EditorPolygonShape EditorPolygonShape::DefaultInitialize::operator()() {
	return EditorPolygonShape::make();
}

EditorRigidBody EditorRigidBody::DefaultInitialize::operator()() {
	return EditorRigidBody(EditorShape(EditorCircleShape(Vec2(0.0f), 0.0f, 0.0f)), EditorMaterial::makeReflecting(), false);
}

EditorEmitter EditorEmitter::DefaultInitialize::operator()() {
	return EditorEmitter(EditorRigidBodyId::invalid(), Vec2(0.0f), 0.0f, false, 0.0f, 0.0f, std::nullopt);
}

EditorPolygonShape EditorPolygonShape::make() {
	return EditorPolygonShape{
		.vertices = List<Vec2>::empty(),
		.trianglesVertices = List<i32>::empty(),
		.boundary = List<i32>::empty(),
		.translation = Vec2(0.0f),
		.rotation = 0.0f
	};
}

void EditorPolygonShape::initializeFromSimplePolygon(View<const Vec2> inputVertices) {
	vertices.clear();
	for (const auto& v : inputVertices) {
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

	for (i64 i = 0; i < vertices.size(); i++) {
		boundary.add(i);
	}
	boundary.add(PATH_END_INDEX);
}

void EditorPolygonShape::cloneFrom(const EditorPolygonShape& other) {
	vertices = other.vertices.clone();
	trianglesVertices = other.trianglesVertices.clone();
	boundary = other.boundary.clone();
	translation = other.translation;
	rotation = other.rotation;
}

EditorMaterial::EditorMaterial(const EditorMaterialTransimisive& material)
	: transimisive(material) 
	, type(EditorMaterialType::TRANSIMISIVE) {}

EditorMaterial EditorMaterial::makeReflecting() {
	return EditorMaterial(EditorMaterialType::RELFECTING);
}

bool EditorMaterial::operator==(const EditorMaterial& other) const {
	if (type != other.type) {
		return false;
	}

	switch (type) {
		using enum EditorMaterialType;

	case RELFECTING:
		return true;
		break;

	case TRANSIMISIVE:
		return transimisive == other.transimisive;
		break;

	}
	CHECK_NOT_REACHED();
	return false;
}

EditorMaterial::EditorMaterial(EditorMaterialType type)	
	: type(type) {}

EditorEmitter::EditorEmitter(std::optional<EditorRigidBodyId> rigidBody, Vec2 position, f32 strength, bool oscillate, f32 period, f32 phaseOffset, std::optional<InputButton> button)
	: rigidBody(rigidBody)
	, position(position)
	, strength(strength)
	, oscillate(oscillate) 
	, period(period) 
	, phaseOffset(phaseOffset)
	, activateOn(button) {}

void EditorEmitter::initialize(std::optional<EditorRigidBodyId> rigidBody, Vec2 position, f32 strength, bool oscillate, f32 period, f32 phaseOffset, std::optional<InputButton> button) {
	this->rigidBody = rigidBody;
	this->position = position;
	this->strength = strength;
	this->oscillate = oscillate;
	this->period = period;
	this->phaseOffset = phaseOffset;
	this->activateOn = button;
}

const char* editorMaterialTypeName(EditorMaterialType material) {
	switch (material) {
		using enum EditorMaterialType;
	case RELFECTING: return "reflecting";
	case TRANSIMISIVE: return "transimisive";
	}
	CHECK_NOT_REACHED();
	return "";
}

bool materialTypeComboGui(EditorMaterialType& selectedType) {
	EditorMaterialType types[]{
		EditorMaterialType::RELFECTING,
		EditorMaterialType::TRANSIMISIVE,
	};
	const char* preview = editorMaterialTypeName(selectedType);

	bool modificationFinished = false;
	if (ImGui::BeginCombo("type", preview)) {
		for (auto& type : types) {
			const auto isSelected = type == selectedType;
			if (ImGui::Selectable(editorMaterialTypeName(type), isSelected)) {
				modificationFinished = selectedType != type;
				selectedType = type;
			}

			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	return modificationFinished;
}
