#pragma once

#include <boost/json.hpp>
#include <filesystem>
#include <fstream>
#include "model.h"
#include "extra_data.h"

namespace json = boost::json;

namespace model {
	using namespace std::string_literals;

	Road tag_invoke(json::value_to_tag<Road>, json::value const& jv);
	Building tag_invoke(json::value_to_tag<Building>, json::value const& jv);
	Office tag_invoke(json::value_to_tag<Office>, json::value const& jv);
	Map tag_invoke(json::value_to_tag<Map>, json::value const& jv);

	void tag_invoke(json::value_from_tag, json::value& jv, Road const& road);
	void tag_invoke(json::value_from_tag, json::value& jv, Building const& build);
	void tag_invoke(json::value_from_tag, json::value& jv, Office const& office);
	void tag_invoke(json::value_from_tag, json::value& jv, Map const& map);

} //namespace model


namespace json_loader {
	using namespace std::string_literals;

	struct JsonConfigNames {
		JsonConfigNames() = delete;
		constexpr static const char* MAPS = "maps";
		constexpr static const char* MAP_ID = "id";
		constexpr static const char* MAP_NAME = "name";
		constexpr static const char* MAP_ROADS = "roads";
		constexpr static const char* MAP_BUILDING = "buildings";
		constexpr static const char* MAP_OFFICES = "offices";
		constexpr static const char* ROAD_X0 = "x0";
		constexpr static const char* ROAD_Y0 = "y0";
		constexpr static const char* ROAD_X1 = "x1";
		constexpr static const char* ROAD_Y1 = "y1";
		constexpr static const char* BUILD_X = "x";
		constexpr static const char* BUILD_Y = "y";
		constexpr static const char* BUILD_W = "w";
		constexpr static const char* BUILD_H = "h";
		constexpr static const char* OFFICE_ID = "id";
		constexpr static const char* OFFICE_X = "x";
		constexpr static const char* OFFICE_Y = "y";
		constexpr static const char* OFFICE_OFFX = "offsetX";
		constexpr static const char* OFFICE_OFFY = "offsetY";
		constexpr static const char* LOOT_GEN_CONFIG = "lootGeneratorConfig";
		constexpr static const char* LG_PERIOD = "period";
		constexpr static const char* LG_PROBABILITY = "probability";
		constexpr static const char* LOOT_TYPES = "lootTypes";
		constexpr static const char* LT_VALUE = "value";
		constexpr static const char* DEFAULT_DOG_SPD = "defaultDogSpeed";
		constexpr static const char* DOG_SPD = "dogSpeed";
		constexpr static const char* DEFAULT_BAG_CAP = "defaultBagCapacity";
		constexpr static const char* BAG_CAP = "bagCapacity";
	};

	std::string JsonAsString(const std::filesystem::path& json_path);
	model::Game LoadGame(const std::filesystem::path& json_path);
	extra_data::LootJsonData LoadLootData(const std::filesystem::path& json_path);
	
}  // namespace json_loader
