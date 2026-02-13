#include "logger.h"
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/date_time.hpp>
#include <iostream>


namespace logger {
    using namespace std::literals;
    namespace keywords = boost::log::keywords;
    BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)

    void JsonFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
        json::object obj;

        if(auto ts = rec[timestamp]) {
            obj["timestamp"s] = boost::posix_time::to_iso_extended_string(*ts);
        }

        obj["message"s] = *rec[logging::expressions::smessage];

        if (auto data = rec[additional_data]) {
            obj["data"s] = *data;
        } else {
            obj["data"s] = json::object{};
        }

        strm << obj;
    }

    void Logger::Init(){
        logging::add_common_attributes();
        
        auto console_sink = logging::add_console_log(
            std::cout,
            keywords::format = &JsonFormatter,
            keywords::auto_flush = true
        );

        logging::core::get()->set_filter(
            logging::trivial::severity >= logging::trivial::info
        );
    }

} // namespace logger

