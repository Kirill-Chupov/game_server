#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <boost/signals2.hpp>
#include <atomic>

#include "tagged.h"
#include "dog.h"
#include "geometry.h"
#include "loot_generator.h"
#include "collision_detector.h"
#include "data_transfer_object.h"


namespace model {

class Road {
    struct HorizontalTag {
        explicit HorizontalTag() = default;
    };

    struct VerticalTag {
        explicit VerticalTag() = default;
    };

public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_{start}
        , end_{end_x, start.y} {
        InitialArea();
    }

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{start}
        , end_{start.x, end_y} {
        InitialArea();
    }

    bool IsHorizontal() const noexcept {
        return start_.y == end_.y;
    }

    bool IsVertical() const noexcept {
        return start_.x == end_.x;
    }

    Point GetStart() const noexcept {
        return start_;
    }

    Point GetEnd() const noexcept {
        return end_;
    }

    bool PointInArea(Position point) const {
        auto Equal = [] (double lhs, double rhs) {
            constexpr double epsilon = 0.000001;
            return std::abs(lhs - rhs) < epsilon;
        };
        
        auto LessEqual = [&Equal](double lhs, double rhs) {
            return lhs < rhs || Equal(lhs, rhs);
        };

        auto GreatEqual = [&Equal](double lhs, double rhs) {
            return lhs > rhs || Equal(lhs, rhs);
        };
        
        bool in_range_x = GreatEqual(point.x, left_up.x) && LessEqual(point.x, right_down.x);
        bool in_range_y = GreatEqual(point.y, left_up.y) && LessEqual(point.y, right_down.y);
        
        return in_range_x && in_range_y;
    }

    Position MoveToPosition(Direction dir, Position point_end) const {
        if (PointInArea(point_end)) {
            return point_end;
        }

        switch (dir) {
        case Direction::NORTH:
            point_end.y = left_up.y;
            break;
        case Direction::SOUTH:
            point_end.y = right_down.y;
            break;
        case Direction::WEST:
            point_end.x = left_up.x;
            break;
        case Direction::EAST:
            point_end.x = right_down.x;
            break;
        }

        return point_end;
    }

private:
    Point start_;
    Point end_;
    Position left_up;
    Position right_down;
private:
    void InitialArea() {
        constexpr double HALF_WIDTH = 0.4;
        //Строим прямоугольник площади по двум точкам
        //ось Y увеличивается вниз, ось X увеличивается вправо
        //Устанавливаем минимальную координату прямоугольника
        left_up.x = std::min(start_.x, end_.x) - HALF_WIDTH;
        left_up.y = std::min(start_.y, end_.y) - HALF_WIDTH;
        //Устанавливаем максимальную координату прямоугольника
        right_down.x = std::max(start_.x, end_.x) + HALF_WIDTH;
        right_down.y = std::max(start_.y, end_.y) + HALF_WIDTH;
    }
};

class Building {
public:
    explicit Building(Rectangle bounds) noexcept
        : bounds_{bounds} {
    }

    const Rectangle& GetBounds() const noexcept {
        return bounds_;
    }

private:
    Rectangle bounds_;
};

class Office {
public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept
        : id_{std::move(id)}
        , position_{position}
        , offset_{offset} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    Point GetPosition() const noexcept {
        return position_;
    }

    Offset GetOffset() const noexcept {
        return offset_;
    }

private:
    Id id_;
    Point position_;
    Offset offset_;
};

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name, double dog_speed, size_t bag_capacity, std::vector<int> price_loot) noexcept
        : id_(std::move(id))
        , name_(std::move(name))
        , dog_speed_{dog_speed}
        , bag_capacity_{bag_capacity}
        , price_loot_{price_loot} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    const Buildings& GetBuildings() const noexcept {
        return buildings_;
    }

    const Roads& GetRoads() const noexcept {
        return roads_;
    }

    const Offices& GetOffices() const noexcept {
        return offices_;
    }

    double GetDogSpeed() const noexcept {
        return dog_speed_;
    }

    size_t GetBagCapacity() const noexcept {
        return bag_capacity_;
    }

    size_t GetNumLootTypes() const noexcept {
        return price_loot_.size();
    }

    int GetPriceLoot(size_t type) const {
        return price_loot_.at(type);
    }

    void AddRoad(const Road& road) {
        roads_.emplace_back(road);
    }

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void AddOffice(Office office);

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
    double dog_speed_;
    size_t bag_capacity_;
    const std::vector<int> price_loot_;
};

class GameSession {
    public:
        using ExitSignal = boost::signals2::signal<void(const std::vector<DTO::ExitPlayer>& exit_players)>;
        using DogIdHasher = util::TaggedHasher<Dog::Id>;
        explicit GameSession(const std::shared_ptr<Map> map, loot_gen::LootGenerator loot_gen, int64_t dog_retirement_time,  bool randomize_spawn_points) 
            : map_{map}
            , loot_gen_{loot_gen}
            , randomize_spawn_points_{randomize_spawn_points} 
            , dog_retirement_time_{dog_retirement_time} {
        }

        Dog& AddDog(const std::string& name);
        void DeleteDog(const Dog::Id& dog_id);
        const std::shared_ptr<Map> GetMap() const;
        const std::vector<Loot>& GetLootInMap() const;
        const std::unordered_map<Dog::Id, Dog, DogIdHasher>& GetDogs() const;
        std::unordered_map<Dog::Id, Dog, DogIdHasher>& GetDogs();
        uint64_t GetCounterDogId() const;
        void Tick(int64_t time_delta);
        void Restore(const std::vector<Dog> dogs, const std::vector<Loot>& loot_in_map, uint64_t next_dog_id);
        boost::signals2::connection DoExit(const ExitSignal::slot_type& handler) {
            return exit_signal_.connect(handler);
        }
    private:
        const std::shared_ptr<Map> map_;
        uint64_t counter_dog_id_ = 0;
        std::unordered_map<Dog::Id, Dog, DogIdHasher> dogs_;
        std::vector<Loot> loot_in_map_;
        loot_gen::LootGenerator loot_gen_;
        const int64_t dog_retirement_time_;
        bool randomize_spawn_points_;
        ExitSignal exit_signal_;
        std::atomic<int> loot_id_counter_ = 0;
        
    private:
        std::pair<Dog::Id,VecMove> MoveDog(Dog& dog, int64_t time_delta);
        Position HandleCollisionsWall(Direction dir, Position start, Position end);
        Position GetRandomStartPos() const;
        void GenerateLoot(int64_t time_delta);
        void HandleCollisionsItem(const std::vector<std::pair<Dog::Id, VecMove>>& dogs_pos);
        void IncreaseTimeDogs(int64_t time_delta);
};

class Game {
public:
    using TickSignal = boost::signals2::signal<void(int64_t)>;
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    explicit Game (loot_gen::LootGenerator loot_gen, double defaul_dog_speed, size_t default_bag_capacity, int64_t dog_retirement_time, bool randomize_spawn_points) 
        : loot_gen_{std::move(loot_gen)}
        , default_dog_speed_{defaul_dog_speed}
        , default_bag_capacity_{default_bag_capacity}
        , dog_retirement_time_{dog_retirement_time}
        , randomize_spawn_points_{randomize_spawn_points} {
    }
    using Maps = std::vector<std::shared_ptr<Map>>;

    void AddMap(std::shared_ptr<Map> map);

    const Maps& GetMaps() const noexcept {
        return maps_;
    }

    const std::shared_ptr<Map> FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return maps_.at(it->second);
        }
        return nullptr;
    }

    GameSession& GetSession(const Map::Id& map_id);
    double GetDefaultSpeed() const;
    size_t GetDefaultBagCapacity() const;
    void Tick(int64_t time_delta);
    const std::unordered_map<Map::Id, GameSession, MapIdHasher>& GetSessions() const;
    std::unordered_map<Map::Id, GameSession, MapIdHasher>& GetSessions();

    boost::signals2::connection DoTick(const TickSignal::slot_type& handler) {
        return tick_signal_.connect(handler);
    }

private:
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    Maps maps_;
    MapIdToIndex map_id_to_index_;
    std::unordered_map<Map::Id, GameSession, MapIdHasher> session_;
    loot_gen::LootGenerator loot_gen_;
    double default_dog_speed_;
    size_t default_bag_capacity_;
    const int64_t dog_retirement_time_;
    bool randomize_spawn_points_;
    TickSignal tick_signal_;
private:
    void AddSession(const std::shared_ptr<Map> map);
};

}  // namespace model
