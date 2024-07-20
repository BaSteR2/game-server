#include "request_handler.h"

namespace http_handler {

    using namespace storage_literals;

    int RequestHandler::FromHexToDec(char ch) const { 
        if (ch >= '0' && ch <= '9') {
            return ch - '0';
        }
        else if (ch >= 'A' && ch <= 'Z') {
            return ch - 'A' + 10;
        }
        else if (ch >= 'a' && ch <= 'z') {
            return ch - 'a' + 10;
        }
        else {
            return -1;
        }
    }

    std::string RequestHandler::URLEncoding(std::string_view uri_str) const {
        std::string result;
        for (size_t i = 0; i < uri_str.size();) {
            if (uri_str[i] == '%') {
                result += (FromHexToDec(uri_str[i + 1]) * 16 + FromHexToDec(uri_str[i + 2]));
                i += 3;
            }
            else if (uri_str[i] == '+') {
                result += ' ';
                ++i;
            }
            else {
                result += uri_str[i];
                ++i;
            }
        }
        return result;
    }

    bool RequestHandler::IsSubPath(const std::filesystem::path& path, const std::filesystem::path& base) const {
        auto path_can = std::filesystem::weakly_canonical(path);
        auto base_can = std::filesystem::weakly_canonical(base);
        for (auto b = base_can.begin(), p = path_can.begin(); b != base_can.end(); ++b, ++p) {
            if (p == path_can.end() || *p != *b) {
                return false;
            }
        }
        return true;
    }

    std::string RequestHandler::DefineContentType(std::string_view extention) const {
        std::string content_type;
        if (extention == ".xml"sv) {
            content_type = ContentType::APP_XML;
        }
        if (extention == ".json"sv) {
            content_type = ContentType::AP_JSON;
        }
        if (extention == ".bmp"sv) {
            content_type = ContentType::IMG_BMP;
        }
        if (extention == ".gif"sv) {
            content_type = ContentType::IMG_GIF;
        }
        if (extention == ".ico"sv) {
            content_type = ContentType::IMG_ICO;
        }
        if (extention == ".jpeg"sv || extention == ".jpg"sv || extention == ".jpe"sv) {
            content_type = ContentType::IMG_JPG;
        }
        if (extention == ".mp3"sv) {
            content_type = ContentType::IMG_MPEG;
        }
        if (extention == ".png"sv) {
            content_type = ContentType::IMG_PNG;
        }
        if (extention == ".svg"sv || extention == ".svgz"sv) {
            content_type = ContentType::IMG_SVG;
        }
        if (extention == ".tif"sv || extention == ".tiff"sv) {
            content_type = ContentType::IMG_TIFF;
        }
        if (extention == ".css"sv) {
            content_type = ContentType::TEXT_CSS;
        }
        if (extention == ".html"sv || extention == ".htm"sv) {
            content_type = ContentType::TEXT_HTML;
        }
        if (extention == ".js"sv) {
            content_type = ContentType::TEXT_JS;
        }
        if (extention == ".txt"sv) {
            content_type = ContentType::TEXT_PL;
        }
        if (extention == ""sv) {
            content_type = ContentType::APP_EMPT;
        }
        return content_type;
    }

    ApiHandler::ApiHandler(app::Application& app, bool accept, extra_data::LootJsonData& loot_data)
        : application_(app)
        , accept_tick_(accept)
        , loot_data_(loot_data)
    {}

    bool ApiHandler::IsApiRequest(const StringRequest& req) {
        return req.target().starts_with(URIEndpoints::ENDPOINT_API);
    }

    bool ApiHandler::IsCorrectDirection(const std::string& dir) const {
        return dir != model::Direction::NORTH && dir != model::Direction::WEST && 
               dir != model::Direction::EAST && dir != model::Direction::SOUTH && dir != "";
    }

    std::pair<int, int> ApiHandler::GetParamsRfomRecordsUrl(std::string_view url) const {
        int offset = 0;
        int max_elements = 100;
        url.remove_prefix(std::min(url.size(), url.find_first_of("?"))); 
        if(!url.empty() || url != "?") {
            if(url.starts_with("?start=")) {
                url.remove_prefix(url.find_first_of("="));
                offset = std::stoi(std::string(url.substr(1, url.find_first_of("&"))));
                url.remove_prefix(url.find_first_of("&") + 1);
                if(url.starts_with("maxItems=")) {
                    url.remove_prefix(url.find_first_of("="));
                    max_elements = std::stoi(std::string(url.substr(1)));
                }
            }
        }
        return {offset, max_elements};
    }

    StringResponse ApiHandler::GetMaps(const StringRequest& request) const {
        if(request.method() != http::verb::get && request.method() != http::verb::head)
                    return http_response_handler::MakeNotAllowResponse(request.version(), request.keep_alive(), 
                                                        "Only GET or HEAD methods is expected", http::verb::get);
        std::string response_body;
        json::array maps;
        for (const auto& map : application_.ListMaps()) {
            maps.push_back({ { json_loader::JsonConfigNames::MAP_ID, *map.GetId() },
                             { json_loader::JsonConfigNames::MAP_NAME, map.GetName() } });
        }                
        response_body = json::serialize(maps);
        return http_response_handler::MakeJsonResponse(request.version(), request.keep_alive(), response_body);
    }

    StringResponse ApiHandler::GetMap(const StringRequest& request, const std::string& map_name) const {
        if(request.method() != http::verb::get && request.method() != http::verb::head)
                    return http_response_handler::MakeNotAllowResponse(request.version(), request.keep_alive(), 
                                                        "Only GET or HEAD methods is expected", http::verb::get);
        const model::Map* map = application_.FindMap(map_name);
        if (map) {
            auto items = loot_data_.GetItemsData(map->GetId());
            json::value res = json::value_from(*map);
            res.as_object().emplace(JsonRequestsNames::LOOT_TYPES, items);
            std::string response_body = json::serialize(res);
            return http_response_handler::MakeJsonResponse(request.version(), request.keep_alive(), response_body);
        }      
        return http_response_handler::MakeNotFoundResponse(request.version(), request.keep_alive(), "Map not found");     
    }

    std::optional<app::Token> ApiHandler::TryExtractToken(const StringRequest& request) const {
        if(!request.count(http::field::authorization)) {
            return std::nullopt;
        }
        std::string_view token =  request.at(http::field::authorization);
        if(!token.starts_with(app::PlayerTokens::BEARER) 
            || token.substr(app::PlayerTokens::BEARER.size()).size() != app::PlayerTokens::TOKEN_SIZE){
            return std::nullopt;
        } 
        return app::Token{std::string(token.substr(app::PlayerTokens::BEARER.size()))};
    }

    StringResponse ApiHandler::GetJoinGame(const StringRequest& request) {
        try{
            if(request.method() != http::verb::post)
                return http_response_handler::MakeNotAllowResponse(request.version(), request.keep_alive(), 
                                                    "Only POST method is expected", http::verb::post);
            beast::error_code ec;
            json::value json_body = json::parse(request.body(), ec);
            if(ec || json_body.as_object().empty() 
            || !json_body.as_object().count(JsonRequestsNames::USER_NAME) || 
               !json_body.as_object().count(JsonRequestsNames::MAP_ID))
                return http_response_handler::MakeInvalidArgumentResponse(request.version(), request.keep_alive(), 
                                                    ErrorResponseType::INVALID_ARGUMENT, "Join game request parse error");

            std::string map_id = std::string(json_body.as_object().at(JsonRequestsNames::MAP_ID).as_string());
            std::string user_name = std::string(json_body.as_object().at(JsonRequestsNames::USER_NAME).as_string());
            
            app::JoinGameUseCase::JoinGameResult result = application_.JoinGame(map_id, user_name);

            json::value json_result = {{JsonRequestsNames::AUTH_TOKEN, *result.token_}, 
                                       {JsonRequestsNames::PLAYER_ID, result.user_id}};
            std::string response_body = json::serialize(json_result);
            return http_response_handler::MakeJsonResponse(request.version(), request.keep_alive(), response_body);
        }
        catch(const app::ApiError& error){
            if(error == app::ApiError::InvalidName) 
                return http_response_handler::MakeInvalidArgumentResponse(request.version(), request.keep_alive(), 
                                                    ErrorResponseType::INVALID_ARGUMENT, "Invalid name");   
            return http_response_handler::MakeNotFoundResponse(request.version(), request.keep_alive(), "Map not found");         
        }
    }

    StringResponse ApiHandler::GetPlayers(const StringRequest& request) const {
        if(request.method() != http::verb::get && request.method() != http::verb::head)
                    return http_response_handler::MakeNotAllowResponse(request.version(), request.keep_alive(), 
                                                        "Only GET or HEAD methods is expected", http::verb::get);
        return ExecuteAuthorized(request, [this, &request](const app::Token& token){
            try{
                json::object players;
                for(const auto& dog : application_.ListPlayers(token)){
                    json::value val = {{JsonRequestsNames::DOG_NAME, dog.second->GetDogName()}};
                    players.emplace(std::to_string(dog.second->GetDogId()), val);
                }
                std::string response_body = json::serialize(players);
                return http_response_handler::MakeJsonResponse(request.version(), request.keep_alive(), response_body);
            }
            catch(const app::ApiError& error){
                return http_response_handler::MakeUnauthorizedResponse(request.version(), request.keep_alive(), 
                                                ErrorResponseType::UNKNOWN_TOKEN, "Player token has not been found");
            }
        });
    }

    StringResponse ApiHandler::GetGameState(const StringRequest& request) const {
        if(request.method() != http::verb::get && request.method() != http::verb::head)
                    return http_response_handler::MakeNotAllowResponse(request.version(), request.keep_alive(), 
                                                        "Only GET or HEAD methods is expected", http::verb::get);  

        return ExecuteAuthorized(request, [this, &request](const app::Token& token) {
            try{
                const auto game_state = this->application_.GameState(token);
                json::object players;
                for(const auto& dog : game_state.dogs) {
                    json::array bag;
                    for(const auto& item : dog.second->GetBagContent()){
                        bag.push_back({{JsonRequestsNames::ITEM_ID, item.id},
                                       {JsonRequestsNames::ITEM_TYPE, item.type_item}});
                    }
                    json::value dog_info = {{JsonRequestsNames::DOG_POS, {dog.second->GetPosition().x, dog.second->GetPosition().y}}, 
                                            {JsonRequestsNames::DOG_SPD, {dog.second->GetSpeed().w, dog.second->GetSpeed().h}},
                                            {JsonRequestsNames::DOG_DIR, dog.second->GetDirection()},
                                            {JsonRequestsNames::DOG_BAG, bag},
                                            {JsonRequestsNames::DOG_SCORE, dog.second->GetScore()}};
                    players.emplace(std::to_string(dog.second->GetDogId()), dog_info);
                }
                json::object lost_objects;
                for(auto& lost_obj : game_state.lost_objects) {
                    json::value obj_info = {{JsonRequestsNames::LOST_OBJ_TYPE, lost_obj.type_loot}, 
                                            {JsonRequestsNames::LOST_OBJ_POS, {lost_obj.pos.x, lost_obj.pos.y}}};
                    lost_objects.emplace(std::to_string(lost_obj.id), obj_info);
                }

                json::object result;
                result.emplace(JsonRequestsNames::PLAYERS, players);
                result.emplace(JsonRequestsNames::LOST_OBJ, lost_objects);

                std::string response_body = json::serialize(result);
                return http_response_handler::MakeJsonResponse(request.version(), request.keep_alive(), response_body);
            }
            catch(const app::ApiError& error){               
                return http_response_handler::MakeUnauthorizedResponse(request.version(), request.keep_alive(), 
                                            ErrorResponseType::UNKNOWN_TOKEN, "Player token has not been found");
            }
        });
    } 

    StringResponse ApiHandler::GetPlayerAction(const StringRequest& request) {
        if(request.method() != http::verb::post)
                    return http_response_handler::MakeNotAllowResponse(request.version(), request.keep_alive(), 
                                                        "Only POST method is expected", http::verb::post);
        return ExecuteAuthorized(request, [this, &request](const app::Token& token){
            try{ 

                if(!request.count(http::field::content_type) || request.at(http::field::content_type) != ContentType::AP_JSON)
                    return http_response_handler::MakeInvalidArgumentResponse(request.version(), request.keep_alive(), 
                                                        ErrorResponseType::INVALID_ARGUMENT, "Invalid content type");
                beast::error_code ec;
                json::value json_body = json::parse(request.body(), ec); 
                if(ec || !json_body.as_object().count(JsonRequestsNames::DOG_MOVE))
                    return http_response_handler::MakeInvalidArgumentResponse(request.version(), request.keep_alive(), 
                                                        ErrorResponseType::INVALID_ARGUMENT, "Failed to parse action");                
                std::string direction = std::string(json_body.as_object().at(JsonRequestsNames::DOG_MOVE).as_string());
                if(this->IsCorrectDirection(direction))
                    return http_response_handler::MakeInvalidArgumentResponse(request.version(), request.keep_alive(), 
                                                        ErrorResponseType::INVALID_ARGUMENT, "Failed to parse action");                                       
                application_.ActionMove(token, direction);
                std::string response_body = "{}";
                return http_response_handler::MakeJsonResponse(request.version(), request.keep_alive(), response_body);
            }
            catch(const app::ApiError& error){
                return http_response_handler::MakeUnauthorizedResponse(request.version(), request.keep_alive(), 
                                                ErrorResponseType::UNKNOWN_TOKEN, "Player token has not been found");
            }
        });
    }

    StringResponse ApiHandler::GetTick(const StringRequest& request) {
        if(accept_tick_){
            if(request.method() != http::verb::post)
                return http_response_handler::MakeNotAllowResponse(request.version(), request.keep_alive(), 
                                                    "Only POST method is expected", http::verb::post);
            beast::error_code ec;
            json::value val = json::parse(request.body(), ec); 
            if(ec || !val.as_object().at(JsonRequestsNames::TIME_DELTA).is_int64())
                return http_response_handler::MakeInvalidArgumentResponse(request.version(), request.keep_alive(), 
                                                    ErrorResponseType::INVALID_ARGUMENT, "Failed to parse action");
            int delta = static_cast<int>(val.as_object().at(JsonRequestsNames::TIME_DELTA).as_int64());
            application_.Tick(std::chrono::milliseconds(delta));
            std::string response_body = "{}";
            return http_response_handler::MakeJsonResponse(request.version(), request.keep_alive(), response_body);
        }
        else{
            return http_response_handler::MakeInvalidArgumentResponse(request.version(), request.keep_alive(), 
                                                ErrorResponseType::BAD_REQUEST, "Invalid endpoint");
        }      
    }

    StringResponse ApiHandler::GetRecords(const StringRequest& request) const {
        if(request.method() != http::verb::get && request.method() != http::verb::head)
                return http_response_handler::MakeNotAllowResponse(request.version(), request.keep_alive(), 
                                                        "Only GET or HEAD methods is expected", http::verb::get);  
        auto params = GetParamsRfomRecordsUrl(request.target());
        constexpr int max_elements = 100;

        if(params.second > max_elements) {
            return http_response_handler::MakeInvalidArgumentResponse(request.version(), request.keep_alive(), 
                                                                       ErrorResponseType::BAD_REQUEST, "max element cannot exceed 100");
        }
            
        const auto records = application_.Records(params.first, params.second);
        json::array data(records.size());
        auto into_json = [](const auto& record){
             return json::object{{JsonRequestsNames::RECORD_NAME, record.GetName()}, 
                                {JsonRequestsNames::RECORD_SCORE, record.GetScore()}, 
                                {JsonRequestsNames::RECORD_PLAY_TIME, record.GetPlayTime() * model::ConvertValues::MS_TO_S}};
        };
        std::transform(records.begin(), records.end(), data.begin(), into_json);

        std::string response_body = json::serialize(data);
        return http_response_handler::MakeJsonResponse(request.version(), request.keep_alive(), response_body); 
    }

    StringResponse ApiHandler::HandlerApiRequest(const StringRequest& req) {
        std::string_view uri_str = req.target();
        http::status status = http::status::ok;
        std::string response_body;
        uri_str.remove_prefix(std::min(uri_str.size(), uri_str.find_first_of("/", 1)));
        if (uri_str.starts_with(URIEndpoints::ENDPOINT_GAME)) {
            uri_str.remove_prefix(URIEndpoints::ENDPOINT_GAME.size());   
            if(uri_str.starts_with(URIEndpoints::ENDPOINT_JOIN)){
                return GetJoinGame(req);           
            }
            if(uri_str.starts_with(URIEndpoints::ENDPOINT_PLAYERS)){
                return GetPlayers(req);                
            }
            if(uri_str.starts_with(URIEndpoints::ENDPOINT_STATE)){
                return GetGameState(req);
            }
            if(uri_str.starts_with(URIEndpoints::ENDPOINT_ACTION)){
                return GetPlayerAction(req);              
            }
            if(uri_str.starts_with(URIEndpoints::ENDPOINT_TICKS)){
                return GetTick(req);
            }     
            if(uri_str.starts_with(URIEndpoints::ENDPOINT_RECORDS)){ 
                return GetRecords(req);
            }
        } else if(uri_str.starts_with(URIEndpoints::ENDPOINT_MAPS)) {
            uri_str.remove_prefix(URIEndpoints::ENDPOINT_MAPS.size());
            if (uri_str.empty() || uri_str == "/"sv) {
                return GetMaps(req);
            }
            std::string map_name = std::string(uri_str.substr(1));
            return GetMap(req, map_name);
        }
        return http_response_handler::MakeInvalidArgumentResponse(req.version(), req.keep_alive(), 
                                            ErrorResponseType::BAD_REQUEST, "Invalid endpoint");
    }

    RequestHandler::RequestHandler(const std::filesystem::path& root, Strand& api_strand, 
                            app::Application& app, bool accept_tick, extra_data::LootJsonData& loot_data)
        : root_{std::move(root)}
        , api_strand_(api_strand)
        , api_handler_(app, accept_tick, loot_data)
    {}

    RequestHandler::FileRequestResult RequestHandler::HandleFileRequest(const StringRequest& req) const {
        http::status status = http::status::ok;
        std::string response_body;
        std::string_view uri_str = req.target();
        std::string uri_decoding = URLEncoding(uri_str);
        if (uri_decoding.empty() || uri_decoding == "/"s) {
            uri_decoding = "/index.html"s;
        }
        std::filesystem::path abs_path = root_ / uri_decoding.substr(1);
        http_response_handler::StringResponseHandler string_response;

        if (IsSubPath(abs_path, root_)) {
            std::string content_type;
            http::file_body::value_type file;
            if (sys::error_code ec; file.open(abs_path.string().data(), beast::file_mode::read, ec), ec) {
                status = http::status::not_found;
                response_body = "file in static directory not found"s;
                return http_response_handler::MakeErrorFileResponse(status, req.version(), req.keep_alive(), response_body);
            }
            content_type = DefineContentType(abs_path.extension().string());
            return http_response_handler::MakeFileResponse(status, std::move(file), req.version(), req.keep_alive(), content_type);
        }
        status = http::status::bad_request;
        response_body = "path turn outside root directory"s; 
        return http_response_handler::MakeErrorFileResponse(status, req.version(), req.keep_alive(), response_body);       
    }

    StringResponse RequestHandler::ReportServerError(unsigned version, bool keep_alive) const {
        StringResponse response;
        response.version(version);
        response.keep_alive(keep_alive);
        response.result(http::status::unknown);
        response.set(http::field::content_type, ContentType::AP_JSON);
        response.set(http::field::cache_control, "no-cache"); 
        json::value val = {{"code", "unknownError"},{"message", "throw unknown error"}};
        response.body() = json::serialize(val);
        return response;
    }
}  // namespace http_handler
