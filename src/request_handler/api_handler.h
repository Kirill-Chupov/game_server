#pragma once
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/bind_executor.hpp>
#include <optional>

#include "model.h"
#include "application.h"
#include "common_type.h"
#include "extra_data.h"

namespace http_handler {
    namespace net = boost::asio;
    
    using RawResponse = std::pair<http::status, std::string>;

    class ApiHandler {
    public:
        using Strand = net::strand<net::io_context::executor_type>;

        explicit ApiHandler(model::Game& game, app::Application& app, extra_data::ExtraData& extra_data, Strand api_strand, std::optional<int64_t> tick_period)
            : game_{game}
            , app_{app}
            , extra_data_{extra_data}
            , strand_{api_strand}
            , tick_period_{tick_period} {
        }
        template <typename Body, typename Allocator, typename Send>
        void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send){
            net::dispatch(strand_, [self = this, req = std::move(req), send = std::forward<Send>(send)]() mutable {
                StringResponse response = self->HandleAPIRequest(std::move(req));
                send(std::move(response));
            });
        }
    private:
        model::Game& game_;
        app::Application& app_;
        extra_data::ExtraData& extra_data_;
        Strand strand_;
        std::optional<int64_t> tick_period_;
    private:
        RawResponse HandleJoinGame(const StringRequest& req);
        RawResponse HandleGetPlayers(const StringRequest& req);
        RawResponse HandleState(const StringRequest& req);
        RawResponse HandlePlayerAction(const StringRequest& req);
        RawResponse HandleGetMaps(const StringRequest& req) const;
        RawResponse HandleTick(const StringRequest& req);
        RawResponse HandleRecords(const StringRequest& req) const;

        StringResponse HandleAPIRequest(StringRequest&& req);
        static StringResponse MakeResponse(
            http::status status, 
            std::string_view body, 
            std::string_view allowed_method, 
            unsigned http_version, 
            bool keep_alive, 
            std::string_view content_type = ContentType::APP_JSON
        );

        std::optional<RawResponse> AuthorizationPlayer(const StringRequest& req, app::Player** player);
    };
} //namespace http_handler