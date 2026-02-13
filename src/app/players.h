#pragma once
#include "player.h"
#include "model.h"
#include "dog.h"
#include <unordered_map>


namespace app {
    class Players {
        struct PlayerKey {
            model::Map::Id map_id;
            model::Dog::Id dog_id;
            bool operator==(const PlayerKey& other) const {
                return map_id == other.map_id && dog_id == other.dog_id;
            }
        };

        struct PlayerKeyHasher {
            size_t operator()(const PlayerKey& key) const {
                // Комбинируем хеши обоих ID
                size_t h1 = util::TaggedHasher<model::Map::Id>{}(key.map_id);
                size_t h2 = util::TaggedHasher<model::Dog::Id>{}(key.dog_id);
                
                return h1 + h2 * 37;
            }
        };
    public:
        Player& Add(model::GameSession& session, model::Dog& dog, Player::Id id);
        Player& Add(const Player& player);
        void DeletePlayer(const model::Dog::Id& dog_id, const model::Map::Id& map_id);
        Player* FindByDogIdAndMapId(const model::Dog::Id& dog_id, const model::Map::Id& map_id);
        std::vector<Player*> GetPlayersInSession(const model::GameSession* session);
    private:
        std::unordered_map<PlayerKey, Player, PlayerKeyHasher> players_;
    };
} // namespace app