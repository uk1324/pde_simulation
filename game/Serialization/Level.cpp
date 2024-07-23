#include <game/Serialization/LevelData.hpp>
#include <Overloaded.hpp>

#define JSON(type) \
	[](const type& value) { return Json::Value{ { "type", type##Name }, { "value", toJson(value) } }; },

#define UNJSON(type) \
	if (json.at("type").string() == type##Name) { return fromJson<type>(json.at("value")); }


static constexpr auto LevelShapeCircleName = "circle";
static constexpr auto LevelShapePolygonName = "polygon";

template<>
LevelShape fromJson<LevelShape>(const Json::Value& json) {
	UNJSON(LevelShapeCircle)
	UNJSON(LevelShapePolygon)
	throw Json::Value::Exception{};
}

Json::Value toJson(const LevelShape& value) {
	return std::visit(overloaded{
		JSON(LevelShapeCircle)
		JSON(LevelShapePolygon)
	}, value);
}

LevelShapePolygon fromJson<LevelShapePolygon>(const Json::Value& json) {
	const auto& boundary = json.at("boundary").array();
	LevelShapePolygon polygon;

	for (const auto& pathJson : boundary) {
		const auto& path = pathJson.array();

		if (path.size() % 2 != 0) {
			throw Json::Value::Exception{};
		}

		std::vector<Vec2> outputPath;
		for (i32 i = 0; i < path.size(); i += 2) {
			const auto x = path[i].number();
			const auto y = path[i + 1].number();
			outputPath.push_back(Vec2(x, y));
		}
		polygon.boundary.push_back(std::move(outputPath));
	}
	
	return polygon;
}

Json::Value toJson(const LevelShapePolygon& value) {
	auto json = Json::Value::emptyObject();
	json["boundary"] = Json::Value::emptyArray();
	auto& boundary = json["boundary"].array();
	for (const auto& path : value.boundary) {
		auto pathJson = Json::Value::emptyArray();
		auto& pathArray = pathJson.array();
		for (const auto vertex : path) {
			pathArray.push_back(vertex.x);
			pathArray.push_back(vertex.y);
		}
		boundary.push_back(std::move(pathJson));
	}
	return json;
}

static constexpr auto LevelMaterialTransimissiveName = "transimissive";
static constexpr auto LevelMaterialReflectiveName = "reflective";

template<>
LevelMaterial fromJson<LevelMaterial>(const Json::Value& json) {
	UNJSON(LevelMaterialTransimissive)
	UNJSON(LevelMaterialReflective)
	throw Json::Value::Exception{};
}

Json::Value toJson(const LevelMaterial& value) {
	return std::visit(overloaded{
		JSON(LevelMaterialTransimissive)
		JSON(LevelMaterialReflective)
	}, value);
}

template<>
LevelBitfield fromJson<LevelBitfield>(const Json::Value& json) {
	const i64 value{ json.intNumber() };
	return LevelBitfield{ .value = u32(value) };
}

Json::Value toJson(const LevelBitfield& value) {
	return Json::Value(i64(value.value));
}

template<>
InputButton fromJson<InputButton>(const Json::Value& json) {
	const auto result = json["keycode"].intNumber();
	return InputButton(KeyCode(result));
}

Json::Value toJson(const InputButton& value) {
	auto json = Json::Value::emptyObject();
	json["keycode"] = Json::Value(Json::Value::IntType(value.keycode));
	return json;
}