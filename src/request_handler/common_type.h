#pragma once
#include <string_view>
#include <boost/beast/http.hpp>


namespace http_handler {
    namespace beast = boost::beast;
    namespace http = beast::http;
    using namespace std::literals;

    // Запрос, тело которого представлено в виде строки
    using StringRequest = http::request<http::string_body>;
    // Ответ, тело которого представлено в виде строки
    using StringResponse = http::response<http::string_body>;
    // Ответ, тело которого представлено в виде файла
    using FileResponse = http::response<http::file_body>;

    struct ContentType {
        ContentType() = delete;
        constexpr static std::string_view TEXT_HTML = "text/html"sv;
        constexpr static std::string_view TEXT_CSS = "text/css"sv;
        constexpr static std::string_view TEXT_TXT = "text/plain"sv;
        constexpr static std::string_view TEXT_JS = "text/javascript"sv;
        constexpr static std::string_view APP_JSON = "application/json"sv;
        constexpr static std::string_view APP_XML = "application/xml"sv;
        constexpr static std::string_view APP_BIN = "application/octet-stream"sv;
        constexpr static std::string_view IMG_PNG = "image/png"sv;
        constexpr static std::string_view IMG_JPG = "image/jpeg"sv;
        constexpr static std::string_view IMG_GIF = "image/gif"sv;
        constexpr static std::string_view IMG_BMP = "image/bmp"sv;
        constexpr static std::string_view IMG_ICO = "image/vnd.microsoft.icon"sv;
        constexpr static std::string_view IMG_TIF = "image/tiff"sv;
        constexpr static std::string_view IMG_SVG = "image/svg+xml"sv;
        constexpr static std::string_view AUDIO_MP3 = "audio/mpeg"sv;
        // При необходимости внутрь ContentType можно добавить и другие типы контента
    };

    struct Endpoints {
        Endpoints() = delete;
        constexpr static std::string_view API ="/api/"sv;
        constexpr static std::string_view MAPS = "/api/v1/maps"sv;
        constexpr static std::string_view MAP_BY_ID = "/api/v1/maps/"sv;
        constexpr static std::string_view JOIN_GAME = "/api/v1/game/join"sv;
        constexpr static std::string_view PLAYERS = "/api/v1/game/players"sv;
        constexpr static std::string_view STATE = "/api/v1/game/state"sv;
        constexpr static std::string_view PLAYER_ACTION = "/api/v1/game/player/action"sv;
        constexpr static std::string_view TICK = "/api/v1/game/tick"sv;
        constexpr static std::string_view RECORDS = "/api/v1/game/records"sv;
    };

    struct AllowedMethod {
        AllowedMethod() = delete;
        constexpr static std::string_view GET_HEAD = "GET, HEAD"sv;
        constexpr static std::string_view POST = "POST"sv;
    };

}  // namespace http_handler