#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include "tagged.h"
#include "geometry.h"
#include <vector>
 

namespace model {
    
    struct Loot {
        int id = 0;
        size_t type = 0;
        Position pos;
        int price = 0;
    };

    class Dog {
    public:
        using Id = util::Tagged<uint64_t, Dog>;
        using Bag = std::vector<Loot>;

        explicit Dog(Id id, const std::string& name, double max_speed, size_t bag_capacity) 
           : dog_id_{id}
           , name_{name}
           , max_speed_{max_speed}
           , bag_capacity_{bag_capacity} {
        }

        const Id& GetId() const;
        const std::string& GetName() const;
        Position GetPos() const;
        Speed GetSpeed() const;
        Direction GetDir() const;
        const Bag& GetBag() const;
        int GetScore() const;
        double GetMaxSpeed() const;
        size_t GetBagCapacity() const;
        int64_t GetPlayTime() const;        

        void Action(std::string_view dir);
        void SetPos(Position pos);
        bool AddLoot(const Loot& loot);
        bool IsBagFull() const;
        void ExchangesLootToPoints();
        void IncreaseInternalTime(int64_t delta_time) const;
        bool IsRetirment(const int64_t retirement_time) const;
        void Restore(Position pos, Speed speed, Direction dir, const Bag& bag, int score);
    private:
        Id dog_id_;
        std::string name_;
        Position pos_;
        Speed speed_;
        double max_speed_;
        Direction dir_ = Direction::NORTH;
        Bag bag_;
        const size_t bag_capacity_;
        int score_ = 0;
        mutable int64_t idle_time_ = 0;
        mutable int64_t play_time_ms_ = 0;
    };
} //namespace model