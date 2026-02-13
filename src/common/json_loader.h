#pragma once

#include <filesystem>

#include "model.h"
#include "extra_data.h"
#include "loot_generator.h"

namespace json_loader {

    model::Game LoadGame(const std::filesystem::path& json_path, bool randomize_spawn_points);
    extra_data::ExtraData LoadMapExtraData(const std::filesystem::path& json_path);
    loot_gen::LootGenerator LoadLootGenerator(const std::filesystem::path& json_path);

}  // namespace json_loader
