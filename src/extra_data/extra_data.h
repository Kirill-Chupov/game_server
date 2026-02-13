#pragma once
#include <boost/json.hpp>
#include <unordered_map>
#include <optional>

#include "model.h"
#include "tagged.h"


namespace extra_data {
    namespace json = boost::json;
    class ExtraData {
    public:
        void AddLootTypes(const model::Map::Id& map_id, const json::array& loot_types);
        std::optional<json::array> GetLootTypes(const model::Map::Id& map_id) const;
    private:
        using MapIdHasher = util::TaggedHasher<model::Map::Id>;
        std::unordered_map<model::Map::Id, json::array, MapIdHasher> data_;
    };
} // extra_data