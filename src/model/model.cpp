#include "model.h"

#include <stdexcept>
#include <random>
#include <unordered_set>
#include "collision_detector_adapters.h"


namespace model {
    using namespace std::literals;

    void Map::AddOffice(Office office) {
        if (warehouse_id_to_index_.contains(office.GetId())) {
            throw std::invalid_argument("Duplicate warehouse");
        }

        const size_t index = offices_.size();
        Office& o = offices_.emplace_back(std::move(office));
        try {
            warehouse_id_to_index_.emplace(o.GetId(), index);
        } catch (...) {
            // Удаляем офис из вектора, если не удалось вставить в unordered_map
            offices_.pop_back();
            throw;
        }
    }

    void Game::AddMap(std::shared_ptr<model::Map> map) {
        const size_t index = maps_.size();
        if (auto [it, inserted] = map_id_to_index_.emplace(map->GetId(), index); !inserted) {
            throw std::invalid_argument("Map with id "s + *map->GetId() + " already exists"s);
        } else {
            try {
                maps_.emplace_back(std::move(map));
            } catch (...) {
                map_id_to_index_.erase(it);
                throw;
            }
        }

        AddSession(maps_.back());
    }

    GameSession& Game::GetSession(const Map::Id& map_id) {
        return session_.at(map_id);
    }

    void Game::AddSession(const std::shared_ptr<Map> map) {
        auto [it, inserted] = session_.try_emplace(map->GetId(), map, loot_gen_, dog_retirement_time_, randomize_spawn_points_);
    }

    double Game::GetDefaultSpeed() const {
        return default_dog_speed_;
    }

    size_t Game::GetDefaultBagCapacity() const {
        return default_bag_capacity_;
    }

    void Game::Tick(int64_t time_delta) {
        for(auto& [map_id, session] : session_) {
            session.Tick(time_delta);
        }
        tick_signal_(time_delta);
    }

    const std::unordered_map<Map::Id, GameSession, Game::MapIdHasher>& Game::GetSessions() const {
        return session_;
    }

    std::unordered_map<Map::Id, GameSession, Game::MapIdHasher>& Game::GetSessions() {
        return session_;
    }

    Dog& GameSession::AddDog(const std::string& name) {
        auto dog_id = Dog::Id{++counter_dog_id_};
        auto[it, inserted] = dogs_.try_emplace(dog_id, dog_id, name, map_->GetDogSpeed(), map_->GetBagCapacity());

        auto start_point = GetRandomStartPos();
        it->second.SetPos(start_point);

        return it->second;
    }

    void GameSession::DeleteDog(const Dog::Id& dog_id) {
        dogs_.erase(dog_id);
    }

    const std::shared_ptr<Map> GameSession::GetMap() const {
        return map_;
    }

    const std::unordered_map<Dog::Id, Dog, GameSession::DogIdHasher>& GameSession::GetDogs() const {
        return dogs_;
    }

    std::unordered_map<Dog::Id, Dog, GameSession::DogIdHasher>& GameSession::GetDogs() {
        return dogs_;
    }

    uint64_t GameSession::GetCounterDogId() const {
        return counter_dog_id_;
    }

    void GameSession::Tick(int64_t time_delta) {
        IncreaseTimeDogs(time_delta);
        GenerateLoot(time_delta);

        std::vector<std::pair<Dog::Id, VecMove>> dogs_pos;

        for(auto& [dog_id, dog] : dogs_) {
            auto dog_vec_move = MoveDog(dog, time_delta);
            dogs_pos.push_back(std::move(dog_vec_move));
        }

        HandleCollisionsItem(dogs_pos);
    }

    std::pair<Dog::Id,VecMove> GameSession::MoveDog(Dog& dog, int64_t time_delta) {
        auto CalculateDeltaDistance = [](int64_t time_delta, double linear_speed) {
            double delta_move = linear_speed * (static_cast<double>(time_delta) / 1000.0);
            return delta_move;
        };

        auto dog_pos = dog.GetPos();
        auto dog_speed = dog.GetSpeed();

        Position new_pos {
            .x = dog_pos.x + CalculateDeltaDistance(time_delta, dog_speed.h_speed),
            .y = dog_pos.y + CalculateDeltaDistance(time_delta, dog_speed.v_speed)
        };

        //Обработать колизии
        auto collisions_pos = HandleCollisionsWall(dog.GetDir(), dog_pos, new_pos);

        dog.SetPos(collisions_pos);
        if(new_pos != collisions_pos) {
            //Остановил dog
            dog.Action(""sv);
        }

        std::pair<Dog::Id, VecMove> dog_vec_move{
            dog.GetId(),
            VecMove{
                .start_pos = dog_pos,
                .end_pos = new_pos
            }
        };

        return dog_vec_move;
    }

    void GameSession::Restore(const std::vector<Dog> dogs, const std::vector<Loot>& loot_in_map, uint64_t next_dog_id) {
        std::unordered_map<Dog::Id, Dog, DogIdHasher> dogs_res;
        for(const auto& dog : dogs) {
            dogs_res.try_emplace(dog.GetId(), dog);
        }

        dogs_ = std::move(dogs_res);
        loot_in_map_ = loot_in_map;
        counter_dog_id_ = next_dog_id;
    }

    Position GameSession::HandleCollisionsWall(Direction dir, Position start, Position end) {
        Position res = start;

        for(const auto& road : map_->GetRoads()) {
            if(!road.PointInArea(start)) {
                continue;
            }

            auto new_pos = road.MoveToPosition(dir, end);
            
            switch(dir) {
            case Direction::NORTH:
                if(new_pos.y < res.y) {
                    res = new_pos;
                }
                break;
            case Direction::SOUTH:
                if(new_pos.y > res.y) {
                    res = new_pos;
                }
                break;
            case Direction::WEST:
                if(new_pos.x < res.x) {
                    res = new_pos;
                }
                break;
            case Direction::EAST:
                if(new_pos.x > res.x) {
                    res = new_pos;
                }
                break;
            }
        }

        return res;
    }

    void GameSession::HandleCollisionsItem(const std::vector<std::pair<Dog::Id, VecMove>>& dogs_pos) {
        auto items_office = collision_detector::OfficesToItems(map_->GetOffices());
        auto items = collision_detector::LootToItems(loot_in_map_);
        auto gatherers = collision_detector::DogsToGatherers(dogs_pos);

        //Запоминаем позицию где закончились объекты лута
        size_t index_base = items.size();
        //Склеиваем два вектора в один
        items.reserve(items.size() + items_office.size());
        items.insert(items.end(), items_office.begin(), items_office.end());

        const auto provider = collision_detector::VectorItemGathererProvider(std::move(items), std::move(gatherers));
        auto events = collision_detector::FindGatherEvents(provider);

        //Контейнер индексов поднятых предметов
        std::unordered_set<size_t> collected_loot; 

        for (const auto& event : events) {
            const auto& [dog_id, dog_vec_move] = dogs_pos[event.gatherer_id];
            auto& dog = dogs_.at(dog_id);

            //Игрок вернул предметы на базу
            if(event.item_id >= index_base) {
                dog.ExchangesLootToPoints();
                continue;
            }

            if(dog.AddLoot(loot_in_map_[event.item_id])) {
                //Игрок поднял предмет
                collected_loot.emplace(event.item_id);
            }            
        }
        
        //Очистка вектора лута от поднятых предметов
        if(!collected_loot.empty()) {
            std::vector<Loot> new_loot_in_map;
            size_t old_size = loot_in_map_.size();
            new_loot_in_map.reserve(old_size);

            for (size_t i = 0; i < old_size; ++i) {
                if(collected_loot.contains(i)) {
                    continue;
                }

                new_loot_in_map.push_back(std::move(loot_in_map_[i]));
            }

            std::swap(loot_in_map_, new_loot_in_map);
        }
    }

    Position GameSession::GetRandomStartPos() const {
        const auto& roads = map_->GetRoads();
        if (roads.empty()) {
            return {0,0};
        }

        Position pos;
        if (randomize_spawn_points_) {
            static std::random_device rd;
            static std::mt19937 gen(rd());

            //Случайный выбор дороги
            std::uniform_int_distribution<size_t> road_dist(0, roads.size() - 1);
            size_t road_index = road_dist(gen);
            const auto& road = roads[road_index];

            //Случайный выбор начала или конца дороги
            std::uniform_int_distribution<int> point_dist(0,1);
            Point start_point = point_dist(gen) == 0 ? road.GetStart() : road.GetEnd();

            pos.x = static_cast<double>(start_point.x);
            pos.y = static_cast<double>(start_point.y);
            
        } else {
            auto start_point = roads[0].GetStart();
            pos.x = static_cast<double>(start_point.x);
            pos.y = static_cast<double>(start_point.y);
        }
        
        return pos;
    }

    void GameSession::GenerateLoot(int64_t time_delta) {
        static std::random_device rd;
        static std::mt19937 gen(rd());

        loot_gen::LootGenerator::TimeInterval delta {time_delta};
        size_t num_loot_types = map_->GetNumLootTypes();
        size_t loot_count = loot_gen_.Generate(delta, loot_in_map_.size(), dogs_.size()); 

        std::uniform_int_distribution<size_t> loot_dist(0, num_loot_types - 1);
        for(size_t i = 0; i < loot_count; ++i) {
            size_t type = loot_dist(gen);
            Loot loot {
                .id = loot_id_counter_++,
                .type = type,
                .pos = GetRandomStartPos(),
                .price = map_->GetPriceLoot(type)
            };

            loot_in_map_.push_back(std::move(loot));
        }
    }

    const std::vector<Loot>& GameSession::GetLootInMap() const {
        return loot_in_map_;
    }

    void GameSession::IncreaseTimeDogs(int64_t time_delta) {
        auto map_id = map_->GetId();
        std::vector<DTO::ExitPlayer> exit_players;
        for(const auto& [dog_id, dog] : dogs_) {
            dog.IncreaseInternalTime(time_delta);
            
            if (dog.IsRetirment(dog_retirement_time_)) {
                //Превышен лимит бездействия
                exit_players.emplace_back(*dog_id, *map_id);
            }
        }

        //Проверка кондидатов на удаление
        if (!exit_players.empty()) {
            exit_signal_(exit_players);
        }
    }

}  // namespace model
