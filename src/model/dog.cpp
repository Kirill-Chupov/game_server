#include "dog.h"
#include <stdexcept>
#include <numeric>

namespace model {

    const Dog::Id& Dog::GetId() const {
        return dog_id_;
    }

    const std::string& Dog::GetName() const {
        return name_;
    }

    Position Dog::GetPos() const {
        return pos_;
    }

    Speed Dog::GetSpeed() const {
        return speed_;
    }

    Direction Dog::GetDir() const {
        return dir_;
    }

    const Dog::Bag& Dog::GetBag() const {
        return bag_;
    }

    bool Dog::IsBagFull() const {
        return bag_.size() == bag_capacity_;
    }

    bool Dog::AddLoot(const Loot& loot) {
        if(IsBagFull()) {
            return false;
        }

        bag_.emplace_back(loot);
        return true;
    }

    int Dog::GetScore() const {
        return score_;
    }

    double Dog::GetMaxSpeed() const {
        return max_speed_;
    }

    size_t Dog::GetBagCapacity() const {
        return bag_capacity_;
    }
    
    int64_t Dog::GetPlayTime() const {
        return play_time_ms_;
    }

    void Dog::Action(std::string_view dir) {
        if (dir.empty()) {
            speed_.v_speed = 0;
            speed_.h_speed = 0;
            return;
        }

        if (dir == "U") {
            dir_ = Direction::NORTH;
            speed_.v_speed = -max_speed_;
            speed_.h_speed = 0;
            return;
        }

        if (dir == "D") {
            dir_ = Direction::SOUTH;
            speed_.v_speed = max_speed_;
            speed_.h_speed = 0;
            return;
        }

        if (dir == "L") {
            dir_ = Direction::WEST;
            speed_.v_speed = 0;
            speed_.h_speed = -max_speed_;
            return;
        }

        if (dir == "R") {
            dir_ = Direction::EAST;
            speed_.v_speed = 0;
            speed_.h_speed = max_speed_;
            return;
        }

        // Сюда программа не дожна попадать. Невалидное значение строки направления
        throw std::invalid_argument("Invalid direction in Dog::Action");
    }

    void Dog::SetPos(Position new_pos) {
        pos_ = new_pos;
    }

    //Обмен предметов на очки. Вызывать при столкновении с базой
    void Dog::ExchangesLootToPoints() {
        auto sum = [](int cur_sum, const Loot& loot) {
            return cur_sum + loot.price;
        };

        score_ += std::accumulate(bag_.begin(), bag_.end(), 0, sum);

        bag_.clear();
    }

    void Dog::IncreaseInternalTime(int64_t delta_time) const {
        if (speed_.v_speed == 0 && speed_.h_speed == 0) {
            idle_time_ += delta_time;
        } else {
            idle_time_ = 0;
        }

        play_time_ms_ += delta_time;
    }

    bool Dog::IsRetirment(const int64_t retirement_time) const {
        return idle_time_ >= retirement_time;
    }
    
    void Dog::Restore(Position pos, Speed speed, Direction dir, const Bag& bag, int score) {
        pos_ = pos;
        speed_ = speed;
        dir_ = dir;
        bag_ = bag;
        score_ = score;
    }
} //namespace model