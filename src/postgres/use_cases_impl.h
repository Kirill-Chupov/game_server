#pragma once
#include "use_cases.h"
#include "unit_of_work.h"

namespace postgres {
    class UseCasesImpl : public UseCases {
    public:
        explicit UseCasesImpl(UnitOfWorkFactory& factory_uow);
        
        void AddScore(const DTO::Score& score) override;
        std::vector<DTO::Score> GetScores(int limit, int offset) const override;
    private:
        UnitOfWorkFactory& factory_uow_;
    };
} // namespace postgres