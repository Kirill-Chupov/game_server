#include "api_handler.h"

#include <boost/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>


namespace json = boost::json;

namespace http_handler {
    using namespace std::literals;
    namespace detail {
        namespace serialize {
            json::object SerializeRoad (const model::Road& road) {
                auto start = road.GetStart();
                auto end = road.GetEnd();
                if(road.IsVertical()){
                    return json::object{
                        {"x0", start.x},
                        {"y0", start.y},
                        {"y1", end.y}
                    };
                }

                return json::object{
                        {"x0", start.x},
                        {"y0", start.y},
                        {"x1", end.x}
                };
            }

            json::array SerializeRoads(const std::shared_ptr<model::Map> map) {
                json::array roads_arr;
                for(const auto& road : map->GetRoads()){
                    roads_arr.push_back(SerializeRoad(road));
                }

                return roads_arr;
            }

            json::object SerializeBuilding (const model::Building& building) {
                auto pos = building.GetBounds().position;
                auto size = building.GetBounds().size;
                return json::object {
                    {"x", pos.x},
                    {"y", pos.y},
                    {"w", size.width},
                    {"h", size.height}
                };
            }

            json::array SerializeBuildings (const std::shared_ptr<model::Map> map) {
                json::array buildings_arr;
                for(const auto& building : map->GetBuildings()){
                    buildings_arr.push_back(SerializeBuilding(building));
                }

                return buildings_arr;
            }

            json::object SerializeOffice (const model::Office& office) {
                auto pos = office.GetPosition();
                auto offset = office.GetOffset();
                return json::object{
                    {"id"s, *office.GetId()},
                    {"x", pos.x},
                    {"y", pos.y},
                    {"offsetX", offset.dx},
                    {"offsetY", offset.dy}
                };
            }

            json::array SerializeOffices (const std::shared_ptr<model::Map> map) {
                json::array offices_arr;
                for(const auto& office : map->GetOffices()){
                    offices_arr.push_back(SerializeOffice(office));
                }

                return offices_arr;
            }

            json::object SerializeMap(const std::shared_ptr<model::Map> map, const extra_data::ExtraData& extra_data) {
                auto loot_types = extra_data.GetLootTypes(map->GetId());
                return json::object {
                    { "id"s, *map->GetId() },
                    { "name"s, map->GetName() },
                    {"lootTypes"s, loot_types.has_value() ? loot_types.value() : json::array{} },
                    { "roads"s, SerializeRoads(map)},
                    { "buildings"s , SerializeBuildings(map) },
                    { "offices"s , SerializeOffices(map) }
                };
            }

            json::array SerializePosition(model::Position pos) {
                return json::array{pos.x, pos.y};
            }

            json::array SerializeSpeed(model::Speed speed) {
                return json::array{speed.h_speed, speed.v_speed};
            }

            const std::string& SerializeDirection(model::Direction dir) {
                static const std::string UP = "U"s;
                static const std::string DOWN = "D"s;
                static const std::string LEFT = "L"s;
                static const std::string RIGHT = "R"s;
                static const std::string UNKNOWN = "unknown dir"s;

                switch (dir) {
                case model::Direction::NORTH:
                    return UP;
                    break;
                case model::Direction::SOUTH:
                    return DOWN;
                    break;
                case model::Direction::WEST:
                    return LEFT;
                    break;
                case model::Direction::EAST:
                    return RIGHT;
                    break;
                default:
                    return UNKNOWN;
                    break;
                }
            }

            json::object SerializeLootBag(const model::Loot& loot) {
                return json::object{
                    {"type", loot.type},
                    {"id", loot.id}
                };
            }

            json::object SerializeLootMap(const model::Loot& loot) {
                return json::object{
                    {"type", loot.type},
                    {"pos", SerializePosition(loot.pos)}
                };
            }

            json::array SerializeBag(const model::Dog::Bag& bag) {
                json::array res;

                for(const auto& loot : bag) {
                    res.emplace_back(SerializeLootBag(loot));
                }

                return res;
            }

            json::object SerializeDog(const model::Dog* dog) {
                if (dog == nullptr) {
                    return json::object {
                        {"pos", json::array{}},
                        {"speed", json::array{}},
                        {"dir", "unknown dir"s}
                    };
                }

                return json::object {
                    {"pos", SerializePosition(dog->GetPos())},
                    {"speed", SerializeSpeed(dog->GetSpeed())},
                    {"dir", SerializeDirection(dog->GetDir())},
                    {"bag", SerializeBag(dog->GetBag())},
                    {"score", dog->GetScore()}
                };
            }

            json::object SerializeLootsMap(const std::vector<model::Loot>& loot_in_map) {
                size_t size = loot_in_map.size();
                json::object lost_object;

                for (size_t i = 0; i < size; ++i) {
                    lost_object[std::to_string(i)] = SerializeLootMap(loot_in_map[i]);
                }

                return lost_object;
            }

        } //namespace serialize

        std::string GetMaps(std::string_view target, const model::Game& game){
            json::array arr;
            for(const auto map : game.GetMaps()){
                json::object obj {
                    {"id"s, *map->GetId()},
                    {"name"s, map->GetName()}
                };
                arr.emplace_back(std::move(obj));
            }

            return json::serialize(arr);
        }

        std::optional<std::string> GetMap(std::string_view target, const model::Game& game, const extra_data::ExtraData& extra_data) {
            auto id_str = std::string{target.substr(13)};
            auto id = model::Map::Id{id_str};
            const auto map = game.FindMap(id);
            if(map == nullptr){
                return std::nullopt;
            }

            auto map_json = serialize::SerializeMap(map, extra_data);
            return json::serialize(map_json);
        }

        std::string MakeError(std::string_view code, std::string_view message) {
            json::object error{
                {"code", code},
                {"message", message}
            };

            return json::serialize(error);
        }

        template<typename... Verbs>
        std::optional<RawResponse> ValidateHttpMethod(const StringRequest& req, Verbs&&... allowed_verbs) {
            static const RawResponse error {
                http::status::method_not_allowed,
                detail::MakeError("invalidMethod"sv, "Invalid method"sv)
            };

            auto method = req.method();

            if (((method == allowed_verbs) || ...)) {
                //Метод поддерживается
                return std::nullopt;
            }
            
            return error;
        }

        std::unordered_map<std::string, std::string> ParseQueryParams(const StringRequest& req, std::string_view endpoint) {
            std::string_view target = req.target();
            if (!target.starts_with(endpoint)) {
                throw std::invalid_argument("Invalid endpoint");
            }

            std::string_view query = target.substr(endpoint.size());
            std::unordered_map<std::string, std::string> params;

            if(query.empty() || query[0] != '?') {
                return params;
            }

            // Убираем '?'
            query.remove_prefix(1);

            std::istringstream iss{std::string{query}};
            std::string pair;

            while (std::getline(iss, pair, '&')) {
                size_t equal_pos = pair.find('=');
                if(equal_pos != std::string::npos) {
                    std::string key = pair.substr(0, equal_pos);
                    std::string value = pair.substr(equal_pos + 1);
                    params[key] = value;
                }
            }

            return params;
        }

    } //namespace detail

    StringResponse ApiHandler::MakeResponse(http::status status, std::string_view body, std::string_view allowed_method, unsigned http_version, bool keep_alive, std::string_view content_type) {
        StringResponse response(status, http_version);
        response.set(http::field::content_type, content_type);
        response.body() = body;
        response.content_length(body.size());
        response.keep_alive(keep_alive);
        response.set(http::field::cache_control, "no-cache"sv);
        if (status == http::status::method_not_allowed) {
            response.set(http::field::allow, allowed_method);
        }
        return response;
    }

    std::optional<RawResponse> ApiHandler::AuthorizationPlayer(const StringRequest& req, app::Player** player) {
        constexpr static std::string_view bearer_prefix = "Bearer ";

        static const RawResponse not_found_header = {http::status::unauthorized, detail::MakeError("invalidToken"sv, "Authorization header is required"sv)};
        static const RawResponse invalid_header_format = {http::status::unauthorized, detail::MakeError("invalidToken"sv, "Invalid authorization header format"sv)};
        static const RawResponse invalid_token = {http::status::unauthorized, detail::MakeError("invalidToken"sv, "Invalid token"sv)};
        static const RawResponse not_found_player = {http::status::unauthorized, detail::MakeError("unknownToken"sv, "Player token has not been found"sv)};

        if(player == nullptr) {
            throw std::invalid_argument("Uninitialized argument");
        }
        
        auto auth_header = req.find(http::field::authorization);
        if (auth_header == req.end()) {
            return not_found_header;
        }

        std::string auth_value = std::string(auth_header->value());
        if (auth_value.size() < bearer_prefix.size() || auth_value.substr(0, bearer_prefix.size()) != bearer_prefix) {
            return invalid_header_format;
        }
        
        std::string token = auth_value.substr(bearer_prefix.size());
        if (token.empty() || token.size() != 32) {
            return invalid_token;
        }

        *player = app_.FindPlayerByToken(token);
        if (*player == nullptr) {
            return not_found_player;
        }

        return std::nullopt;
    }

    StringResponse ApiHandler::HandleAPIRequest(StringRequest&& req) {
        const auto json_response = [&req](http::status status, std::string_view text, std::string_view allowed_method) {
            return MakeResponse(status, text, allowed_method, req.version(), req.keep_alive());
        };

        auto target = req.target();

        if (target == Endpoints::JOIN_GAME) {
            auto [status, str] = HandleJoinGame(req);           
            return json_response(status, str, AllowedMethod::POST);
        }

        if (target == Endpoints::PLAYERS) {
            auto [status, str] = HandleGetPlayers(req);
            return json_response(status, str, AllowedMethod::GET_HEAD);
        }

        if (target == Endpoints::STATE) {
            auto [status, str] = HandleState(req);
            return json_response(status, str, AllowedMethod::GET_HEAD);
        }

        if (target == Endpoints::PLAYER_ACTION) {
            auto [status, str] = HandlePlayerAction(req);
            return json_response(status, str, AllowedMethod::POST);
        }

        if (target == Endpoints::TICK) {
            auto [status, str] = HandleTick(req);
            return json_response(status, str, AllowedMethod::POST);
        }

        if (target.starts_with(Endpoints::RECORDS)) {
            auto [status, str] = HandleRecords(req);
            return json_response(status, str, AllowedMethod::GET_HEAD);
        }

        auto [status, str] = HandleGetMaps(req);
        return json_response(status, str, AllowedMethod::GET_HEAD);
    }

    RawResponse ApiHandler::HandleGetMaps(const StringRequest& req) const{
        if(auto error = detail::ValidateHttpMethod(req, http::verb::get, http::verb::head)) {
            return *error;
        }

        auto target = req.target();

        if(target == Endpoints::MAPS) {
            auto maps = detail::GetMaps(target, game_);
            return {http::status::ok, maps};
        }

        if(target.starts_with(Endpoints::MAP_BY_ID)  && target.size() > Endpoints::MAP_BY_ID.size()){
            auto map = detail::GetMap(target, game_, extra_data_);
            if (map.has_value()) {
                return {http::status::ok, map.value()};
            }

            return {http::status::not_found, detail::MakeError("mapNotFound"sv, "Map not found"sv)};
        }

        if (target.starts_with(Endpoints::API) && target.size() > Endpoints::API.size()) {
            return {http::status::bad_request, detail::MakeError("badRequest"sv, "Bad request"sv)};
        }

        return {http::status::not_found, detail::MakeError("notFound"sv, target)};
    }
    
    RawResponse ApiHandler::HandleJoinGame(const StringRequest& req){
        if(auto error = detail::ValidateHttpMethod(req, http::verb::post)) {
            return *error;
        }

        try {
            json::value body = json::parse(req.body());
            const json::object& obj = body.as_object();

            std::string userName = std::string(obj.at("userName").as_string());
            if (userName.empty()) {
                return {http::status::bad_request, detail::MakeError("invalidArgument"sv, "Invalid userName"sv)};
            }

            std::string mapId = std::string(obj.at("mapId").as_string());
            auto map_ptr = game_.FindMap(model::Map::Id{mapId});

            if(map_ptr == nullptr) {
                return {http::status::not_found, detail::MakeError("mapNotFound", "Map not found")};
            }

            auto& session = game_.GetSession(model::Map::Id{mapId});
            auto& dog = session.AddDog(userName);
            auto [token, player_id] = app_.AddPlayer(session, dog);

            json::object answer {
                {"authToken", token},
                {"playerId", player_id}
            };

            return {http::status::ok, json::serialize(answer)};
        } catch (const std::exception& ex) {
            std::string error = "Join game request parse error: "s + std::string(ex.what());
            return {http::status::bad_request, detail::MakeError("invalidArgument"sv, error)};
        }
    }
    

    RawResponse ApiHandler::HandleGetPlayers(const StringRequest& req){
        if(auto error = detail::ValidateHttpMethod(req, http::verb::get, http::verb::head)) {
            return *error;
        }

        app::Player* player = nullptr;
        if(auto error = AuthorizationPlayer(req, &player)) {
            return *error;
        }

        const auto* session = player->GetSession();
        auto players = app_.GetPlayersInSession(session);

        json::object players_json;
        for (auto* p : players) {
            std::string id = std::to_string(p->GetId());

            json::object player_info;
            player_info["name"] = p->GetName();
            players_json[id] = player_info;
        }

        return {http::status::ok, json::serialize(players_json)};
    }

    RawResponse ApiHandler::HandleState(const StringRequest& req) {
        if(auto error = detail::ValidateHttpMethod(req, http::verb::get, http::verb::head)) {
            return *error;
        }

        app::Player* player = nullptr;
        if(auto error = AuthorizationPlayer(req, &player)) {
            return *error;
        }

        const auto* session = player->GetSession();
        auto players = app_.GetPlayersInSession(session);

        const auto& loot_in_map = session->GetLootInMap();
        auto lost_object = detail::serialize::SerializeLootsMap(loot_in_map);

        json::object state_json {
            {"players", json::object{}},
            {"lostObjects", std::move(lost_object)}
        };

        auto& players_json = state_json["players"].as_object();

        for (auto* p : players) {
            std::string id = std::to_string(p->GetId());
            players_json[id] = detail::serialize::SerializeDog(&p->GetDog());
        }

        return {http::status::ok, json::serialize(state_json)};
    }
    
    RawResponse ApiHandler::HandlePlayerAction(const StringRequest& req) {
        if(auto error = detail::ValidateHttpMethod(req, http::verb::post)) {
            return *error;
        }

        app::Player* player = nullptr;
        if(auto error = AuthorizationPlayer(req, &player)) {
            return *error;
        }

        auto content_header = req.find(http::field::content_type);
        if (content_header == req.end() || content_header->value() != ContentType::APP_JSON) {
            return {http::status::bad_request, detail::MakeError("invalidArgument"sv, "Invalid content type"sv)};
        }

        try{
            auto body = json::parse(req.body());
            const auto& obj = body.as_object();

            auto it = obj.find("move");
            if (it == obj.end()) {
                throw std::invalid_argument("Missing \"move\" field");
            }

            if (!it->value().is_string()) {
                throw std::invalid_argument("Field \"move\" must be an string");
            }

            const std::string dir {it->value().get_string()};

            if (!dir.empty() && dir != "U" && dir != "D" && dir != "L" && dir != "R") {
                throw std::invalid_argument("Invalid Direction");
            }

            auto& dog = player->GetDog();
            dog.Action(dir);
        } catch (const std::exception& ex) {
            std::string error = "Failed to parse action: "s + std::string(ex.what());
            return {http::status::bad_request, detail::MakeError("invalidArgument"sv, error)};
        }

        return{http::status::ok, json::serialize(json::object())};
    }

    RawResponse ApiHandler::HandleTick(const StringRequest& req) {
        if (tick_period_.has_value()) {
            return {http::status::bad_request, detail::MakeError("badRequest"sv, "Invalid endpoint"sv)};
        }
        
        if(auto error = detail::ValidateHttpMethod(req, http::verb::post)) {
            return *error;
        }
        
        try {
            auto body = json::parse(req.body());
            const auto& obj = body.as_object();

            auto it = obj.find("timeDelta");
            if (it == obj.end()) {
                throw std::invalid_argument("Missing \"timeDelta\" field");
            }

            if (!it->value().is_int64()) {
                throw std::invalid_argument("Field \"timeDelta\" must be an interger");
            }

            const int64_t time_delta = it->value().get_int64();

            if (time_delta <= 0) {
                throw std::invalid_argument("Invalid time delta");
            }

            game_.Tick(time_delta);

        }   catch (const std::exception& ex) {
            std::string error = "Failed to parse tick request JSON: "s + std::string(ex.what());
            return {http::status::bad_request, detail::MakeError("invalidArgument"sv, error)};
        }

        return{http::status::ok, json::serialize(json::object())};
    }

    RawResponse ApiHandler::HandleRecords(const StringRequest& req) const {
        if(auto error = detail::ValidateHttpMethod(req, http::verb::get, http::verb::head)) {
            return *error;
        }

        try {
            auto params = detail::ParseQueryParams(req, Endpoints::RECORDS);
            int64_t max_items = 100;
            int64_t start = 0;
            
            if (auto it = params.find("maxItems"); it != params.end()) {
                max_items = std::stoi(it->second);
            }

            if (max_items > 100) {
                throw std::invalid_argument("Value max_items > 100");
            }

            if (auto it = params.find("start"); it != params.end()) {
                start = std::stoi(it->second);
            }

            auto scores = app_.GetScores(max_items, start);
            json::array score_json;
            for (const auto& score : scores) {
                score_json.push_back(
                    json::object{
                        {"name", score.name},
                        {"score", score.score},
                        {"playTime", static_cast<double>(score.play_time_ms / 1000.0)}
                    }
                );
            }

            return {http::status::ok, json::serialize(score_json)};
        } catch (const std::exception& ex) {
            std::string error = "Error: "s + std::string(ex.what());
            return {http::status::bad_request, detail::MakeError("invalidArgument"sv, error)};
        }
    }

}  // namespace http_handler
