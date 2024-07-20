#include "logger.h"

namespace logger{

    void LogParametr(){
        logging::add_common_attributes();
        logging::add_console_log( 
            std::cout,
            keywords::format = &MyFormatter,
            keywords::auto_flush = true
        );
    }

    void LogInfo(const json::value& data, std::string_view message){
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, data) << message;
    }

    void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm) {
            strm << "{"s;
            auto ts = *rec[timestamp];
            strm << R"("timestamp":")"s << to_iso_extended_string(ts) << R"(",)"s; // timestamp
            strm << R"("data":)"s << json::serialize(*rec[additional_data]) << ","s; // data
            strm << R"("message":")"s << rec[logging::expressions::smessage] << R"(")"s; // message
            strm << "}"s;
    }
} //namespace logger