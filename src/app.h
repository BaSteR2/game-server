#pragma once
#include "model.h"
#include "json_loader.h"
#include "collision_detector.h"
#include "postgres.h"

#include <set>
#include <iostream>
#include <random>
#include <string_view>
#include <pqxx/connection>

namespace detail {
    struct TokenTag {};
}  // namespace detail

namespace app {

    using namespace std::literals;  
 
    class Player {
    public:
        explicit Player(std::shared_ptr<model::GameSession> session, std::shared_ptr<model::Dog> dog);
        int GetId() const;
        std::shared_ptr<model::Dog> GetDog() const;
        std::shared_ptr<model::GameSession> GetSession() const;

    private:
        std::shared_ptr<model::GameSession> session_;
        std::shared_ptr<model::Dog> dog_; 
    };

    using Token = util::Tagged<std::string, detail::TokenTag>;
    struct PlayerHasher {
            size_t operator()(std::pair<int, std::string> dog_map_id) const {                
                return hasher_map_id_(dog_map_id.second) + 37 * hasher_dog_id_(dog_map_id.first);
            }
            std::hash<std::string> hasher_map_id_;
            std::hash<int> hasher_dog_id_;
    };

    class PlayerTokens {
    public:
        constexpr static int TOKEN_SIZE = 32;
        constexpr static std::string_view BEARER = "Bearer "sv;

        std::shared_ptr<Player> FindPlayerByToken(const Token& token) const; 
        const Token& AddPlayer(std::shared_ptr<Player> player);
        void AddPlayerWithToken(std::shared_ptr<Player> player, const Token& token);

        const Token* FindToken(std::shared_ptr<Player> player) const;
        void DeletePlayer(int dog_id, const model::Map::Id& map_id);

    private:    
        Token CreateRandomToken();
        std::random_device random_device_;
        
        std::unordered_map<std::pair<int, std::string>, const Token*, PlayerHasher> player_to_token_;
        std::unordered_map<Token, std::shared_ptr<Player>, util::TaggedHasher<Token>> token_to_player_;
    };

    class Players {
    public:   
        std::shared_ptr<Player> AddPlayer(std::shared_ptr<model::Dog> dog, std::shared_ptr<model::GameSession> session);
        std::shared_ptr<Player> FindByDogidAndMapid(int dog_id, const model::Map::Id& map_id) const; 
        void DeletePlayer(int dog_id, const model::Map::Id& map_id);

    private:
        std::unordered_map<std::pair<int, std::string>, std::shared_ptr<Player>, PlayerHasher> players_; 
    };

    class PlayerListener : public model::SessionListener {
    public:
        explicit PlayerListener(app::Players& players, app::PlayerTokens& tokens);
        void RetirementDog(std::shared_ptr<model::Dog> dog, const model::Map::Id& map_id, std::chrono::milliseconds time) override;
    private:
        app::Players& players_;
        app::PlayerTokens& tokens_;
    };

    class LostObjDogProvider : public collision_detector::ItemGathererProvider {
    public:
        explicit LostObjDogProvider(const std::vector<collision_detector::Item>& items,
                                const std::vector<collision_detector::Gatherer>& gatherers);
     
        size_t ItemsCount() const override;
        collision_detector::Item GetItem(size_t idx) const override;
        size_t GatherersCount() const override;
        collision_detector::Gatherer GetGatherer(size_t idx) const override;
    private:
        std::vector<collision_detector::Item> items_;
        std::vector<collision_detector::Gatherer> gatherers_;
    };

    class ListMapsUseCase {
    public:
        explicit ListMapsUseCase(const model::Game& game);
        const model::Game::Maps& List() const;
    private:
        const model::Game* game_; 
    };

    class FindMapUseCase {
    public:
        explicit FindMapUseCase(const model::Game& game);   
        const model::Map* Find(const std::string& map_id) const ;

    private:
        const model::Game* game_; 
    };

    enum class ApiError {InvalidName, MapNotFound, TokenUnknown};

    class JoinGameUseCase {
    public:
        explicit JoinGameUseCase(model::Game& game, Players& players, PlayerTokens& tokens);
        struct JoinGameResult{
            Token token_;
            int user_id;
        };
        JoinGameResult Join(const std::string& map_id, const std::string& name); 
    private:
        model::Game* game_;
        Players* players_;
        PlayerTokens* tokens_;
    };

    class ListPlayersUseCase {
    public:
        explicit ListPlayersUseCase(const model::Game& game, const Players& players, const PlayerTokens& tokens);
        const model::GameSession::Dogs& List(const Token& token) const;
    private:
        const model::Game* game_;
        const Players* players_;
        const PlayerTokens* tokens_;
    };

    class GetStateUseCase {
    public:
        explicit GetStateUseCase(const model::Game& game, const Players& players, const PlayerTokens& tokens);
        struct GameStateResult {
            const model::GameSession::Dogs& dogs;
            const model::GameSession::LostObjects& lost_objects;
        };
        const GameStateResult State(const Token& token) const;
    private:
        const model::Game* game_;
        const Players* players_;
        const PlayerTokens* tokens_;
    };

    class ActionMoveUseCase {
    public:
        explicit ActionMoveUseCase(model::Game& game, Players& players, PlayerTokens& tokens);
        void Move(const Token& token, const std::string& dir);
    private:
        model::Game* game_;
        Players* players_;
        PlayerTokens* tokens_;
    };

    class TickUseCase {
    public:
        explicit TickUseCase(model::Game& game, postgres::DataBase& game_db);
        void Tick(std::chrono::milliseconds delta);
    private:
        bool IsPosBelongToRoad(const model::Position& pos, const model::FieldRoad& field_road);
        std::vector<model::FieldRoad> GetContainRoads(const model::Map map, const model::Position& pos);
        model::Position MakeMove(std::shared_ptr<model::Dog> dog, const std::vector<model::FieldRoad>& roads, 
                                        model::Position new_pos, model::Position mid_pos);
        double FindNearestPoint(double axis_point_l, double  axis_point_b, double mid_point, double new_point);
        std::vector<collision_detector::Gatherer> MakeGatherersData(const model::Map& map, model::GameSession::Dogs& dogs, 
                                                                                          std::chrono::milliseconds delta);
        static std::vector<collision_detector::Item> MakeItemsData(const model::Map& map, const model::GameSession::LostObjects& lost_objects);

        std::set<int> ExecuteActionEvents(std::vector<collision_detector::GatheringEvent>& dog_events, 
                                                        std::shared_ptr<model::GameSession> session, LostObjDogProvider& prov);

        model::Game* game_;
        postgres::DataBase& game_db_;
    };

    class RecordsUseCase {
    public:
        explicit RecordsUseCase(const postgres::DataBase& game_db);
        const std::vector<model::RetiredDog> GetRecords(int offset, int max_elements) const;
    private:
        const postgres::DataBase& game_db_;
    };

    class Application;
    
    class ApplicationListener {
    public:
        virtual void OnTick(std::chrono::milliseconds delta, app::Application& app) = 0;
    };

    class Application {
    public:
        explicit Application(model::Game& game, const postgres::AppConfig& config);

        void SetListener(ApplicationListener& listener);
        model::Game& GetGame();
        Players& GetPlayers();
        PlayerTokens& GetPlayerTokens();

        const model::Game::Maps ListMaps() const;
        const model::Map* FindMap(const std::string& map_id) const;
        const JoinGameUseCase::JoinGameResult JoinGame(const std::string& map_id, const std::string& name);
        const model::GameSession::Dogs& ListPlayers(const Token& token) const;
        const GetStateUseCase::GameStateResult GameState(const Token& token) const;
        void ActionMove(const Token& token, const std::string& dir);
        void Tick(std::chrono::milliseconds delta);
        const std::vector<model::RetiredDog> Records(int offset, int max_elements) const;

    private:
        model::Game& game_;
        Players players_;
        PlayerTokens tokens_;
        postgres::DataBase game_db_;

        ListMapsUseCase list_maps_;
        FindMapUseCase find_map_;
        JoinGameUseCase join_game_;
        ListPlayersUseCase list_players_;
        GetStateUseCase game_state_;
        ActionMoveUseCase action_move_;
        TickUseCase tick_;
        RecordsUseCase records_;

        ApplicationListener* listener_ = nullptr;
    };

} // namespace app


