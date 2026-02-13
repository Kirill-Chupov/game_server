#pragma once
#include <pqxx/pqxx>
#include "repository.h"
#include "unit_of_work.h"
#include <string>


namespace postgres {

    class ScoresRepositoryImpl : public ScoresRepository {
    public:
        explicit ScoresRepositoryImpl(pqxx::transaction_base& transaction);

        void Save(const DTO::Score& score) override;
        std::vector<DTO::Score> GetScores(int limit, int offset) const override;
    private:
        pqxx::transaction_base& transaction_;
    };

    class UnitOfWorkImpl : public UnitOfWork {
    public:
        explicit UnitOfWorkImpl(pqxx::connection& connection);

        ScoresRepository& GetScores() override;

        void Commit() override;
        void RollBack() override;
    private:
        pqxx::work work_;
        ScoresRepositoryImpl scores_;
    };

    class UnitOfWorkFactoryImpl : public UnitOfWorkFactory {
    public:
        explicit UnitOfWorkFactoryImpl(pqxx::connection& connection);

        std::unique_ptr<UnitOfWork> CreateUnitOfWork() override;
    private:
        pqxx::connection& connection_;
    };

    class DataBase {
    public:
        explicit DataBase(pqxx::connection connection);

        UnitOfWorkFactory& GetFactory() &;
    private:
        pqxx::connection connection_;
        UnitOfWorkFactoryImpl factory_uow_;
    private:
        void CreateTabels();
        void PrepareTransaction();
        void PrepareScore();
    };
} // namespace postgres