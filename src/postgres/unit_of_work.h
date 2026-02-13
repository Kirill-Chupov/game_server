#pragma once
#include <memory>
#include "repository.h"

namespace postgres {
    class UnitOfWork {
    public:
        virtual ~UnitOfWork() = default;

        virtual ScoresRepository& GetScores() = 0;

        virtual void Commit() = 0;
        virtual void RollBack() = 0;
    };

    class UnitOfWorkFactory {
    public:
        virtual ~UnitOfWorkFactory() = default;
        virtual std::unique_ptr<UnitOfWork> CreateUnitOfWork() = 0;
    };
} // namespace postgres