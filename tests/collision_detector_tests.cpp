#define _USE_MATH_DEFINES

#include <cmath>
#include <functional>
#include <sstream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_range_equals.hpp>

#include "collision_detector.h"

namespace Catch {
template<>
struct StringMaker<collision_detector::GatheringEvent> {
    static std::string convert(collision_detector::GatheringEvent const& value) {
        std::ostringstream tmp;
        tmp << "(gatherer:" << value.gatherer_id
            << ", item:" << value.item_id
            << ", sq_dist:" << value.sq_distance
            << ", time:" << value.time << ")";

        return tmp.str();
    }
};
}  // namespace Catch

namespace {
using Item = collision_detector::Item;
using Gatherer = collision_detector::Gatherer;
using GatheringEvent = collision_detector::GatheringEvent;
using Point = collision_detector::Point;

// Вспомогательная функция
inline Point P(double x, double y) {
     return {x, y};
}

class VectorItemGathererProvider : public collision_detector::ItemGathererProvider {
public:
    VectorItemGathererProvider(std::vector<Item> items, std::vector<Gatherer> gatherers)
        : items_(std::move(items))
        , gatherers_(std::move(gatherers)) {
    }
    
    size_t ItemsCount() const override {
        return items_.size();
    }
    Item GetItem(size_t idx) const override {
        return items_[idx];
    }
    size_t GatherersCount() const override {
        return gatherers_.size();
    }
    Gatherer GetGatherer(size_t idx) const override {
        return gatherers_[idx];
    }

private:
    std::vector<Item> items_;
    std::vector<Gatherer> gatherers_;
};

class CompareEvents {
public:
    bool operator()(const GatheringEvent& lhs, const GatheringEvent& rhs) const {
        static const double eps = 1e-10;

        if (lhs.gatherer_id != rhs.gatherer_id || lhs.item_id != rhs.item_id) {
            return false;
        }  

        if (std::abs(lhs.sq_distance - rhs.sq_distance) > eps) {
            return false;
        }

        if (std::abs(lhs.time - rhs.time) > eps) {
            return false;
        }
        return true;
    }
};

}

SCENARIO("Collision detection") {
    WHEN("no items") {
        std::vector<Item> items {};
        std::vector<Gatherer> gatherers {
            {P(1, 2), P(4, 2), 5.},
            {P(0, 0), P(10, 10), 5.},
            {P(-5, 0), P(10, 5), 5.}
        };
        VectorItemGathererProvider provider{items, gatherers};
        THEN("No events") {
            auto events = collision_detector::FindGatherEvents(provider);
            CHECK(events.empty());
        }
    }
    WHEN("no gatherers") {
        std::vector<Item> items {
            {P(1, 2), 5.},
            {P(0, 0), 5.},
            {P(-5, 0), 5.}
        };
        std::vector<Gatherer> gatherers {};
        VectorItemGathererProvider provider{items, gatherers};
        THEN("No events") {
            auto events = collision_detector::FindGatherEvents(provider);
            CHECK(events.empty());
        }
    }
    WHEN("multiple items on a way of gatherer") {
        std::vector<Item> items {
            {P(9, 0.27), .1},
            {P(8, 0.24), .1},
            {P(7, 0.21), .1},
            {P(6, 0.18), .1},
            {P(5, 0.15), .1},
            {P(4, 0.12), .1},
            {P(3, 0.09), .1},
            {P(2, 0.06), .1},
            {P(1, 0.03), .1},
            {P(0, 0.0), .1},
            {P(-1, 0), .1},
        };
        std::vector<Gatherer> gatherers {
            {P(0, 0), P(10, 0), 0.1}
        };
        VectorItemGathererProvider provider{items, gatherers};
        THEN("Gathered items in right order") {
            auto events = collision_detector::FindGatherEvents(provider);
            CHECK_THAT(
                events,
                Catch::Matchers::RangeEquals(std::vector{
                    GatheringEvent{9, 0,0.*0., 0.0},
                    GatheringEvent{8, 0,0.03*0.03, 0.1},
                    GatheringEvent{7, 0,0.06*0.06, 0.2},
                    GatheringEvent{6, 0,0.09*0.09, 0.3},
                    GatheringEvent{5, 0,0.12*0.12, 0.4},
                    GatheringEvent{4, 0,0.15*0.15, 0.5},
                    GatheringEvent{3, 0,0.18*0.18, 0.6},
                }, CompareEvents()));
        }
    }
    WHEN("multiple gatherers and one item") {
        std::vector<Item> items {
            {P(0, 0), 0.}
        };
        std::vector<Gatherer> gatherers {
            {P(-5, 0), P(5, 0), 1.},
            {P(0, 1), P(0, -1), 1.},
            {P(-10, 10), P(101, -100), 0.5}, // <-- that one
            {P(-100, 100), P(10, -10), 0.5},
        };
        VectorItemGathererProvider provider{items, gatherers};
        THEN("Item gathered by faster gatherer") {
            auto events = collision_detector::FindGatherEvents(provider);
            CHECK(events.front().gatherer_id == 2);
        }
    }
    WHEN("Gatherers stay put") {
        std::vector<Item> items {
            {P(0, 0), 10.}
        };
        std::vector<Gatherer> gatherers {
            {P(-5, 0), P(-5, 0), 1.},
            {P(0, 0), P(0, 0), 1.},
            {P(-10, 10), P(-10, 10), 100}
        };
        VectorItemGathererProvider provider{items, gatherers};
        THEN("No events detected") {
            auto events = collision_detector::FindGatherEvents(provider);

            CHECK(events.empty());
        }
    }
}