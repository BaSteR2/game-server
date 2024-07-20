#pragma once
#include <string_view>

namespace storage_literals {

using namespace std::literals;

    struct ContentType {
        ContentType() = delete;
        constexpr static std::string_view TEXT_HTML = "text/html"sv; //.htm, .html
        constexpr static std::string_view AP_JSON = "application/json"sv; //.json
        constexpr static std::string_view TEXT_CSS = "text/css"sv; // .css
        constexpr static std::string_view TEXT_PL = "text/plain"sv; // .txt
        constexpr static std::string_view TEXT_JS = "text/javascript"sv; // .js
        constexpr static std::string_view APP_XML = "application/xml"sv; // .xml
        constexpr static std::string_view IMG_PNG = "image/png"sv; // .png
        constexpr static std::string_view IMG_JPG = "image/jpeg"sv; // .jpg, .jpe, .jpeg
        constexpr static std::string_view IMG_GIF = "image/gif"sv; // .gif
        constexpr static std::string_view IMG_BMP = "image/bmp"sv; // .bmp
        constexpr static std::string_view IMG_ICO = "image/vnd.microsoft.icon"sv; // .ico
        constexpr static std::string_view IMG_TIFF = "image/tiff"sv; // .tiff .tif
        constexpr static std::string_view IMG_SVG = "image/svg+xml"sv; // .svg, .svgz
        constexpr static std::string_view IMG_MPEG = "audio/mpeg"sv; // .mp3
        constexpr static std::string_view APP_EMPT = "application/octet-stream"sv; // empty and unknown
    };

    struct URIEndpoints {
        URIEndpoints() = delete;
        constexpr static std::string_view ENDPOINT_API = "/api"sv;
        constexpr static std::string_view ENDPOINT_MAPS = "/v1/maps"sv;
        constexpr static std::string_view ENDPOINT_GAME = "/v1/game"sv;
        constexpr static std::string_view ENDPOINT_JOIN = "/join"sv;
        constexpr static std::string_view ENDPOINT_PLAYERS = "/players"sv;
        constexpr static std::string_view ENDPOINT_STATE = "/state"sv;
        constexpr static std::string_view ENDPOINT_ACTION = "/player/action"sv;
        constexpr static std::string_view ENDPOINT_TICKS = "/tick"sv;
        constexpr static std::string_view ENDPOINT_RECORDS = "/records"sv;
    };

    struct ErrorResponseType {
        ErrorResponseType() = delete;
        constexpr static std::string_view MAP_NOT_FOUND = "mapNotFound"sv;
        constexpr static std::string_view BAD_REQUEST = "badRequest"sv;
        constexpr static std::string_view INVALID_ARGUMENT = "invalidArgument"sv;
        constexpr static std::string_view UNKNOWN_TOKEN = "unknownToken"sv;
        constexpr static std::string_view INVALID_TOKEN = "invalidToken"sv;
        constexpr static std::string_view INVALID_METHOD = "invalidMethod"sv;        
    };

    struct JsonRequestsNames {
        JsonRequestsNames() = delete;
        constexpr static const char* LOOT_TYPES = "lootTypes";
		constexpr static const char* MAP_ID = "mapId";
		constexpr static const char* USER_NAME = "userName";
		constexpr static const char* AUTH_TOKEN = "authToken";
		constexpr static const char* PLAYER_ID = "playerId";
        constexpr static const char* PLAYERS = "players";

        constexpr static const char* DOG_NAME = "name";
        constexpr static const char* DOG_POS = "pos";
        constexpr static const char* DOG_DIR = "dir";
        constexpr static const char* DOG_SPD = "speed";
        constexpr static const char* DOG_SCORE = "score";
        constexpr static const char* DOG_BAG = "bag";
        constexpr static const char* DOG_MOVE = "move";      
        constexpr static const char* ITEM_ID = "id";
        constexpr static const char* ITEM_TYPE = "type";


        constexpr static const char* LOST_OBJ = "lostObjects";
        constexpr static const char* LOST_OBJ_TYPE = "type";
        constexpr static const char* LOST_OBJ_POS = "pos";

        constexpr static const char* TIME_DELTA = "timeDelta";

        constexpr static const char* RECORD_NAME = "name";
        constexpr static const char* RECORD_SCORE = "score";
        constexpr static const char* RECORD_PLAY_TIME = "playTime";
    };

}