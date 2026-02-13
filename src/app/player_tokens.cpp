#include "player_tokens.h"
#include <cstdint>


namespace app {
    
    namespace detail {
        std::string NumberToHexString(uint64_t num) {
            static const std::string hex_chars = "0123456789ABCDEF";
            std::string result(16, '0');  // 16 символов для 64 бит
            
            for (int i = 15; i >= 0; --i) {
                if (num == 0) {
                    break;
                }

                result[i] = hex_chars[num & 0xF];  // Младшие 4 бита
                num >>= 4;  // Сдвигаем на 4 бита вправо
            }
            return result;
        }
    } //namespace detail 

    Token PlayerTokens::GetToken() {
        auto low = detail::NumberToHexString(static_cast<uint64_t>(gen_low_()));
        auto high = detail::NumberToHexString(static_cast<uint64_t>(gen_high_()));
        return high + low;
    }    

    Token PlayerTokens::AddPlayer(Player& player) {
        Token token = GetToken();
        //Исключаем дубли токенов
        while (token_to_player_.contains(token)) {
            token = GetToken();
        }

        token_to_player_[token] = &player;
        player_to_token_[&player] = token;
        return token;
    }

    Token PlayerTokens::AddPlayer(const Token& token, Player& player) {
        auto [it_t, inserted_t] = token_to_player_.emplace(token, &player);
        player_to_token_.emplace(&player, token);

        return it_t->first;
    }

    void PlayerTokens::DeleteToken(const Player* player_ptr) {
        auto token = player_to_token_.at(player_ptr);
        token_to_player_.erase(token);
        player_to_token_.erase(player_ptr);
    }

    Player* PlayerTokens::FindPlayerByToken(const Token& token) const {
        if (auto it = token_to_player_.find(token); it != token_to_player_.end()) {
            return it->second;
        }

        return nullptr;
    }

    const std::unordered_map<Token, Player*>& PlayerTokens::GetTokensPlayers() const {
        return token_to_player_;
    }

} //namespace app