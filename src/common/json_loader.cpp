#include "json_loader.h"
#include <boost/json.hpp>
#include <fstream>
#include <vector>

namespace json_loader {
namespace json = boost::json;
namespace detail {
    json::value ParseFile(const std::filesystem::path& json_path) {
        std::ifstream ifs(json_path, std::ios::in);
        if(!ifs.good()){
            throw std::runtime_error("Can't open file: " + json_path.string());
        }
        std::string file_str;
        std::string line;

        while (std::getline(ifs, line)) {
            file_str += line;
        }

        ifs.close();

        return json::parse(file_str);
    }

    model::Road ParseRoad(const json::object& obj_road) {
        bool is_horizontal = obj_road.contains("x1");
        std::string key_end = is_horizontal ? "x1" : "y1";

        model::Point start{
            .x = static_cast<model::Coord>(obj_road.at("x0").as_int64()),
            .y = static_cast<model::Coord>(obj_road.at("y0").as_int64())
        };
        model::Coord end = static_cast<model::Coord>(obj_road.at(key_end).as_int64());
        
        return is_horizontal ? model::Road(model::Road::HORIZONTAL, start, end) : model::Road(model::Road::VERTICAL, start, end);
    }

    model::Building ParseBuilding(const json::object& obj_building) {
        model::Rectangle rect{
            .position {
                .x = static_cast<model::Coord>(obj_building.at("x").as_int64()),
                .y = static_cast<model::Coord>(obj_building.at("y").as_int64())
            },
            .size{
                .width = static_cast<model::Dimension>(obj_building.at("w").as_int64()),
                .height = static_cast<model::Dimension>(obj_building.at("h").as_int64())
            }
        };
        
        return model::Building(rect);
    }

    model::Office ParseOffice(const json::object& obj_office) {
        model::Office::Id office_id {obj_office.at("id").as_string().c_str()};
        model::Point pos{
            .x = static_cast<model::Coord>(obj_office.at("x").as_int64()),
            .y = static_cast<model::Coord>(obj_office.at("y").as_int64())
        };

        model::Offset offset{
            .dx = static_cast<model::Dimension>(obj_office.at("offsetX").as_int64()),
            .dy = static_cast<model::Dimension>(obj_office.at("offsetY").as_int64())
        };

        return model::Office(std::move(office_id), pos, offset);
    }

    std::vector<int> ParseLootTypes(const json::array& loot_types) {
        std::vector<int> price_loot;
        for(const auto& loot : loot_types) {
            int price = loot.as_object().at("value").as_int64();
            price_loot.push_back(price);
        }

        return price_loot;
    }

    std::shared_ptr<model::Map> ParseMap(const json::object& obj_map, const model::Game& game) {
        const auto& id = obj_map.at("id");
        const auto& name = obj_map.at("name");
        const auto& loot_types = obj_map.at("lootTypes").as_array();
        const auto& roads = obj_map.at("roads");
        const auto& buildings = obj_map.at("buildings");
        const auto& offices = obj_map.at("offices");

        model::Map::Id map_id{id.as_string().c_str()};
        std::string map_name {name.as_string().c_str()};
        double dog_speed = game.GetDefaultSpeed();
        if ( auto it = obj_map.find("dogSpeed"); it != obj_map.end() ) {
            dog_speed = it->value().as_double();
        }

        size_t bag_capacity = game.GetDefaultBagCapacity();
        if ( auto it = obj_map.find("bagCapacity"); it != obj_map.end() ) {
            bag_capacity = it->value().as_uint64();
        }

        auto price_loot = ParseLootTypes(loot_types);

        auto map = std::make_shared<model::Map>(map_id, map_name, dog_speed, bag_capacity, std::move(price_loot));

        for(const auto& road : roads.as_array()){
            map->AddRoad(ParseRoad(road.as_object()));
        }

        for(const auto& building : buildings.as_array()){
            map->AddBuilding(ParseBuilding(building.as_object()));
        }
        
        for(const auto& office : offices.as_array()){
            map->AddOffice(ParseOffice(office.as_object()));
        }

        return map;
    }

    std::vector<std::shared_ptr<model::Map>> ParseMaps(const json::object& config, const model::Game& game ) {
        std::vector<std::shared_ptr<model::Map>> maps;
        for (const auto& map : config.at("maps").as_array()){
            maps.push_back(ParseMap(map.as_object(), game));
        }
        return maps;
    }

    loot_gen::LootGenerator ParseLootGenerator(const json::object& config) {
        const auto& loot_generator_config = config.at("lootGeneratorConfig").as_object();
        double period = loot_generator_config.at("period").as_double();
        double probability = loot_generator_config.at("probability").as_double();

        std::chrono::milliseconds base_interval { static_cast<int>((period * 1000))};

        loot_gen::LootGenerator gen(base_interval, probability);

        return gen;
    }
} //namespace detail

model::Game LoadGame(const std::filesystem::path& json_path, bool randomize_spawn_points) {
    // Загрузить содержимое файла json_path, например, в виде строки
    // Распарсить строку как JSON, используя boost::json::parse
    // Загрузить модель игры из файла
    json::value config = detail::ParseFile(json_path);
    const json::object& obj = config.as_object();

    double default_dog_speed = 1;
    if (auto it = obj.find("defaultDogSpeed"); it != obj.end()) {
        default_dog_speed = it->value().as_double();
    }

    size_t default_bag_capacity = 3;
    if (auto it = obj.find("defaultBagCapacity"); it != obj.end()) {
        default_bag_capacity = it->value().as_uint64();
    }

    //Время бездействия по умолчанию в секундах
    double dog_retirement_time_s = 60.0;
    if (auto it = obj.find("dogRetirementTime"); it != obj.end()) {
        dog_retirement_time_s = it->value().as_double();
    }
    //Всё игровое время в миллисекундах
    int64_t dog_retirement_time_ms {static_cast<int64_t>(dog_retirement_time_s * 1000)};

    auto loot_gen = detail::ParseLootGenerator(obj);

    model::Game game(std::move(loot_gen), default_dog_speed, default_bag_capacity, dog_retirement_time_ms, randomize_spawn_points);
    std::vector<std::shared_ptr<model::Map>> maps {detail::ParseMaps(obj, game)};
    
    for (auto& map : maps ) {
        game.AddMap(std::move(map));
    }
    
    return game;
}

extra_data::ExtraData LoadMapExtraData(const std::filesystem::path& json_path) {
    json::value config = detail::ParseFile(json_path);
    const json::object& obj = config.as_object();
    extra_data::ExtraData data;

    for (const auto& map : config.at("maps").as_array()) {
        const auto& obj_map = map.as_object();

        const auto& id = obj_map.at("id").as_string();
        const auto& loot_types = obj_map.at("lootTypes").as_array();

        model::Map::Id map_id {id.c_str()};

        data.AddLootTypes(map_id, loot_types);
    }

    return data;
}

}  // namespace json_loader
