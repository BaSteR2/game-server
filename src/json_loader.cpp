#include "json_loader.h"

namespace model {

	using namespace json_loader;

	Road tag_invoke(json::value_to_tag<Road>, json::value const& jv) {
		json::object const& obj = jv.as_object();

		Point start{ static_cast<int>(obj.at(JsonConfigNames::ROAD_X0).as_int64()),
					 static_cast<int>(obj.at(JsonConfigNames::ROAD_Y0).as_int64()) };

		if (obj.count(JsonConfigNames::ROAD_X1)) {
			int end_x = static_cast<int>(obj.at(JsonConfigNames::ROAD_X1).as_int64());
			return Road{ Road::HORIZONTAL, start, end_x };
		}
		else {
			int end_y = static_cast<int>(obj.at(JsonConfigNames::ROAD_Y1).as_int64());
			return Road(Road::VERTICAL, start, end_y);
		}
	}

	Building tag_invoke(json::value_to_tag<Building>, json::value const& jv) {
		json::object const& obj = jv.as_object();

		Point position{ static_cast<int>(obj.at(JsonConfigNames::BUILD_X).as_int64()),
						static_cast<int>(obj.at(JsonConfigNames::BUILD_Y).as_int64()) };
		Size size{ static_cast<int>(obj.at(JsonConfigNames::BUILD_W).as_int64()),
				   static_cast<int>(obj.at(JsonConfigNames::BUILD_H).as_int64()) };

		return Building{ Rectangle{ position, size } };
	}

	Office tag_invoke(json::value_to_tag<Office>, json::value const& jv) {
		json::object const& obj = jv.as_object();

		std::string id{ obj.at(JsonConfigNames::OFFICE_ID).as_string() };
		Point position{ static_cast<int>(obj.at(JsonConfigNames::OFFICE_X).as_int64()),
						 static_cast<int>(obj.at(JsonConfigNames::OFFICE_Y).as_int64()) };
		Offset offset{ static_cast<int>(obj.at(JsonConfigNames::OFFICE_OFFX).as_int64()),
						static_cast<int>(obj.at(JsonConfigNames::OFFICE_OFFY).as_int64()) };

		return Office{ Office::Id{id}, position, offset };
	}

	Map tag_invoke(json::value_to_tag<Map>, json::value const& jv) {
		json::object const& obj = jv.as_object();

		Map::Id map_id{ std::string(obj.at(JsonConfigNames::MAP_ID).as_string()) };
		std::string map_name = std::string(obj.at(JsonConfigNames::MAP_NAME).as_string());

		Map map(map_id, map_name);
		if(obj.count(JsonConfigNames::DOG_SPD)){
			map.SetSpeed(obj.at(JsonConfigNames::DOG_SPD).as_double()); 
		}
		if(obj.count(JsonConfigNames::BAG_CAP)){
			map.SetBagCapacity(obj.at(JsonConfigNames::BAG_CAP).as_int64());
		}		
		for(const auto& item : obj.at(JsonConfigNames::LOOT_TYPES).as_array()){
			map.SetScoreForLoot(item.as_object().at(JsonConfigNames::LT_VALUE).as_int64());
		}
		for (const auto& road : obj.at(JsonConfigNames::MAP_ROADS).as_array()) {
			map.AddRoad(json::value_to<Road>(road));
		}
		for (const auto& build : obj.at(JsonConfigNames::MAP_BUILDING).as_array()) {
			map.AddBuilding(json::value_to<Building>(build));
		}
		for (const auto& office : obj.at(JsonConfigNames::MAP_OFFICES).as_array()) {
			map.AddOffice(json::value_to<Office>(office));
		}
		return map;
	}

	void tag_invoke(json::value_from_tag, json::value& jv, Road const& road) {
		if (road.IsHorizontal()) {
			jv = {
				{ JsonConfigNames::ROAD_X0, road.GetStart().x },
				{ JsonConfigNames::ROAD_Y0, road.GetStart().y },
				{ JsonConfigNames::ROAD_X1, road.GetEnd().x }
			};
		}
		else {
			jv = {
				{ JsonConfigNames::ROAD_X0, road.GetStart().x },
				{ JsonConfigNames::ROAD_Y0, road.GetStart().y },
				{ JsonConfigNames::ROAD_Y1, road.GetEnd().y }
			};
		}
	}

	void tag_invoke(json::value_from_tag, json::value& jv, Building const& build) {
		jv = {
			{ JsonConfigNames::BUILD_X, build.GetBounds().position.x },
			{ JsonConfigNames::BUILD_Y, build.GetBounds().position.y },
			{ JsonConfigNames::BUILD_W, build.GetBounds().size.width },
			{ JsonConfigNames::BUILD_H, build.GetBounds().size.height }
		};
	}

	void tag_invoke(json::value_from_tag, json::value& jv, Office const& office) {
		jv = {
			{ JsonConfigNames::OFFICE_ID, *office.GetId() },
			{ JsonConfigNames::OFFICE_X, office.GetPosition().x },
			{ JsonConfigNames::OFFICE_Y, office.GetPosition().y },
			{ JsonConfigNames::OFFICE_OFFX, office.GetOffset().dx },
			{ JsonConfigNames::OFFICE_OFFY, office.GetOffset().dy }
		};
	}

	void tag_invoke(json::value_from_tag, json::value& jv, Map const& map) {
		jv = {
			{ JsonConfigNames::MAP_ID ,  *map.GetId() },
			{ JsonConfigNames::MAP_NAME,  map.GetName() },
			{ JsonConfigNames::MAP_ROADS,   map.GetRoads() },
			{ JsonConfigNames::MAP_BUILDING,  map.GetBuildings() },
			{ JsonConfigNames::MAP_OFFICES,  map.GetOffices() }
		};
	}
} // namespace model

namespace json_loader {

	std::string JsonAsString(const std::filesystem::path& json_path) {
		std::ifstream in(json_path);
		if (!in.is_open()) {
			throw std::ios_base::failure("Json data file not found");
		}
		return { std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
	}

	model::Game LoadGame(const std::filesystem::path& json_path) {
		std::string json_info = JsonAsString(json_path);
		json::object obj = json::parse(json_info).as_object();
		model::Game game;

		double default_speed = obj.count(JsonConfigNames::DEFAULT_DOG_SPD) ? obj.at(JsonConfigNames::DEFAULT_DOG_SPD).as_double() : 1.0;
		int default_bag_capacity = obj.count(JsonConfigNames::DEFAULT_BAG_CAP) ? obj.at(JsonConfigNames::DEFAULT_BAG_CAP).as_int64() : 3;
		for (const auto& map: obj.at(JsonConfigNames::MAPS).as_array()) {
			game.AddMap(json::value_to<model::Map>(map), default_speed, default_bag_capacity);						
		}

		double period = obj.at(JsonConfigNames::LOOT_GEN_CONFIG).as_object().at(JsonConfigNames::LG_PERIOD).as_double();
		double probability = obj.at(JsonConfigNames::LOOT_GEN_CONFIG).as_object().at(JsonConfigNames::LG_PROBABILITY).as_double();
		game.SetLootGenData(period, probability);

		double dog_retirement_time = obj.count("dogRetirementTime") ? obj.at("dogRetirementTime").as_double() : 60.0;	
		game.SetDogRetirementTime(dog_retirement_time);

		return game;
	}

	extra_data::LootJsonData LoadLootData(const std::filesystem::path& json_path) {
		std::string json_info = JsonAsString(json_path);
		json::object obj = json::parse(json_info).as_object();

		extra_data::LootJsonData loot_data;
		for (const auto& map: obj.at(JsonConfigNames::MAPS).as_array()) {
			loot_data.AddItemsData(model::Map::Id{ std::string(map.as_object().at(JsonConfigNames::MAP_ID).as_string()) }, 
								   map.as_object().at(JsonConfigNames::LOOT_TYPES).as_array());
		}
		return loot_data;
	}
}  // namespace json_loader
