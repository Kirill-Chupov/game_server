#include "application.h"
#include <pqxx/connection.hxx>


namespace app {
    Application::Application (std::string url_db) 
        : db_{pqxx::connection{url_db}}
        , use_cases_{db_.GetFactory()} {
    }

    std::pair<Token, uint64_t> Application::AddPlayer(model::GameSession& session, model::Dog& dog) {
        ++counter_player_id_;
        auto& player = players_.Add(session, dog, counter_player_id_);
        auto token = tokens_.AddPlayer(player);
        return {token, counter_player_id_};
    }

    Player* Application::FindByDogIdAndMapId(const model::Dog::Id& dog_id, const model::Map::Id& map_id){
        return players_.FindByDogIdAndMapId(dog_id, map_id);
    }

    Player* Application::FindPlayerByToken(const Token& token) const{
        return tokens_.FindPlayerByToken(token);
    }

    std::vector<Player*> Application::GetPlayersInSession(const model::GameSession* session){
        return players_.GetPlayersInSession(session);
    }

    uint64_t Application::GetCounterPlayerId() const {
        return counter_player_id_;
    }

    const std::unordered_map<Token, Player*>& Application::GetTokensPlayers() const {
        return tokens_.GetTokensPlayers();
    }

    void Application::ExitPlayer(const std::vector<DTO::ExitPlayer>& exit_players) {
        std::lock_guard<std::mutex> lock(mtx_);
        for(const auto& exit_player : exit_players) {
            model::Dog::Id dog_id (exit_player.dog_id);
            model::Map::Id map_id {exit_player.map_id};
            ExitPlayer(dog_id, map_id);
        }
    }

    std::vector<DTO::Score> Application::GetScores(int limit, int offset) const {
        std::lock_guard<std::mutex> lock(mtx_);
        return use_cases_.GetScores(limit, offset);
    }

    void Application::Restore(const std::unordered_map<Token, Player>& token_to_player, uint64_t next_player_id) {
        for(const auto& [token, player] : token_to_player) {
            app::Player& ref_player = players_.Add(player);
            tokens_.AddPlayer(token, ref_player);
        }
        counter_player_id_ = next_player_id;
    }

    void Application::ExitPlayer(const model::Dog::Id& dog_id, const model::Map::Id& map_id) {
        auto player_ptr = players_.FindByDogIdAndMapId(dog_id, map_id);
        if(player_ptr == nullptr) {
            return;
        }

        auto& dog = player_ptr->GetDog();
        auto* session = player_ptr->GetSession();
        if (session == nullptr) {
            return;
        }

        DTO::Score score {
            .name = dog.GetName(),
            .score = dog.GetScore(),
            .play_time_ms = dog.GetPlayTime()
        };

        tokens_.DeleteToken(player_ptr);
        players_.DeletePlayer(dog_id, map_id);
        session->DeleteDog(dog_id);

        use_cases_.AddScore(score);
    }
} //namespace app