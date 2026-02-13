#pragma once

#include <optional>
#include <string>
#include <cstdint>

#include "model.h"
#include "application.h"

namespace state_manager {
    class AutoSaver {
    public:
        AutoSaver(model::Game& game, app::Application& app, const std::string& state_file, int64_t save_period) 
            : game_{game}
            , app_{app}
            , state_file_{state_file}
            , save_period_{save_period}{
        }

        void Tick(int64_t time_delta);

    private:
        model::Game& game_;
        app::Application& app_;
        const std::string state_file_;
        int64_t save_period_;
        int64_t accumulate_time_ = 0;
    };
} // namespace state_manager