#pragma once
#include <boost/program_options.hpp>
#include <string>
#include <optional>
#include <iostream>


namespace parser_command_line {
    using namespace std::literals;

    struct Args {
        std::string config_file;
        std::string www_root;
        std::optional<int64_t> tick_period;
        bool randomize_spawn_points;
        std::optional<std::string> state_file;
        std::optional<int64_t> save_state_period;
    };

    [[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* const argv[]) {
        namespace po = boost::program_options;

        po::options_description desc{"Allowed options"s};

        Args args;

         // Объявляем временные переменные для опциональных параметров
        int64_t tick_period_val = 0;
        int64_t save_state_period = 0;
        std::string state_file;

        desc.add_options()
            ("help,h", "produce help message")
            ("tick-period,t", po::value<int64_t>(&tick_period_val)->value_name("milliseconds"), "set tick period")
            ("config-file,c", po::value(&args.config_file)->required()->value_name("file"), "set config file path")
            ("www-root,w", po::value(&args.www_root)->required()->value_name("dir"), "set static files root")
            ("randomize-spawn-points", po::bool_switch(&args.randomize_spawn_points), "spawn dogs at random positions")
            ("state-file", po::value(&state_file)->value_name("file"), "set state file path")
            ("save-state-period", po::value(&save_state_period)->value_name("milliseconds"), "set save state period");
        
        po::variables_map vm;
        try{
            po::store(po::parse_command_line(argc, argv, desc), vm);

            if (vm.contains("help"s)) {
                std::cout << desc;
                return std::nullopt;
            }

            po::notify(vm);
        } catch (const std::exception& ex) {
            std::cout << "Error parsing command line: " << ex.what() << std::endl;
            std::cout << desc << std::endl;
            return std::nullopt;
        }
        
        if (vm.contains("tick-period"s)) {
            args.tick_period = tick_period_val;
        }

        if(vm.contains("save-state-period")) {
            args.save_state_period = save_state_period;
        }

        if(vm.contains("state-file")) {
            args.state_file = std::move(state_file);
        }

        return args;
    }
}//parser_command_line