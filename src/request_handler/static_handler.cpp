#include "static_handler.h"

namespace http_handler {
    
    namespace detail {
        // Возвращает true, если каталог p содержится внутри base_path.
        bool IsSubPath(fs::path path, fs::path base) {
            // Приводим оба пути к каноничному виду (без . и ..)
            path = fs::weakly_canonical(path);
            base = fs::weakly_canonical(base);

            // Проверяем, что все компоненты base содержатся внутри path
            for (auto b = base.begin(), p = path.begin(); b != base.end(); ++b, ++p) {
                if (p == path.end() || *p != *b) {
                    return false;
                }
            }
            return true;
        }

        std::string_view DefinitionContentType(std::string_view target) {
            auto idx = target.find_last_of('.');
            auto type = target.substr(idx);
            // Создаем временную строку в нижнем регистре
            std::string lower_type(type);
            std::transform(lower_type.begin(), lower_type.end(), lower_type.begin(),
                        [](unsigned char c) { return std::tolower(c); });

            if(lower_type == ".htm"s || lower_type == ".html"s){
                return ContentType::TEXT_HTML;
            }

            if(lower_type == ".css"s) {
                return ContentType::TEXT_CSS;
            }

            if(lower_type == ".txt"s) {
                return ContentType::TEXT_TXT;
            }

            if(lower_type == ".js"s) {
                return ContentType::TEXT_JS;
            }

            if(lower_type == ".json"s) {
                return ContentType::APP_JSON;
            }

            if(lower_type == ".xml"s) {
                return ContentType::APP_XML;
            }

            if(lower_type == ".png"s) {
                return ContentType::IMG_PNG;
            }

            if(lower_type == ".jpg"s || lower_type == ".jpe"s || lower_type == ".jpeg"s) {
                return ContentType::IMG_JPG;
            }

            if(lower_type == ".gif"s) {
                return ContentType::IMG_GIF;
            }

            if(lower_type == ".bmp"s) {
                return ContentType::IMG_BMP;
            }

            if(lower_type == ".ico"s) {
                return ContentType::IMG_ICO;
            }

            if(lower_type == ".tiff"s || lower_type == ".tif"s) {
                return ContentType::IMG_TIF;
            }

            if(lower_type == ".svg"s || lower_type == ".svgz"s) {
                return ContentType::IMG_SVG;
            }

            if(lower_type == ".mp3"s) {
                return ContentType::AUDIO_MP3;
            }

            return ContentType::APP_BIN;
        }
    } //namespace detail

    FileResponse StaticHandler::HandleFileRequest(StringRequest&& req) {
        FileResponse answer;
        if(req.method() !=  http::verb::get && req.method() != http::verb::head) {
            answer.result(http::status::bad_request);
            answer.set(http::field::allow, "GET, HEAD"sv);
        } else {
            answer = GetFile(req);                    
        }

        return answer;
    }

    FileResponse StaticHandler::GetFile(const StringRequest& req) const{
        std::string target {req.target()};
        if(target == "/"s || target.empty()){
            target = "/index.html"s;
        }

        auto path = base_path_.string() + target;
        if (!detail::IsSubPath(path, base_path_)) {
            throw std::runtime_error("Invalid path");
        }

        http::file_body::value_type file;
        beast::error_code ec;
        file.open(path.data(), beast::file_mode::read, ec);

        if(ec) {
            throw std::runtime_error("Not found file");
        }

        http::response<http::file_body> res;
        res.version(req.version());
        res.result(http::status::ok);
        res.body() = std::move(file);
        res.prepare_payload();
        auto content_type = detail::DefinitionContentType(target);
        res.set(http::field::content_type, content_type);

        return res;
    }

}  // namespace http_handler
