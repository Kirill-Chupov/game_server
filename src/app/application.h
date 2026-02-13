#pragma once
#include "player.h"
#include "players.h"
#include "player_tokens.h"
#include "use_cases_impl.h"
#include "postgres.h"
#include <vector>
#include <mutex>

namespace app {
    class Application {
    public:
        explicit Application (std::string url_db);
        std::pair<Token, uint64_t> AddPlayer(model::GameSession& session, model::Dog& dog);
        Player* FindByDogIdAndMapId(const model::Dog::Id& dog_id, const model::Map::Id& map_id);
        Player* FindPlayerByToken(const Token& token) const;
        std::vector<Player*> GetPlayersInSession(const model::GameSession* session);
        uint64_t GetCounterPlayerId() const;
        const std::unordered_map<Token, Player*>& GetTokensPlayers() const;

        void ExitPlayer(const std::vector<DTO::ExitPlayer>& exit_players);
        std::vector<DTO::Score> GetScores(int limit, int offset) const;

        void Restore(const std::unordered_map<Token, Player>& token_to_player, uint64_t next_player_id);
    private:
        postgres::DataBase db_;
        postgres::UseCasesImpl use_cases_;
        mutable std::mutex mtx_;
        uint64_t counter_player_id_ = 0;
        Players players_;
        PlayerTokens tokens_;
    private:
        void ExitPlayer(const model::Dog::Id& dog_id, const model::Map::Id& map_id);
    };

} //namespace app