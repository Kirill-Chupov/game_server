#include "player.h"

namespace app {
    const model::GameSession* Player::GetSession() const {
        return &session_;
    }

    model::GameSession* Player::GetSession() {
        return &session_;
    }

    model::Dog& Player::GetDog() const {
        return dog_;
    }

    const std::string& Player::GetName() const {
        return user_name_;
    }

    Player::Id Player::GetId() const {
        return id_;
    }
} //namespace app