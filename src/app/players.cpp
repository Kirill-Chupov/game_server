#include "players.h"

namespace app {
    Player& Players::Add(model::GameSession& session, model::Dog& dog, Player::Id id) {
        const PlayerKey player_key{
            .map_id = session.GetMap()->GetId(),
            .dog_id = dog.GetId()
        };

        auto [it, inserted] = players_.try_emplace(player_key, session, dog, id, dog.GetName());
        return it->second;
    }

    Player& Players::Add(const Player& player) {
        const PlayerKey player_key{
            .map_id = player.GetSession()->GetMap()->GetId(),
            .dog_id = player.GetDog().GetId()
        };

        auto [it, insert] = players_.emplace(player_key, player);

        return it->second;
    }

    void Players::DeletePlayer(const model::Dog::Id& dog_id, const model::Map::Id& map_id) {
        PlayerKey pk {
            .map_id = map_id,
            .dog_id = dog_id
        };
        players_.erase(pk);
    }

    Player* Players::FindByDogIdAndMapId(const model::Dog::Id& dog_id, const model::Map::Id& map_id) {
        const PlayerKey player_key{
            .map_id = map_id,
            .dog_id = dog_id
        };

        if (auto it = players_.find(player_key); it != players_.end()) {
            return &it->second;
        }

        return nullptr;
    }

    std::vector<Player*> Players::GetPlayersInSession(const model::GameSession* session) {
        std::vector<Player*> result;
        for (auto& [key, player] : players_) {
            if (player.GetSession() == session) {
                result.push_back(&player);
            }
        }
        return result;
    }
} // namespace app