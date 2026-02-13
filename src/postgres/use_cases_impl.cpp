#include "use_cases_impl.h"

namespace postgres {

    UseCasesImpl::UseCasesImpl(UnitOfWorkFactory& factory_uow) 
        :factory_uow_{factory_uow} {
    }
        
    void UseCasesImpl::AddScore(const DTO::Score& score) {
        auto uow = factory_uow_.CreateUnitOfWork();
        auto& scores = uow->GetScores();
        try {
            scores.Save(score);
            uow->Commit();
        } catch (const std::exception&) {
            uow->RollBack();
            throw;
        }
    }

    std::vector<DTO::Score> UseCasesImpl::GetScores(int limit, int offset) const {
        auto uow = factory_uow_.CreateUnitOfWork();
        auto& scores = uow->GetScores();
        return scores.GetScores(limit, offset);
    }
} // namespace postgres