#pragma once
#include <boost/log/trivial.hpp>     // для BOOST_LOG_TRIVIAL
#include <boost/log/core.hpp>        // для logging::core
#include <boost/log/expressions.hpp> // для выражения, задающего фильтр 
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/json.hpp>
#include <string>




namespace logger {
    namespace logging = boost::log;
    namespace json = boost::json;
    BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", json::value)

    class Logger {
    public:
        static void Init();

        template<typename... Args>
        static void LogInfo(const std::string& message, Args&&... args){
            json::value data = BuildData(std::forward<Args>(args)...);
            BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, data) << message;
        }

        template<typename... Args>
        static void LogWarning(const std::string& message, Args&&... args){
            json::value data = BuildData(std::forward<Args>(args)...);
            BOOST_LOG_TRIVIAL(warning) << logging::add_value(additional_data, data) << message;
        }

        template<typename... Args>
        static void LogError(const std::string& message, Args&&... args){
            json::value data = BuildData(std::forward<Args>(args)...);
            BOOST_LOG_TRIVIAL(error) << logging::add_value(additional_data, data) << message;
        }

        template<typename... Args>
        static void LogFatal(const std::string& message, Args&&... args){
            json::value data = BuildData(std::forward<Args>(args)...);
            BOOST_LOG_TRIVIAL(fatal) << logging::add_value(additional_data, data) << message;
        }

    private:
        static json::value BuildData() {
             return json::object{};
        }

        template<typename T, typename... Rest>
        static json::value BuildData(const std::string& key, T&& value, Rest&&... rest) {
            json::object obj = BuildData(std::forward<Rest>(rest)...).as_object();
            obj[key] = json::value_from(std::forward<T>(value));
            return obj;
        }

    };

} // namespace logger