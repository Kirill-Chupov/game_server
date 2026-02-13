#pragma once
#include "data_transfer_object.h"
#include <vector>

namespace postgres {
    class UseCases {
    public:
        virtual void AddScore(const DTO::Score& score) = 0;
        virtual std::vector<DTO::Score> GetScores(int limit, int offset) const = 0;
    protected:
        ~UseCases() = default;
    };
} // namespace postgres