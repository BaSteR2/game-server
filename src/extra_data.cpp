#include "extra_data.h"

namespace extra_data {

    void LootJsonData::AddItemsData(const model::Map::Id& id, const json::array& json_items) {
        map_to_items_.emplace(std::pair{id, json_items});
    }

    const json::array& LootJsonData::GetItemsData(const model::Map::Id& id) const {
        return map_to_items_.at(id);
    }

} //namespace extra_data