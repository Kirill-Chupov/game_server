#include "state_serialization.h"

namespace serialization {

    model::Dog DogRepr::Restore() const {
        model::Dog::Id id{id_};
        model::Dog dog(id, name_, max_speed_, bag_capacity_);
        dog.Restore(pos_, speed_, dir_, bag_, score_);

        return dog;
    }

    app::Player PlayerRepr::Restore(model::Game& game) const {
        model::Map::Id map_id{map_id_};
        model::GameSession& session = game.GetSession(map_id);

        model::Dog::Id dog_id{dog_id_};
        auto& dogs = session.GetDogs();
        model::Dog& dog = dogs.at(dog_id);

        return app::Player(session, dog, id_, name_);
    }

    uint64_t PlayerRepr::GetId() const {
        return id_;
    }

    std::pair<std::string, uint64_t> TokenRepr::Restore() const {
        return std::pair<std::string, uint64_t>{token_, player_id_};
    }

    std::vector<DogRepr> SessionRepr::GetDogsRepr(const std::unordered_map<model::Dog::Id, model::Dog, model::GameSession::DogIdHasher>& dogs) const {
        std::vector<DogRepr> res;
        for(const auto& [dog_id, dog] : dogs) {
            res.emplace_back(dog);
        }

        return res;
    }

    void SessionRepr::Restore(model::Game& game) {
        model::Map::Id id {map_id_};
        model::GameSession& session = game.GetSession(id);
        std::vector<model::Dog> dogs;

        for(const auto& dog : dogs_) {
            dogs.emplace_back(dog.Restore());
        }
        session.Restore(dogs, loot_in_map_, next_dog_id_);
    }

    std::vector<SessionRepr> GameRepr::GetSessionsRepr(const std::unordered_map<model::Map::Id, model::GameSession, model::Game::MapIdHasher>& sessions) const {
        std::vector<SessionRepr> sessions_res;

        for(const auto& [map_id, session] : sessions) {
            sessions_res.emplace_back(session);
        }

        return sessions_res;
    }

    void GameRepr::Restore(model::Game& game) {
        for(auto& session : sessions_) {
            session.Restore(game);
        }
    }

    void ApplicationRepr::Init(const std::unordered_map<app::Token, app::Player*>& tokens_to_player) {
        for(const auto& [token, player_ptr] : tokens_to_player) {
            tokens_.emplace_back(token, *player_ptr);
            players_.emplace_back(*player_ptr);
        }
    }

    void ApplicationRepr::Restore(model::Game& game, app::Application& app) {
        std::unordered_map<uint64_t, app::Player> players;
        std::unordered_map<app::Token, app::Player> token_to_player;
        for(const auto& player : players_) {
            players.try_emplace(player.GetId(), player.Restore(game));
        }

        for(const auto& token_repr : tokens_) {
            auto [token, player_id] = token_repr.Restore();
            token_to_player.try_emplace(token, players.at(player_id));
        }

        app.Restore(token_to_player, next_player_id_);
    }

} //namespace serialization