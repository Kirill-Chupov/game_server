#pragma once

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <cstdint>

#include "model.h"
#include "application.h"


namespace boost {
    namespace serialization {
        template<class Archive>
        void serialize(Archive& ar, model::Position& pos, [[maybe_unused]] const unsigned int version) {
            ar & pos.x;
            ar & pos.y;
        }

        template<class Archive>
        void serialize(Archive& ar, model::Speed& speed, [[maybe_unused]] const unsigned int version) {
            ar & speed.h_speed;
            ar & speed.v_speed;
        } 

        template<class Archive>
        void serialize(Archive& ar, model::Loot& loot, [[maybe_unused]] const unsigned int version) {
            ar & loot.pos;
            ar & loot.price;
            ar & loot.type;
        }
    }// namespace serialization
} // namespace boost


namespace serialization {

    class DogRepr {
    public:
        DogRepr() = default;
       explicit DogRepr(const model::Dog& dog) 
            : id_{*dog.GetId()}
            , name_{dog.GetName()}
            , pos_{dog.GetPos()}
            , speed_{dog.GetSpeed()}
            , max_speed_{dog.GetMaxSpeed()}
            , dir_{dog.GetDir()}
            , bag_{dog.GetBag()}
            , bag_capacity_{dog.GetBagCapacity()}
            , score_{dog.GetScore()} {
        }

        template<class Archive>
        void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
            ar & id_;
            ar & name_;
            ar & pos_;
            ar & speed_;
            ar & max_speed_;
            ar & dir_;
            ar & bag_;
            ar & bag_capacity_;
            ar & score_;
        }

        model::Dog Restore() const;
    private:
        uint64_t id_ = 0;
        std::string name_;
        model::Position pos_;
        model::Speed speed_;
        double max_speed_ = 0;
        model::Direction dir_;
        std::vector<model::Loot> bag_;
        size_t bag_capacity_ = 0;
        int score_ = 0;
    };

    class PlayerRepr {
    public:
        PlayerRepr() = default;
        explicit PlayerRepr (const app::Player& player) 
            : id_{player.GetId()}
            , name_{player.GetName()}
            , dog_id_{*player.GetDog().GetId()} 
            , map_id_{*player.GetSession()->GetMap()->GetId()} {
        }

        template<class Archive>
        void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
            ar & id_;
            ar & name_;
            ar & dog_id_;
            ar & map_id_;
        }

        app::Player Restore(model::Game& game) const;
        uint64_t GetId() const;
    private:
        uint64_t id_ = 0;
        std::string name_;
        uint64_t dog_id_ = 0;
        std::string map_id_;
    };

    class TokenRepr {
    public:
        TokenRepr() = default;
        explicit TokenRepr(const std::string& token, const app::Player& player) 
            : token_{token}
            , player_id_{player.GetId()}{
        }

        template<class Archive>
        void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
            ar & token_;
            ar & player_id_;
        }

        std::pair<std::string, uint64_t> Restore() const;
    private:
        std::string token_;
        uint64_t player_id_ = 0;
    };

    class SessionRepr {
    public:
        SessionRepr() = default;
        explicit SessionRepr(const model::GameSession& session) 
            : map_id_{*session.GetMap()->GetId()}
            , dogs_{GetDogsRepr(session.GetDogs())}
            , loot_in_map_{session.GetLootInMap()}
            , next_dog_id_{session.GetCounterDogId()} {
        }

        template<class Archive>
        void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
            ar & map_id_;
            ar & dogs_;
            ar & loot_in_map_;
            ar & next_dog_id_;
        }

        void Restore(model::Game& game);
    private:
        std::string map_id_;
        std::vector<DogRepr> dogs_;
        std::vector<model::Loot> loot_in_map_;
        uint64_t next_dog_id_ = 0;
    private:
        std::vector<DogRepr> GetDogsRepr(const std::unordered_map<model::Dog::Id, model::Dog, model::GameSession::DogIdHasher>& dogs) const;
    };

    class GameRepr {
    public:
        GameRepr() = default;
        explicit GameRepr(const model::Game& game) 
            : sessions_{GetSessionsRepr(game.GetSessions())} {
        }

        template<class Archive>
        void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
            ar & sessions_;
        }

        void Restore(model::Game& game);
    private:
        std::vector<SessionRepr> sessions_;
    private:
        std::vector<SessionRepr> GetSessionsRepr(const std::unordered_map<model::Map::Id, model::GameSession, model::Game::MapIdHasher>& sessions) const;
    };

    class ApplicationRepr {
    public:
        ApplicationRepr() = default;
        explicit ApplicationRepr(const app::Application& app) 
            : next_player_id_{app.GetCounterPlayerId()} {
            Init(app.GetTokensPlayers());
        }

        template<class Archive>
        void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
            ar & players_;
            ar & tokens_;
            ar & next_player_id_;
        }

        void Restore(model::Game& game, app::Application& app);
    private:
        std::vector<PlayerRepr> players_;
        std::vector<TokenRepr> tokens_;
        uint64_t next_player_id_ = 0;
    private:
        void Init(const std::unordered_map<app::Token, app::Player*>& tokens_to_player);
    };

} //namespace serialization