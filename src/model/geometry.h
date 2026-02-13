#pragma once
#include <cmath>

namespace model {
    
    using Dimension = int;
    using Coord = Dimension;

    struct Point {
        Coord x, y;
    };

    struct Size {
        Dimension width, height;
    };

    struct Rectangle {
        Point position;
        Size size;
    };

    struct Offset {
        Dimension dx, dy;
    };

    struct Position {
        double x = 0;
        double y = 0;

        friend bool operator==(const Position& lhs, const Position& rhs) {
            constexpr double epsilon = 1.0E-6;
            auto compare = [epsilon](double lhs, double rhs) {
                return std::abs(lhs - rhs) < epsilon;
            };
            return compare(lhs.x, rhs.x) && compare(lhs.y, rhs.y);
        }
    };

    struct Speed {
        double h_speed = 0;
        double v_speed = 0;
    };

    struct VecMove {
        Position start_pos;
        Position end_pos;
    };

    enum class Direction {
        NORTH,
        SOUTH,
        WEST,
        EAST
    };
} //namespace model