#pragma once
#include <filesystem>
#include "model.h"
#include "api_handler.h"
#include "static_handler.h"
#include "common_type.h"
#include "application.h"

namespace http_handler {
    namespace fs = std::filesystem;

    class RequestHandler {
    public:
        using Strand = net::strand<net::io_context::executor_type>;

        explicit RequestHandler(model::Game& game, app::Application& app, extra_data::ExtraData& extra_data, fs::path base_path, Strand api_strand, std::optional<int64_t> tick_period)
            : api_handler_{game, app, extra_data, api_strand, tick_period}
            , static_handler_{base_path} {
        }

        RequestHandler(const RequestHandler&) = delete;
        RequestHandler& operator=(const RequestHandler&) = delete;

        template <typename Body, typename Allocator, typename Send>
        void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
            // Обработать запрос request и отправить ответ, используя send
            if ( req.target().starts_with("/api/") ) {
                api_handler_(std::move(req), std::forward<Send>(send));
            } else {
                static_handler_(std::move(req), std::forward<Send>(send));          
            }        
        }

    private:
        ApiHandler api_handler_;
        StaticHandler static_handler_;
    };

}  // namespace http_handler
