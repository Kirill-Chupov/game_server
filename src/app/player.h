#pragma once
#include "model.h"
#include "dog.h"
#include <cstdint>


namespace app {
    class Player {
    public:
        using Id = uint64_t;

        explicit Player(model::GameSession& session, model::Dog& dog, Id id, const std::string& user_name) 
            : session_{session}
            , dog_{dog}
            , id_{id}
            , user_name_{user_name} {
        }

        const model::GameSession* GetSession() const;
        model::GameSession* GetSession();
        model::Dog& GetDog() const;
        const std::string& GetName() const;
        Player::Id GetId() const;
        
    private:
        model::GameSession& session_;
        model::Dog& dog_;
        Id id_;
        std::string user_name_;
    };
} //namespace app