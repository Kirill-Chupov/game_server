#pragma once
#include <string>
#include <random>
#include <unordered_map>
#include "player.h"


namespace app {
    using Token = std::string;

    class PlayerTokens {
    public:
        PlayerTokens() = default;
        
        Token AddPlayer(Player& player);
        Token AddPlayer(const Token& token, Player& player);
        void DeleteToken(const Player* player_ptr);
        Player* FindPlayerByToken(const Token& token) const;
        const std::unordered_map<Token, Player*>& GetTokensPlayers() const;

    private:
        std::unordered_map<Token, Player*> token_to_player_;
        std::unordered_map<const Player*, Token> player_to_token_;
        std::random_device rd_;
        std::mt19937_64 gen_low_{[this] {
            std::uniform_int_distribution<std::mt19937_64::result_type> dist;
            return dist(rd_);
        }()};
        std::mt19937_64 gen_high_{[this] {
            std::uniform_int_distribution<std::mt19937_64::result_type> dist;
            return dist(rd_);
        }()};
    private:
        Token GetToken();
    };
} //namespace app