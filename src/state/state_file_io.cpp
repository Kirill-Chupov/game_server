#include "state_file_io.h"
#include <fstream>
#include <iomanip>
#include "logger.h"


namespace state_manager {
    using namespace std::literals;

    void LoadState(const std::filesystem::path& state_file, model::Game& game, app::Application& app) {
        if (!std::filesystem::exists(state_file)) {
            // Файл не существует - это нормально для первого запуска
            logger::Logger::LogInfo("State file doesn't exist, skipping load"s,
                "file"s, state_file.string()
            );
            return;
        }

        std::ifstream ifs {state_file, std::ios::in | std::ios::binary};
        if (!ifs.good()) {
            // Файл существует, но не открывается - это ошибка
            throw std::runtime_error("Can't open file: " + state_file.string());
        }

        boost::archive::text_iarchive iarchive{ifs};

        serialization::GameRepr game_repr;
        serialization::ApplicationRepr app_repr;

        iarchive >> game_repr >> app_repr;

        game_repr.Restore(game);
        app_repr.Restore(game, app);
    }

    void SaveState(const std::filesystem::path& state_file, const model::Game& game, const app::Application& app) {
        auto temp_path = state_file.parent_path() / ("temp-save-file"s + ".tmp"s);

        try {
            if (!state_file.parent_path().empty()) {
                std::filesystem::create_directories(state_file.parent_path());
            }
            
            serialization::GameRepr game_repr(game);
            serialization::ApplicationRepr app_repr(app);

            std::ofstream ofs{temp_path, std::ios::out | std::ios::binary};
            if (!ofs.good()) {
                throw std::runtime_error("Can't open file: " + temp_path.string());
            }

            boost::archive::text_oarchive oarchive{ofs};
            oarchive << game_repr << app_repr;
            ofs.close();

            std::filesystem::rename(temp_path, state_file);
        } catch (const std::exception& ex) {
            std::error_code ec;
            std::filesystem::remove(temp_path, ec);

            if(ec) {
                logger::Logger::LogInfo("error"s,
                    "code"s, ec.value(),
                    "text"s, ec.message()
                );
            }
            
            throw;
        }
    }
    
} // namespace state_manager