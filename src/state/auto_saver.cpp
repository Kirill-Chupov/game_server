#include "auto_saver.h"
#include "state_file_io.h"
#include "logger.h"

namespace state_manager {
    void AutoSaver::Tick(int64_t time_delta) {
        accumulate_time_ += time_delta;
        
        if(accumulate_time_ >= save_period_) {
            SaveState(state_file_, game_, app_);
            accumulate_time_ = 0;
        }
    }
} // namespace state_manager