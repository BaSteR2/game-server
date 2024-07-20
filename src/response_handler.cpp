#include "response_handler.h"



namespace http_response_handler {

    StringResponseHandler& StringResponseHandler::SetBasicSettings(uint version, bool keep_alive) {
        response.version(version);
        response.keep_alive(keep_alive);
        response.result(http::status::ok);   
        return *this;    
    }

    StringResponseHandler& StringResponseHandler::SetStatus(http::status status) {
        response.result(status);
        return *this;
    }

    StringResponseHandler& StringResponseHandler::SetContentType(std::string_view content_type){
        response.set(http::field::content_type, content_type);
        return *this;   
    }

    StringResponseHandler& StringResponseHandler::SetCatchControl(){
        response.set(http::field::cache_control, "no-cache");
        return *this;   
    }

    StringResponseHandler& StringResponseHandler::SetBody(const std::string& response_body){
        response.body() = response_body;
        response.content_length(response_body.size());
        return *this; 
    }

    StringResponseHandler& StringResponseHandler::SetAllowMethods(http::verb method) {
        if(method == http::verb::get)
            response.set(http::field::allow, "GET, HEAD");
        if(method == http::verb::post)
            response.set(http::field::allow, "POST");
        return *this; 
    }

    StringResponse StringResponseHandler::GetResponse(){
        return response;
    }

    StringResponse MakeJsonResponse(uint version, bool keep_alive, const std::string& body) {
        http_response_handler::StringResponseHandler string_response;
        return string_response.SetBasicSettings(version, keep_alive)
                              .SetContentType(storage_literals::ContentType::AP_JSON)
                              .SetCatchControl()
                              .SetBody(body)
                              .GetResponse();
    }

    StringResponse MakeErrorFileResponse(http::status status, uint version, bool keep_alive, const std::string& body) {
        http_response_handler::StringResponseHandler string_response;
        return string_response.SetBasicSettings(version, keep_alive)
                              .SetStatus(status)
                              .SetContentType(storage_literals::ContentType::TEXT_PL)
                              .SetBody(body)
                              .GetResponse();
    }

    StringResponse MakeInvalidArgumentResponse(uint version, bool keep_alive, std::string_view code, std::string_view message) {
        http_response_handler::StringResponseHandler string_response;
        json::value val = {{"code", code},{"message", message}};
        return string_response.SetBasicSettings(version, keep_alive)
                              .SetStatus(http::status::bad_request)
                              .SetContentType(storage_literals::ContentType::AP_JSON)
                              .SetBody(json::serialize(val))
                              .SetCatchControl()
                              .GetResponse();
    }

    StringResponse MakeNotAllowResponse(uint version, bool keep_alive, std::string_view message, http::verb allow_method) {
        http_response_handler::StringResponseHandler string_response;
        json::value val = {{"code", storage_literals::ErrorResponseType::INVALID_METHOD},{"message", message}};
        return string_response.SetBasicSettings(version, keep_alive)
                              .SetStatus(http::status::method_not_allowed)
                              .SetContentType(storage_literals::ContentType::AP_JSON)
                              .SetBody(json::serialize(val))
                              .SetAllowMethods(allow_method)
                              .SetCatchControl()
                              .GetResponse();
    }

    StringResponse MakeNotFoundResponse(uint version, bool keep_alive, std::string_view message) {
        http_response_handler::StringResponseHandler string_response;
        json::value val = {{"code", storage_literals::ErrorResponseType::MAP_NOT_FOUND},{"message", message}};
        return string_response.SetBasicSettings(version, keep_alive)
                              .SetStatus(http::status::not_found)
                              .SetContentType(storage_literals::ContentType::AP_JSON)
                              .SetBody(json::serialize(val))
                              .SetCatchControl()
                              .GetResponse();
    }


    StringResponse MakeUnauthorizedResponse(uint version, bool keep_alive, std::string_view code, std::string_view message) {
        http_response_handler::StringResponseHandler string_response;
        json::value val = {{"code", code},{"message", message}};
        return string_response.SetBasicSettings(version, keep_alive)
                              .SetStatus(http::status::unauthorized)
                              .SetContentType(storage_literals::ContentType::AP_JSON)
                              .SetBody(json::serialize(val))
                              .SetCatchControl()
                              .GetResponse();
    }

    FileResponseHandler& FileResponseHandler::SetBasicSettings(uint version, bool keep_alive) {
        response.version(version);
        response.keep_alive(keep_alive);
        return *this;
    }

    FileResponseHandler& FileResponseHandler::SetStatus(http::status status) {
        response.result(status);
        return *this;
    }

    FileResponseHandler& FileResponseHandler::SetContentType(std::string_view content_type) {
        response.insert(http::field::content_type, content_type);
        return *this;
    }

    FileResponseHandler& FileResponseHandler::SetBody(http::file_body::value_type&& file) {
        response.body() = std::move(file);
        response.prepare_payload(); 
        return *this;
    }

    FileResponse FileResponseHandler::GetResponse(){
        return std::move(response);
    }

    FileResponse MakeFileResponse(http::status status, http::file_body::value_type&& file, uint version, bool keep_alive, std::string_view content_type) {
        http_response_handler::FileResponseHandler file_response;
        file_response.SetBasicSettings(version, keep_alive)
                     .SetStatus(status)
                     .SetContentType(content_type)
                     .SetBody(std::move(file));
        return  file_response.GetResponse();
    }

    
} // namespace http_response_handler