#include "collision_detector_adapters.h"

namespace collision_detector {

    size_t VectorItemGathererProvider::ItemsCount() const {
        return items_.size();
    }

    Item VectorItemGathererProvider::GetItem(size_t idx) const {
        return items_.at(idx);
    }

    size_t VectorItemGathererProvider::GatherersCount() const {
        return gatherers_.size();
    }

    Gatherer VectorItemGathererProvider::GetGatherer(size_t idx) const {
        return gatherers_.at(idx);
    }

    std::vector<Item> OfficesToItems(const std::vector<model::Office>& offices) {
        std::vector<Item> items;

        for (const auto& office : offices) {
            Item item {
                .position{
                    .x = static_cast<double>(office.GetPosition().x),
                    .y = static_cast<double>(office.GetPosition().y)
                },
                .width = WIDTH_BASE
            };

            items.push_back(std::move(item));
        }

        return  items;
    }

    std::vector<Item> LootToItems(const std::vector<model::Loot>& loots) {
        std::vector<Item> items;

        for (const auto& loot : loots) {
            Item item {
                .position{
                    .x = loot.pos.x,
                    .y = loot.pos.y
                },
                .width = WIDTH_ITEM
            };
            
            items.push_back(std::move(item));
        }

        return  items;
    }

    std::vector<Gatherer> DogsToGatherers(const std::vector<std::pair<model::Dog::Id, model::VecMove>>& dogs_pos){
        std::vector<Gatherer> gatherers;

        for (const auto& [dog_id, dog_pos] : dogs_pos) {
            Gatherer gatherer{
                .start_pos = dog_pos.start_pos,
                .end_pos = dog_pos.end_pos,
                .width = WIDTH_GATHERER
            };

            gatherers.push_back(std::move(gatherer));
        }

        return gatherers;
    }


} // namespace collision_detector