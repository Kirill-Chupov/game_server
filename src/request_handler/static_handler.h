#pragma once
#include <filesystem>
#include "common_type.h"
#include <string_view>

namespace http_handler {
    namespace fs = std::filesystem;
    using namespace std::literals;
    
    class StaticHandler {
    public:
        explicit StaticHandler(fs::path base_path)
        : base_path_{fs::weakly_canonical(base_path)} {

        }
        template <typename Body, typename Allocator, typename Send>
        void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send){
            try {
                FileResponse response = HandleFileRequest(std::move(req));
                send(std::move(response));
            } catch (std::exception& ex) {
                StringResponse error;
                
                if(std::string_view(ex.what()) == "Invalid path"sv){
                    error.result(http::status::bad_request);
                } else {
                    error.result(http::status::not_found);
                }
                
                error.set(http::field::content_type, ContentType::TEXT_TXT);
                error.body() = ex.what();
                error.prepare_payload();
                send(std::move(error));
            }  
        }
    private:
        fs::path base_path_;
    private:
        FileResponse GetFile(const StringRequest& req) const;
        FileResponse HandleFileRequest(StringRequest&& req);
    };

} //namespace http_handler