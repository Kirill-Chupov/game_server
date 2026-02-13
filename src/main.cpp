#include "sdk.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>

#include "json_loader.h"
#include "request_handler.h"
#include "logging_request_handler.h"
#include "http_server.h"
#include "parser_command_line.h"
#include "ticker.h"
#include "extra_data.h"
#include "loot_generator.h"
#include "state_file_io.h"
#include "auto_saver.h"
#include "postgres.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;

namespace {

    // Запускает функцию fn на n потоках, включая текущий
    template <typename Fn>
    void RunWorkers(unsigned n, const Fn& fn) {
        n = std::max(1u, n);
        std::vector<std::jthread> workers;
        workers.reserve(n - 1);
        // Запускаем n-1 рабочих потоков, выполняющих функцию fn
        while (--n) {
            workers.emplace_back(fn);
        }
        fn();
    }

    std::string GetUrlFromEnv() {
        constexpr const char DB_URL_ENV_NAME[]{"GAME_DB_URL"};
        std::string url_db;
        if (const auto* url = std::getenv(DB_URL_ENV_NAME)) {
            url_db = url;
        } else {
            throw std::runtime_error(DB_URL_ENV_NAME + " environment variable not found"s);
        }
        
        return url_db;
    }

    void SubscribeExitSignals(app::Application& app, model::Game& game) {
        for(auto& [map_id, session] : game.GetSessions()) {
            session.DoExit([&app](const std::vector<DTO::ExitPlayer>& exit_players){
                app.ExitPlayer(exit_players);
            });
        }
    }

}  // namespace

int main(int argc, const char* argv[]) {
    try {
        // 1. Извлекаем параметры из коммандной строки
        auto args = parser_command_line::ParseCommandLine(argc, argv);
        if (!args) {
            return EXIT_SUCCESS;
        }

        logger::Logger::Init();
        std::string url_db = GetUrlFromEnv();
        // 2. Загружаем конфигурацию из файла
        model::Game game = json_loader::LoadGame(args->config_file, args->randomize_spawn_points);
        extra_data::ExtraData data = json_loader::LoadMapExtraData(args->config_file);
        app::Application app(url_db);

        // 3. Восстанавливаем состояния сервера с последнего запуска
        if (args->state_file.has_value()) {
            state_manager::LoadState(*args->state_file, game, app);
        }

        // 4. Инициализируем автосохранение
        std::unique_ptr<state_manager::AutoSaver> auto_saver;
        if(args->state_file.has_value() && args->save_state_period.has_value()) {
            auto_saver = std::make_unique<state_manager::AutoSaver> (
                game,
                app,
                args->state_file.value(),
                args->save_state_period.value()
            );

            //Подписываем автосохраниение на тики игры
            game.DoTick([&auto_saver](int64_t time_delta) {
                auto_saver->Tick(time_delta);
            });
        }

        // 5. Подписываем Application::ExitPlayer на сигналы каждой сессии
        SubscribeExitSignals(app, game);

        // 6. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);
        net::strand<net::io_context::executor_type> api_strand {net::make_strand(ioc)};

        // 7. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number){
            if(!ec) {
                ioc.stop();
            }
        });

        // 8. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        http_handler::RequestHandler handler{game, app, data, args->www_root, api_strand, args->tick_period};
        http_handler::LoggingRequestHandler logging_handler{&handler};

        // 9. Запускаем игровые часы, кроме тестового случая
        std::shared_ptr<ticker::Ticker> game_ticker;
        if (args->tick_period.has_value()) {
            game_ticker = std::make_shared<ticker::Ticker>(
                api_strand,
                std::chrono::milliseconds(args->tick_period.value()),
                [&game](std::chrono::milliseconds delta) {
                    game.Tick(delta.count());
                }
            );
            game_ticker->Start();

            logger::Logger::LogInfo("ticker started"s,
                "tick_period_ms"s, args->tick_period.value()
            );
        }

        // 10. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        const auto address = net::ip::make_address("0.0.0.0"sv);
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, [&logging_handler](auto&& req, std::string client_ip, auto&& send) {
            logging_handler(std::forward<decltype(req)>(req), std::move(client_ip), std::forward<decltype(send)>(send));
        });

        logger::Logger::LogInfo("server started"s,
            "port"s, port,
            "address"s, address.to_string()
        );

        // 11. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });

        // 12. При корректном завершении сохраняем состояние сервера
        if (args->state_file.has_value()) {
            state_manager::SaveState(*args->state_file, game, app);
        }
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        logger::Logger::LogInfo("server exited"s,
            "code"s, EXIT_FAILURE,
            "exception"s, ex.what()
        );
        return EXIT_FAILURE;
    }

    logger::Logger::LogInfo("server exited"s,
        "code"s, EXIT_SUCCESS
    );
    return EXIT_SUCCESS;
}
