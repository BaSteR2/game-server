#pragma once
#include "model.h"
#include "tagged.h"
#include <unordered_map>

#include <boost/json.hpp>


namespace extra_data {

    namespace json = boost::json;

    class LootJsonData {
    public:
        void AddItemsData(const model::Map::Id& id, const json::array& json_items);
        const json::array& GetItemsData(const model::Map::Id& id) const;
    private:
        using MapIdHasher = util::TaggedHasher<model::Map::Id>;
        std::unordered_map<model::Map::Id, json::array, MapIdHasher> map_to_items_;
    };

} //namespace extra_data