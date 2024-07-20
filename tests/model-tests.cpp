#include "../src/model.h"
#include "../src/loot_generator.h"
#include <catch2/catch_test_macros.hpp>

using namespace std::literals;

SCENARIO("Lost objects generation"){
    using TimeInterval = loot_gen::LootGenerator::TimeInterval;
    GIVEN("game session with map(one road) and loot generator"){
        model::LootGenData loot_generator{0.5, 1};
        model::Map test_map{model::Map::Id{"1"}, "test"};
        test_map.AddRoad(model::Road{model::Road::HORIZONTAL, model::Point{0, 0}, 20});       
        model::GameSession test_session(&test_map, loot_generator);
        int type_item_count = 4;

        WHEN("no dogs on map"){
            THEN("no lost objects on map"){
                auto obj = test_session.GetLostObjects();
                CHECK(obj.empty());
            }
            AND_THEN("no new lost objects genarate"){
                test_session.GenerateNewLoot(1s, type_item_count);
                auto& obj = test_session.GetLostObjects();
                CHECK(obj.empty());
            }           
        }
        WHEN("with one dog on map"){
            test_session.AddDog("dog1");
            THEN("only one object appear"){
                test_session.GenerateNewLoot(1s, type_item_count);
                auto& obj = test_session.GetLostObjects();
                CHECK(obj.size() == 1);
            }
        }
        WHEN("generate some dogs on map"){
            for(size_t i = 0; i < 10; i++){
                test_session.AddDog(std::to_string(i));
            }
            THEN("generated lost objects count equal dogs count"){
                auto& obj = test_session.GetLostObjects();
                test_session.GenerateNewLoot(1s, type_item_count);
                CHECK(obj.size() == test_session.GetInfoDogs().size());
            }
            AND_THEN("generated lost objects have unique id "){
                auto& obj = test_session.GetLostObjects();
                test_session.GenerateNewLoot(1s, type_item_count);             
                for(size_t i = 1; i < obj.size(); i++){
                    CHECK(obj[i].id == obj[i - 1].id + 1);
                }
            }
            AND_THEN("generated lost objects have correct loot type"){
                auto& obj = test_session.GetLostObjects();
                test_session.GenerateNewLoot(1s, type_item_count);
                for(size_t i = 1; i < obj.size(); i++){
                    CHECK(((0 <= obj[i].type_loot) && (obj[i].type_loot < type_item_count)));
                } 
            }
        }      
    }   
}