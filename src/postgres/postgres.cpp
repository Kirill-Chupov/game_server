#include "postgres.h"


namespace postgres {
    using namespace std::literals;
    using pqxx::operator"" _zv;

    namespace detail {
        DTO::Score ScoreFromRow(const pqxx::row& row, size_t start_index) {
            return DTO::Score{
                .name = row[start_index++].as<std::string>(),
                .score = row[start_index++].as<int64_t>(),
                .play_time_ms = row[start_index++].as<int64_t>()
            };
        }
    } // namespace detail

    ScoresRepositoryImpl::ScoresRepositoryImpl(pqxx::transaction_base& transaction) 
        : transaction_{transaction} {
    }

    void ScoresRepositoryImpl::Save(const DTO::Score& score) {
        //Генерации id делигорована СУБД
        transaction_.exec_prepared("add_score",
            score.name,
            score.score,
            score.play_time_ms
        );
    }

    std::vector<DTO::Score> ScoresRepositoryImpl::GetScores(int limit, int offset) const {
        std::vector<DTO::Score> scores;
        auto result = transaction_.exec_prepared("get_scores", limit, offset);
        for (const auto& row : result) {
            scores.push_back(detail::ScoreFromRow(row, 0));
        }
        return scores;
    }

    UnitOfWorkImpl::UnitOfWorkImpl(pqxx::connection& connection) 
        : work_{connection} 
        , scores_{work_} {
    }

    ScoresRepository& UnitOfWorkImpl::GetScores() {
        return scores_;
    }

    void UnitOfWorkImpl::Commit() {
        work_.commit();
    }

    void UnitOfWorkImpl::RollBack() {
        work_.abort();
    }

    UnitOfWorkFactoryImpl::UnitOfWorkFactoryImpl(pqxx::connection& connection) 
        : connection_{connection} {
    }

    std::unique_ptr<UnitOfWork> UnitOfWorkFactoryImpl::CreateUnitOfWork() {
        return std::make_unique<UnitOfWorkImpl>(connection_);
    }

    DataBase::DataBase(pqxx::connection connection) 
        : connection_{std::move(connection)} 
        , factory_uow_{connection_} {
        CreateTabels();
        PrepareTransaction();
    }

    UnitOfWorkFactory& DataBase::GetFactory() & {
        return factory_uow_;
    }

    void DataBase::CreateTabels() {
        pqxx::work work{connection_};
        work.exec(R"(
        CREATE TABLE IF NOT EXISTS retired_players (
            id UUID DEFAULT gen_random_uuid() CONSTRAINT player_id_constraint PRIMARY KEY,
            name varchar(100) NOT NULL,
            score INT NOT NULL,
            play_time_ms INT NOT NULL CHECK (play_time_ms >= 0)
        );  
        )"_zv);

        work.exec(R"(
        CREATE INDEX IF NOT EXISTS score_idx ON retired_players (score DESC, play_time_ms, name)
        )"_zv);
        
        work.commit();
    }

    void DataBase::PrepareTransaction() {
        //Инициализируем prepeare sql инъекции
        PrepareScore();
    }

    void DataBase::PrepareScore() {
        connection_.prepare("add_score",
            R"(
            INSERT INTO retired_players (name, score, play_time_ms)
            VALUES ($1, $2, $3)
            )"_zv
        );

        connection_.prepare("get_scores",
            R"(
            SELECT name, score, play_time_ms
            FROM retired_players
            ORDER BY score DESC, play_time_ms, name
            LIMIT $1 OFFSET $2
            )"_zv
        );
    }
} // namespace postgres