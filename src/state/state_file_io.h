#pragma once

#include <filesystem>
#include "state_serialization.h"
#include "model.h"
#include "application.h"


namespace state_manager {

    void LoadState(const std::filesystem::path& state_file, model::Game& game, app::Application& app);
    void SaveState(const std::filesystem::path& state_file, const model::Game& game, const app::Application& app);
    
} // namespace state_manager