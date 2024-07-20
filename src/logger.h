#pragma once

#include <chrono>
#include <iomanip>
#include <string>
#include <thread>
#include <iostream>

#include "json_loader.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/date_time.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>


namespace logger {
    namespace sys = boost::system;
    using namespace std::literals;

    namespace logging = boost::log;
    namespace sinks = boost::log::sinks;
    namespace keywords = boost::log::keywords;
    namespace expr = boost::log::expressions;
    namespace attrs = boost::log::attributes;

    BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
    BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", json::value)

    void LogParametr();
    void LogInfo(const json::value& data, std::string_view message);
    void MyFormatter(logging::record_view const& rec, logging::formatting_ostream& strm);
    
} //namespace logger