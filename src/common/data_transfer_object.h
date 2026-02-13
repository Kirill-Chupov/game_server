#pragma once
#include <string>
#include <cstdint>


namespace DTO {
    struct Score {
        std::string name;
        int64_t score = 0;
        int64_t play_time_ms = 0;
    };

    struct ExitPlayer {
        int64_t dog_id;
        std::string map_id;
    };
} // namespace DTO