#pragma once
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/beast/http/message.hpp>
#include <boost/json.hpp>
#include <boost/beast/http.hpp>
#include "storage.h"

namespace http_response_handler {

    namespace json = boost::json;
    namespace beast = boost::beast;
    namespace http = beast::http;

    using namespace std::literals;

    using StringResponse = http::response<http::string_body>;
    using FileResponse = http::response<http::file_body>;


    class StringResponseHandler {
    public:
        StringResponseHandler& SetBasicSettings(uint version, bool keep_alive);
        StringResponseHandler& SetStatus(http::status status);
        StringResponseHandler& SetContentType(std::string_view content_type);
        StringResponseHandler& SetCatchControl();
        StringResponseHandler& SetBody(const std::string& response_body);
        StringResponseHandler& SetAllowMethods(http::verb method);
        StringResponse GetResponse();

    private:
        StringResponse response;
    };

    StringResponse MakeJsonResponse(uint version, bool keep_alive, const std::string& body);
    StringResponse MakeErrorFileResponse(http::status status, uint version, bool keep_alive, const std::string& body);
    StringResponse MakeInvalidArgumentResponse(uint version, bool keep_alive, std::string_view code, std::string_view message);
    StringResponse MakeNotAllowResponse(uint version, bool keep_alive, std::string_view message, http::verb allow_method);
    StringResponse MakeNotFoundResponse(uint version, bool keep_alive, std::string_view message);
    StringResponse MakeUnauthorizedResponse(uint version, bool keep_alive, std::string_view code, std::string_view message);

    class FileResponseHandler {
    public:
        FileResponseHandler& SetBasicSettings(uint version, bool keep_alive);
        FileResponseHandler& SetStatus(http::status status);
        FileResponseHandler& SetContentType(std::string_view content_type);
        FileResponseHandler& SetBody(http::file_body::value_type&& file);
        FileResponse GetResponse();

    private:
        FileResponse response;
    };

    FileResponse MakeFileResponse(http::status status, http::file_body::value_type&& file, 
                        uint version, bool keep_alive, std::string_view content_type);

} // namespace http_response_handler