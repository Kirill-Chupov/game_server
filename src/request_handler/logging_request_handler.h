#pragma once
#include "request_handler.h"
#include "logger.h"
#include <chrono>


namespace http_handler {
    template<class Handler>
    class LoggingRequestHandler{
    public:
        explicit LoggingRequestHandler(Handler* handler) 
            : handler_{handler} {
        }

        template <typename Body, typename Allocator, typename Send>
        void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, std::string client_ip, Send&& send) {
            auto start_time = std::chrono::steady_clock::now();
            LogRequest(req, client_ip);

            (*handler_)(std::move(req), [start_time, send = std::forward<Send>(send), this](auto&& response){
                LogResponse(response, start_time);
                send(std::move(response));
            });
        }
    private:
        Handler* handler_;

    private:
        template <typename Body, typename Allocator> 
        void LogRequest(const http::request<Body, http::basic_fields<Allocator>>& req, const std::string& client_ip) {
            logger::Logger::LogInfo("request received"s, 
                "ip"s , client_ip,
                "URI"s, std::string(req.target()),
                "method"s, std::string(req.method_string())
            );
        }
        template<typename Response>
        static void LogResponse(const Response& res, std::chrono::steady_clock::time_point start_time) {
            auto end_time = std::chrono::steady_clock::now();
            auto response_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

            std::string content_type = "null"s;

            if (res.find(http::field::content_type) != res.end()) {
                content_type = std::string(res[http::field::content_type]);
            }

            logger::Logger::LogInfo("response sent"s,
                "response_time"s, response_time,
                "code"s, static_cast<int>(res.result()),
                "content_type"s, content_type
            );
        }
    };
} //namespace http_handler