#include "app.h"

namespace app {

    Player::Player(std::shared_ptr<model::GameSession> session, std::shared_ptr<model::Dog> dog)
        : session_(session)
        , dog_(dog)
    {} 

    int Player::GetId() const {
        return dog_->GetDogId();
    }

    std::shared_ptr<model::Dog> Player::GetDog() const {
        return dog_;
    }

    std::shared_ptr<model::GameSession> Player::GetSession() const {
        return session_;
    }

    std::shared_ptr<Player> PlayerTokens::FindPlayerByToken(const Token& token) const {
        if(token_to_player_.count(token)){
            return token_to_player_.at(token);
        }
        return nullptr;
    } 

    Token PlayerTokens::CreateRandomToken() {
        std::default_random_engine generator1(random_device_());
        std::default_random_engine generator2(random_device_());
        std::uniform_int_distribution<uint64_t> distribution(0x1000000000000000,0xFFFFFFFFFFFFFFFF); 
        std::stringstream str;
        str << std::hex << distribution(generator1) <<  distribution(generator2);
        str.width(TOKEN_SIZE);
        return Token{str.str()};
    }

    const Token& PlayerTokens::AddPlayer(std::shared_ptr<Player> player) {
        Token token = CreateRandomToken();
        auto val = token_to_player_.emplace(token, player);
        player_to_token_.emplace(std::pair{val.first->second->GetId(), *val.first->second->GetSession()->GetMapId()}, 
                                                                                                    &val.first->first);    
        return val.first->first;
    }

    void PlayerTokens::AddPlayerWithToken(std::shared_ptr<Player> player, const Token& token) {
        auto val = token_to_player_.emplace(std::pair{token, player});
        player_to_token_.emplace(std::pair{val.first->second->GetId(), *val.first->second->GetSession()->GetMapId()}, 
                                                                                                    &val.first->first);
    }

    const Token* PlayerTokens::FindToken(std::shared_ptr<Player> player) const {
        if(player_to_token_.count({player->GetId(), *player->GetSession()->GetMapId()})){
            return player_to_token_.at({player->GetId(), *player->GetSession()->GetMapId()});
        }
        return nullptr;
    }

    void PlayerTokens::DeletePlayer(int dog_id, const model::Map::Id& map_id) {
        if(player_to_token_.count({dog_id, *map_id})){
            auto token = *player_to_token_.at({dog_id, *map_id});
            player_to_token_.erase({dog_id, *map_id});
            if(token_to_player_.count(token)){
                token_to_player_.erase(token);
            }
        }
    }

    std::shared_ptr<Player> Players::AddPlayer(std::shared_ptr<model::Dog> dog, std::shared_ptr<model::GameSession> session) {
        players_.emplace(std::pair{dog->GetDogId(), *session->GetMapId()}, std::make_shared<Player>(Player{session, dog}));
        return players_.at(std::pair{dog->GetDogId(), *session->GetMapId()}); 
    }

    std::shared_ptr<Player> Players::FindByDogidAndMapid(int dog_id, const model::Map::Id& map_id) const {
        if(players_.count({dog_id, *map_id})){
            return players_.at({dog_id, *map_id});;
        }
        return nullptr;
    }

    void Players::DeletePlayer(int dog_id, const model::Map::Id& map_id) {
        if(players_.count({dog_id, *map_id})){
            players_.erase({dog_id, *map_id});
        }
    }

    PlayerListener::PlayerListener(app::Players& players, app::PlayerTokens& tokens) 
            : players_(players)
            , tokens_(tokens)
    {}

    void PlayerListener::RetirementDog(std::shared_ptr<model::Dog> dog, const model::Map::Id& map_id, std::chrono::milliseconds time) {
        tokens_.DeletePlayer(dog->GetDogId(), map_id);
        players_.DeletePlayer(dog->GetDogId(), map_id);
    }

    LostObjDogProvider::LostObjDogProvider(const std::vector<collision_detector::Item>& items,
                                const std::vector<collision_detector::Gatherer>& gatherers)
        : items_(items)
        , gatherers_(gatherers) 
    {}
     
    size_t LostObjDogProvider::ItemsCount() const  {
        return items_.size();
    }

    collision_detector::Item LostObjDogProvider::GetItem(size_t idx) const  {
        return items_.at(idx);
    }

    size_t LostObjDogProvider::GatherersCount() const {
        return gatherers_.size();
    }

    collision_detector::Gatherer LostObjDogProvider::GetGatherer(size_t idx) const  {
        return gatherers_.at(idx);
    }

    ListMapsUseCase::ListMapsUseCase(const model::Game& game)
        : game_(&game)
    {}

    const model::Game::Maps& ListMapsUseCase::List() const {
        return game_->GetMaps();
    }

    FindMapUseCase::FindMapUseCase(const model::Game& game)
        : game_(&game)
    {}    

    const model::Map* FindMapUseCase::Find(const std::string& map_id) const {       
        return game_->FindMap(model::Map::Id{map_id});
    }

    JoinGameUseCase::JoinGameUseCase(model::Game& game, Players& players, PlayerTokens& tokens)
        : game_(&game)
        , players_(&players)
        , tokens_(&tokens)
    {}

    JoinGameUseCase::JoinGameResult JoinGameUseCase::Join(const std::string& map_id, const std::string& name) {
        if(name.empty()){ 
            throw ApiError::InvalidName;
        }
        if(auto session = game_->FindSession(model::Map::Id{map_id})){
            if(!session->HasListener()){
                session->SetListener(std::make_shared<PlayerListener>(*players_, *tokens_));
            }
            auto player = players_->AddPlayer(session->AddDog(name), session);
            const auto& token = tokens_->AddPlayer(player);
            return {token, player->GetId()};
        }
        throw ApiError::MapNotFound; 
    }

    ListPlayersUseCase::ListPlayersUseCase(const model::Game& game, const Players& players, const PlayerTokens& tokens) 
        : game_(&game)
        , players_(&players)
        , tokens_(&tokens)
    {}

    const model::GameSession::Dogs& ListPlayersUseCase::List(const Token& token) const {
        auto player = tokens_->FindPlayerByToken(token);
        if(player){
            return player->GetSession()->GetInfoDogs();
        }
        throw ApiError::TokenUnknown; 
    }

    GetStateUseCase::GetStateUseCase(const model::Game& game, const Players& players, const PlayerTokens& tokens)
        : game_(&game)
        , players_(&players)
        , tokens_(&tokens)
    {}

    const GetStateUseCase::GameStateResult GetStateUseCase::State(const Token& token) const {
        auto player = tokens_->FindPlayerByToken(token);
        if(player){
            return {player->GetSession()->GetInfoDogs(), player->GetSession()->GetLostObjects()};
        }
        throw ApiError::TokenUnknown; 
    }

    ActionMoveUseCase::ActionMoveUseCase(model::Game& game, Players& players, PlayerTokens& tokens)
        : game_(&game)
        , players_(&players)
        , tokens_(&tokens)
    {}
    
    void ActionMoveUseCase::Move(const Token& token, const std::string& dir){
        auto player = tokens_->FindPlayerByToken(token);
        if(player){
            player->GetDog()->ChangeDirection(dir); 
        }
        else{
            throw ApiError::TokenUnknown;   
        }            
    }

    TickUseCase::TickUseCase(model::Game& game, postgres::DataBase& game_db)
        : game_(&game)
        , game_db_(game_db)
    {}

    bool TickUseCase::IsPosBelongToRoad(const model::Position& pos, const model::FieldRoad& field_road) {
        constexpr double eps = 1e-10;
        return (field_road.up_left.x < pos.x || std::abs(pos.x - field_road.up_left.x) < eps) && 
               (pos.x < field_road.down_right.x || std::abs(field_road.down_right.x - pos.x) < eps) &&
               (field_road.up_left.y < pos.y || std::abs(pos.y - field_road.up_left.y) < eps) && 
               (pos.y < field_road.down_right.y || std::abs(field_road.down_right.y - pos.y) < eps);
    }

    std::vector<model::FieldRoad> TickUseCase::GetContainRoads(const model::Map map, const model::Position& pos) {
        std::vector<model::FieldRoad> contain_roads;
        for(const auto& road : map.GetRoads()){                      
            const model::Point& start = road.GetStart();
            const model::Point& end = road.GetEnd();
            model::FieldRoad field_road{{std::min(start.x, end.x) - model::ObjectsWidth::ROAD_WIDTH, 
                                            std::min(start.y, end.y) - model::ObjectsWidth::ROAD_WIDTH}, 
                                        {std::max(start.x, end.x) + model::ObjectsWidth::ROAD_WIDTH, 
                                            std::max(start.y, end.y) + model::ObjectsWidth::ROAD_WIDTH}};
            if(IsPosBelongToRoad(pos, field_road)){ 
                contain_roads.push_back(field_road);
            }                       
        }
        return contain_roads;
    }

    double TickUseCase::FindNearestPoint(double axis_point_l, double  axis_point_b, double mid_point, double new_point) {       
        if(std::abs(new_point - axis_point_l) < std::abs(new_point - axis_point_b)){
            if(std::abs(new_point - axis_point_l) < std::abs(new_point - mid_point)){
                return axis_point_l;
            }
        }
        else {
            if(std::abs(new_point - axis_point_b) < std::abs(new_point - mid_point)){
                return axis_point_b;
            }
        }
        return mid_point;
    }

    model::Position TickUseCase::MakeMove(std::shared_ptr<model::Dog> dog, const std::vector<model::FieldRoad>& roads, 
                                                                model::Position new_pos, model::Position mid_pos) {
        for(const auto& road : roads){
            if(IsPosBelongToRoad(new_pos, road)){
                dog->ChangePosition(new_pos);
                return new_pos;
            }
            if((road.up_left.x > new_pos.x) || (new_pos.x > road.down_right.x)){
                mid_pos.x = FindNearestPoint(road.up_left.x, road.down_right.x, mid_pos.x, new_pos.x);
            }
            if((road.up_left.y > new_pos.y) || (new_pos.y > road.down_right.y)){
                mid_pos.y = FindNearestPoint(road.up_left.y, road.down_right.y, mid_pos.y, new_pos.y);
            }
        }
        dog->ChangePosition(mid_pos);
        dog->StopMove();
        return mid_pos; 
    }

    std::vector<collision_detector::Gatherer> TickUseCase::MakeGatherersData(const model::Map& map, model::GameSession::Dogs& dogs, 
                                                                                                    std::chrono::milliseconds delta) {
        std::vector<collision_detector::Gatherer> gatherers;
        for(auto& dog : dogs) { 
            model::Speed dog_speed = dog.second->GetSpeed();
            model::Position current_pos = dog.second->GetPosition();
            std::vector<model::FieldRoad> contain_roads = GetContainRoads(map, current_pos);
            model::Position new_pos{{current_pos.x + dog_speed.w * delta.count() * model::ConvertValues::MS_TO_S }, 
                                     current_pos.y + dog_speed.h * delta.count() * model::ConvertValues::MS_TO_S };
            model::Position mid_pos{current_pos.x, current_pos.y};

            auto res_pos = MakeMove(dog.second, contain_roads, new_pos, mid_pos); 

            gatherers.emplace_back(collision_detector::Gatherer{static_cast<size_t>(dog.second->GetDogId()), 
                                            {current_pos.x,current_pos.y},
                                            {res_pos.x,res_pos.y}, 
                                            model::ObjectsWidth::DOG_WIDTH}); 
        }
        return gatherers;
    }

    std::vector<collision_detector::Item> TickUseCase::MakeItemsData(const model::Map& map, const model::GameSession::LostObjects& lost_objects) {
        const auto& offices = map.GetOffices();
        std::vector<collision_detector::Item> items(lost_objects.size() + offices.size());

        auto push_lost_obj = [](const auto& obj){
            return collision_detector::Item{{obj.pos.x, obj.pos.y}, model::ObjectsWidth::LOST_OBJ_WIDTH};
        };
        std::transform(lost_objects.begin(), lost_objects.end(), items.begin(), push_lost_obj);

        auto push_offices = [](const auto& office){
            const auto& pos = office.GetPosition();
            return collision_detector::Item{{static_cast<double>(pos.x), static_cast<double>(pos.y)}, model::ObjectsWidth::OFFICE_WIDTH};
        };
        std::transform(offices.begin(), offices.end(), items.end() - 1, push_offices);

        return items;
    }

    std::set<int> TickUseCase::ExecuteActionEvents(std::vector<collision_detector::GatheringEvent>& dog_events, 
                                                        std::shared_ptr<model::GameSession> session, LostObjDogProvider& prov) {    
        std::set<int> items_for_delete;
        auto& dogs = session->GetInfoDogs();
        auto& lost_objects = session->GetLostObjects();
        for(const auto& event : dog_events){
            if(prov.GetItem(event.item_id).width == 0) { 
                if(!items_for_delete.count(event.item_id)){
                    auto obj = lost_objects[event.item_id];
                     if(dogs[event.gatherer_id]->PutInBag({obj.id, obj.type_loot})){
                        items_for_delete.insert(event.item_id);
                    }
                }
            }
            else { 
                session->ExchangeItemForScore(event.gatherer_id);
            }
        }
        return items_for_delete;
    }

    void TickUseCase::Tick(std::chrono::milliseconds delta) {
        for(const auto& map : game_->GetMaps()){
            auto session = game_->FindSession(map.GetId());
            if(session){
                int item_type_count = map.GetValueLoots().size();
                session->GenerateNewLoot(delta, item_type_count);

                auto& dogs = session->GetInfoDogs();
                std::vector<std::pair<int, std::chrono::milliseconds>> dogs_to_delete;
                for(const auto& dog: dogs){
                    if(auto dele = dog.second->InActiveDog(delta)){
                        dogs_to_delete.push_back({dog.second->GetDogId(), dele.value()});
                    }
                }  
                std::vector<collision_detector::Gatherer> gatherers = MakeGatherersData(map, dogs, delta);
                auto& lost_objects = session->GetLostObjects();
                std::vector<collision_detector::Item> items = MakeItemsData(map, lost_objects);
              
                LostObjDogProvider obj_dogs{items, gatherers};
                auto dog_events = collision_detector::FindGatherEvents(obj_dogs);
                std::set<int> items_for_delete = ExecuteActionEvents(dog_events, session, obj_dogs);
                if(!items_for_delete.empty()) {
                    session->RemoveCollectedItems(items_for_delete);
                }
              
                for(const auto& dog : dogs_to_delete){
                    auto retir_dog = session->DeleteDog(dog.first, dog.second);
                    game_db_.SaveRetiredDog(retir_dog);
                }      
            }
        }
    }

    RecordsUseCase::RecordsUseCase(const postgres::DataBase& game_db)
        : game_db_(game_db)
    {}

    const std::vector<model::RetiredDog> RecordsUseCase::GetRecords(int offset, int max_elements) const {
       return game_db_.GetRetiredDogs(offset, max_elements);
    }

    Application::Application(model::Game& game, const postgres::AppConfig& config)
        : game_(game)
        , game_db_(config) 
        , list_maps_(game)
        , find_map_(game)
        , join_game_(game, players_, tokens_)
        , list_players_(game, players_, tokens_)
        , game_state_(game, players_, tokens_)
        , action_move_(game, players_, tokens_)
        , tick_(game, game_db_)
        , records_(game_db_)
    {}

    void Application::SetListener(ApplicationListener& listener) {
        listener_ = &listener;
    }

    model::Game& Application::GetGame() {
        return game_;
    }

    Players& Application::GetPlayers() {
        return players_;
    }

    PlayerTokens& Application::GetPlayerTokens() {
        return tokens_;
    }

    const model::Game::Maps Application::ListMaps() const {
        return list_maps_.List();
    }

    const model::Map* Application::FindMap(const std::string& map_id) const {
        return find_map_.Find(map_id);
    }

    const JoinGameUseCase::JoinGameResult Application::JoinGame(const std::string& map_id, const std::string& name) {
        try{
            return join_game_.Join(map_id, name);
        }
        catch(const ApiError& error){
            throw error;
        }
    }

    const model::GameSession::Dogs& Application::ListPlayers(const Token& token) const {
        try{
            return list_players_.List(token);
        }
        catch(const ApiError& error){
            throw error;
        }
    }

    const GetStateUseCase::GameStateResult Application::GameState(const Token& token) const {
        try{
            return game_state_.State(token);
        }
        catch(const ApiError& error){
            throw error;
        }
    }

    void Application::ActionMove(const Token& token, const std::string& dir) {
        try{
            action_move_.Move(token, dir);
        }
        catch(const ApiError& error){
            throw error;
        }
    }

    void Application::Tick(std::chrono::milliseconds delta) {
        tick_.Tick(delta);     
        if(listener_){ 
            listener_->OnTick(delta, *this); 
        }
    }

    const std::vector<model::RetiredDog> Application::Records(int offset, int max_elements) const {
        return records_.GetRecords(offset, max_elements);
    }
    
} //namespase app