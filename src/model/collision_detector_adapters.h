#pragma once

#include "collision_detector.h"
#include "model.h"
#include "dog.h"

#include <vector>


namespace collision_detector {
    constexpr double WIDTH_GATHERER = 0.6;
    constexpr double WIDTH_BASE = 0.5;
    constexpr double WIDTH_ITEM = 0;

    class VectorItemGathererProvider : public ItemGathererProvider {
    public:
        VectorItemGathererProvider(std::vector<Item> items, std::vector<Gatherer> gatherers)
            : items_(std::move(items))
            , gatherers_(std::move(gatherers)) {
        }
        
        size_t ItemsCount() const override;
        Item GetItem(size_t idx) const override;
        size_t GatherersCount() const override;
        Gatherer GetGatherer(size_t idx) const override;

    private:
        std::vector<Item> items_;
        std::vector<Gatherer> gatherers_;
    };

    std::vector<Item> OfficesToItems(const std::vector<model::Office>& offices);
    std::vector<Item> LootToItems(const std::vector<model::Loot>& loots);
    std::vector<Gatherer> DogsToGatherers(const std::vector<std::pair<model::Dog::Id, model::VecMove>>& dogs_pos);

} // namespace collision_detector