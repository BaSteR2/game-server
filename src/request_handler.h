#pragma once
#include "http_server.h"
#include "model.h"
#include "app.h"
#include "storage.h"
#include "response_handler.h"
#include "json_loader.h"
#include "logger.h"

#define BOOST_BEAST_USE_STD_STRING_VIEW
#include <boost/beast/http/message.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/json.hpp>

#include <variant>
#include <chrono>
#include <optional>

namespace http_handler {

    namespace json = boost::json;
    namespace beast = boost::beast;
    namespace net = boost::asio;
    namespace http = beast::http;
    namespace sys = boost::system;
    using tcp = net::ip::tcp;

    using namespace std::literals;

    using StringRequest = http::request<http::string_body>;
    using StringResponse = http::response<http::string_body>;
    using FileResponse = http::response<http::file_body>;
    using Response = std::variant<StringResponse, FileResponse>;

    class ApiHandler {
    public:
        struct ErrorInfo { 
            http::status status_;
            std::string code_;
            std::string message_;
            http::verb allow_method = http::verb::get;
        };

        explicit ApiHandler(app::Application& app, bool accept, extra_data::LootJsonData& loot_data);
        bool IsApiRequest(const StringRequest& req);
        StringResponse HandlerApiRequest(const StringRequest& req);
    private:
        std::optional<app::Token> TryExtractToken(const StringRequest& request) const;

        template <typename Fn>
        StringResponse ExecuteAuthorized(const StringRequest& request, Fn&& action) const {
            if (auto token = TryExtractToken(request)) {
                return action(*token);
            }
            return http_response_handler::MakeUnauthorizedResponse(request.version(), request.keep_alive(), 
                                        storage_literals::ErrorResponseType::INVALID_TOKEN, "Authorization header is missing");       
        }
        
        bool IsCorrectDirection(const std::string& dir) const;
        std::pair<int, int> GetParamsRfomRecordsUrl(std::string_view url) const;
        StringResponse GetMaps(const StringRequest& request) const;
        StringResponse GetMap(const StringRequest& request, const std::string& map_name) const;
        StringResponse GetJoinGame(const StringRequest& request);
        StringResponse GetPlayers(const StringRequest& request) const;
        StringResponse GetGameState(const StringRequest& request) const;
        StringResponse GetPlayerAction(const StringRequest& request);
        StringResponse GetTick(const StringRequest& request);
        StringResponse GetRecords(const StringRequest& request) const;

        bool accept_tick_ = true;
        app::Application& application_;
        extra_data::LootJsonData& loot_data_;
    };

    class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
    public:
        using Strand = net::strand<net::io_context::executor_type>;

        explicit RequestHandler(const std::filesystem::path& root, Strand& api_strand, app::Application& app, 
                                    bool accept_tick, extra_data::LootJsonData& loot_data);

        RequestHandler(const RequestHandler&) = delete;
        RequestHandler& operator=(const RequestHandler&) = delete;

        template <typename Body, typename Allocator, typename Send>
        void operator() (tcp::endpoint, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
            auto version = req.version();
            auto keep_alive = req.keep_alive();       
            try {
                if (api_handler_.IsApiRequest(req)) {
                    auto handle = [self = shared_from_this(), send,
                                req = std::forward<decltype(req)>(req), version, keep_alive] {
                        try {
                            assert(self->api_strand_.running_in_this_thread());
                            auto res = self->api_handler_.HandlerApiRequest(req);
                            return send(res);                        
                        } catch (...) { 
                            return send(self->ReportServerError(version, keep_alive));
                        }
                    };
                    return net::dispatch(api_strand_, handle);
                }
                return std::visit(
                    [&send](auto&& result) {
                        send(std::forward<decltype(result)>(result));
                    },
                    HandleFileRequest(req));
            } catch (...) {
                send(ReportServerError(version, keep_alive));
            }
        }

    private:
        using FileRequestResult = std::variant<StringResponse, FileResponse>;

        FileRequestResult HandleFileRequest(const StringRequest& req) const;
        StringResponse ReportServerError(unsigned version, bool keep_alive) const;

        int FromHexToDec(char ch) const;
        std::string URLEncoding(std::string_view uri_str) const ;
        bool IsSubPath(const std::filesystem::path& path, const std::filesystem::path& base) const ;
        std::string DefineContentType(std::string_view extention) const ;

        std::filesystem::path root_;
        Strand& api_strand_;
        ApiHandler api_handler_;
    }; 


    template<typename RequestHandler>
    class LoggingRequestHandler {
    private:
        void LogRequest(std::string address, const StringRequest req) {
            json::value request_data ({{"ip"s, address},{"URI"s, req.target()}, {"method"s, req.method_string()}});
            logger::LogInfo(request_data , "request received"sv);
        }

        void LogResponse(std::string address, int code_res, std::string_view content_type, boost::posix_time::time_duration diff) {
            json::value data({{"ip"s, address}, {"response_time"s, diff.total_milliseconds()},{"code"s, code_res}, 
                                    {"content_type"s, content_type}});     
            logger::LogInfo( data, "response sent"sv);
        }

    public:
        using Strand = net::strand<net::io_context::executor_type>;

        explicit LoggingRequestHandler(RequestHandler& request_handler, Strand& api_strand)
            : decorated_(request_handler)
            , api_strand_{api_strand}
        {}

        LoggingRequestHandler(const LoggingRequestHandler&) = delete;
        LoggingRequestHandler& operator=(const LoggingRequestHandler&) = delete;

        template <typename Body, typename Allocator, typename Send>
        void operator()(tcp::endpoint endpoint, http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) { 
            std::string address =  endpoint.address().to_string();
            LogRequest(address, req);
            auto now = boost::posix_time::microsec_clock::local_time();
            auto my_send = [this, send, now, address](auto&& result){
                int code_res = result.result_int(); 
                std::string_view content_type = result.at(http::field::content_type);  
                auto resp_ready = boost::posix_time::microsec_clock::local_time(); 
                boost::posix_time::time_duration diff = resp_ready - now;
                this->LogResponse(address, code_res, content_type, diff);
                send(result);
            };        
            decorated_(endpoint, std::move(req), std::move(my_send));
        }

    private:
        Strand& api_strand_;
        RequestHandler& decorated_;
    };

}  // namespace http_handler
