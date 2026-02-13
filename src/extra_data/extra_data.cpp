#include "extra_data.h"


namespace extra_data {
    void ExtraData::AddLootTypes(const model::Map::Id& map_id, const json::array& loot_types) {
        data_.emplace(map_id, loot_types);
    }

    std::optional<json::array> ExtraData::GetLootTypes(const model::Map::Id& map_id) const {
        if(auto it = data_.find(map_id); it != data_.end()) {
            return it->second;
        }
        
        return std::nullopt;
    }
} // extra_data