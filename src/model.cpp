#include "model.h"

#include <stdexcept>

namespace model {

    using namespace std::literals;

    Road::Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_{ start }
        , end_{ end_x, start.y } {
    }

    Road::Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{ start }
        , end_{ start.x, end_y } {
    }

    bool Road::IsHorizontal() const noexcept {
        return start_.y == end_.y;
    }

    bool Road::IsVertical() const noexcept {
        return start_.x == end_.x;
    }

    const Point& Road::GetStart() const noexcept {
        return start_;
    }

    const Point& Road::GetEnd() const noexcept {
        return end_;
    }

    Building::Building(Rectangle bounds) noexcept
        : bounds_{ bounds } {
    }

    const Rectangle& Building::GetBounds() const noexcept {
        return bounds_;
    }

    Office::Office(Id id, Point position, Offset offset) noexcept
        : id_{ std::move(id) }
        , position_{ position }
        , offset_{ offset } 
    {}

    const Office::Id& Office::GetId() const noexcept {
        return id_;
    }

    const Point& Office::GetPosition() const noexcept {
        return position_;
    }

    const Offset& Office::GetOffset() const noexcept {
        return offset_;
    }

    Map::Map(Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const Map::Id& Map::GetId() const noexcept {
        return id_;
    }

    const std::string& Map::GetName() const noexcept {
        return name_;
    }

    const Map::Buildings& Map::GetBuildings() const noexcept {
        return buildings_;
    }

    const Map::Roads& Map::GetRoads() const noexcept {
        return roads_;
    }

    const Map::Offices& Map::GetOffices() const noexcept {
        return offices_;
    }

    const double Map::GetSpeed() const noexcept {
        return speed_on_map_;
    }

    const int Map::GetTypeItemCount() const noexcept {
        return type_loot_count_;
    }

    const int Map::GetBagCapacity() const noexcept {
        return bag_capacity_on_map_;
    }

    void Map::AddRoad(const Road& road) {
        roads_.emplace_back(road);
    }

    void Map::AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void Map::AddOffice(const Office& office) {
        if (warehouse_id_to_index_.contains(office.GetId())) {
            throw std::invalid_argument("Duplicate warehouse");
        }
        const size_t index = offices_.size();
        Office& o = offices_.emplace_back(office);
        try {
            warehouse_id_to_index_.emplace(o.GetId(), index);
        } catch (const std::exception& e) {
            offices_.pop_back();
            throw e;
        }
    }

    void Map::SetSpeed(double speed) {
        if(speed_on_map_ == 1.0) {
            speed_on_map_ = speed;
        }
    }

    void Map::SetBagCapacity(int capacity) {
        if(bag_capacity_on_map_ == 3){
            bag_capacity_on_map_ = capacity;
        }
    }

    void Map::SetScoreForLoot(int value) {
        value_loots_.push_back(value);
    }

    const Map::ValueLoots& Map::GetValueLoots() const noexcept {
        return value_loots_;
    }

    TimeChanger::TimeChanger(std::chrono::milliseconds time)
            : accept_inaction_time_(time)
            , all_time_(0ms)
            , inaction_time_(0ms) 
    {}           

    std::optional<std::chrono::milliseconds> TimeChanger::TimerChange(std::chrono::milliseconds delta, bool inaction) {
        all_time_+= delta;
        if(inaction){
            inaction_time_ += delta;
            if(inaction_time_ >= accept_inaction_time_){
                return all_time_;
            }
        }
        else {
            inaction_time_ = 0ms;
        }      
        return std::nullopt;
    }

    Dog::Dog(int id, const std::string& nickname, const Position& pos)
        : id_(id)
        , nickname_(nickname)
        , pos_(pos)
        , speed_{0, 0}
    {}

    const std::string& Dog::GetDogName() const {
        return nickname_;
    }

    int Dog::GetDogId() const {
        return id_;
    }

    const Speed& Dog::GetSpeed() const {
        return speed_;
    }

    const int Dog::GetBagCapacity() const {
        return bag_capacity_;
    }

    const Position& Dog::GetPosition() const {
        return pos_;
    }

    const std::string& Dog::GetDirection() const {
        return direction_;
    }

    const Dog::BagContent& Dog::GetBagContent() const {
        return bag_content_;
    }
    const int Dog::GetScore() const {
        return score_;
    }

    void Dog::SetSpeed(double speed) {
        speed_value_ = speed;
    }

    double Dog::GetSpeedValue() const {
        return speed_value_;
    }
    
    void Dog::SetBagCapacity(int capacity) {
        bag_capacity_ = capacity;
    }

    void Dog::AddScore(int score) {
        score_ += score;
    }
    
    void Dog::ChangeDirection(const std::string& dir) {
        if(listener_){
            listener_->TimerChange(0ms, false);
        }
        if(dir == Direction::NORTH){
            direction_ = dir;
            speed_ = {0, -1.0 * speed_value_};
        }
        if(dir == Direction::SOUTH){
            direction_ = dir;
            speed_ = {0, speed_value_};
        }
        if(dir == Direction::WEST){
            direction_ = dir;
            speed_ = {-1.0 * speed_value_, 0};
        }
        if(dir == Direction::EAST){
            direction_ = dir;
            speed_ = {speed_value_, 0};
        }
        if(dir == ""){
            speed_ = {0, 0};
        }
    }

    void Dog::StopMove() {
        speed_.w = 0;
        speed_.h = 0;
    }

    void Dog::ChangePosition(const Position& pos) {
        pos_ = pos; 
    }

    bool Dog::PutInBag(const FindItem& item) {
        if(bag_content_.size() < bag_capacity_){
            bag_content_.push_back(item);
            return true;
        }
        return false;
    }

    void Dog::ReturnBagContents() {
        bag_content_.clear();
    }

    void Dog::SetListener(std::chrono::milliseconds time) {
        listener_ = std::make_shared<TimeChanger>(time);
    }

    std::optional<std::chrono::milliseconds> Dog::InActiveDog(std::chrono::milliseconds delta) {
        if(listener_){
            return listener_->TimerChange(delta, DogIsStop());
        }
        return std::nullopt;
    }

    bool Dog::DogIsStop() {
        return speed_.h == 0 && speed_.w == 0;
    }

    GameSession::GameSession(const Map* map, LootGenData data) 
        : map_(map) 
        , loot_gen_{std::chrono::milliseconds(static_cast<int>(data.period * ConvertValues::S_TO_MS)), data.probability}
    {}
  
    void GameSession::SetRandom() {
        random_points_ = true;
    }

    void GameSession::SetDogRetirementTime(std::chrono::milliseconds time) {
        retirement_time_ = time;
    }

    int GameSession::GenerateRandomValue(int max_value) {
        std::random_device random_device;
        std::default_random_engine generator(random_device());
        std::uniform_int_distribution<int> dist_roads(0, max_value); 
        return dist_roads(generator);
    }

    void GameSession::GenerateNewLoot(std::chrono::milliseconds interval, int item_count) {
        unsigned new_lost_objects = loot_gen_.Generate(interval, lost_objects_.size(), dogs_.size());
        for(unsigned i = 0; i < new_lost_objects; i++) {
            Position loot_pos = GenerateRandomPosition();
            int loot_type = GenerateRandomValue(item_count - 1);
            lost_objects_.emplace_back(LootData{loot_count_++, loot_type, loot_pos}); 
        }
    }

    Position GameSession::GenerateRandomPosition() {      
        auto roads = map_->GetRoads();
        int num_road = GenerateRandomValue(roads.size() - 1);

        std::random_device random_device;
        std::default_random_engine generator(random_device());
        model::Point start = roads[num_road].GetStart();
        model::Point end = roads[num_road].GetEnd();       
        std::uniform_real_distribution<double> dist_x(std::min(start.x, end.x), std::max(start.x, end.x)); 
        std::uniform_real_distribution<double> dist_y(std::min(start.y, end.y), std::max(start.y, end.y)); 
        double x = dist_x(generator);
        double y = dist_y(generator);
        return {x, y};
    }

    std::shared_ptr<Dog> GameSession::AddDog(const std::string& user_name) {
        auto roads = map_->GetRoads();        
        Position point_begin;
        if(random_points_){
            point_begin = GenerateRandomPosition();
        }
        else{
            point_begin.x = static_cast<double>(roads.at(0).GetStart().x);
            point_begin.y = static_cast<double>(roads.at(0).GetStart().y);
        }
        auto dog = dogs_.emplace(id_count, std::make_shared<Dog>(Dog{id_count, user_name, point_begin}));
        dog.first->second->SetBagCapacity(map_->GetBagCapacity());
        dog.first->second->SetSpeed(map_->GetSpeed());

        dog.first->second->SetListener(retirement_time_);

        ++id_count;
        return dog.first->second;
    }

    void GameSession::AddExistDog(std::shared_ptr<Dog> dog) {
        auto add_dog = dogs_.emplace(dog->GetDogId(), dog);
        add_dog.first->second->SetListener(retirement_time_); 
        if(id_count <= dog->GetDogId()){
            id_count = dog->GetDogId() + 1;
        }
    }

    void GameSession::AddLootData(LootData loot) {
        lost_objects_.push_back(loot);
        if(loot_count_<= loot.id){
            loot_count_ = loot.id + 1;
        }
    }

    ToRetiredDogInfo GameSession::DeleteDog(int dog_id, std::chrono::milliseconds time) {
        auto dog = FindDog(dog_id);
        ToRetiredDogInfo retired_dog{dog->GetDogName(), dog->GetScore(), static_cast<int>(time.count())};
        if(listener_){
            listener_->RetirementDog(dog, map_->GetId(), time);
        }   
        dogs_.erase(dog_id);  
        return retired_dog;   
    }

    void GameSession::SetListener(std::shared_ptr<SessionListener> listener) {
        listener_ = listener;
    }

    bool GameSession::HasListener() const  { 
        return listener_.get();
    }

    std::shared_ptr<Dog> GameSession::FindDog(int dog_id) {
        return dogs_.at(dog_id);
    }

    const Map::Id& GameSession::GetMapId() const { 
        return map_->GetId();
    }

    GameSession::Dogs& GameSession::GetInfoDogs() {
        return dogs_;
    }

    GameSession::LostObjects& GameSession::GetLostObjects() {
        return lost_objects_;
    }

    void GameSession::ExchangeItemForScore(int dog_id) {
        auto dog = dogs_.at(dog_id);
        auto& values = map_->GetValueLoots();
        auto& items = dog->GetBagContent();

        auto item_to_score = [&values](int a, Dog::FindItem item){
                return std::move(a) + values.at(item.type_item);
            };
        int score = std::accumulate(items.begin(), items.end(), 0, item_to_score);
        dog->AddScore(score);
        dog->ReturnBagContents();
    }

    void GameSession::RemoveCollectedItems(const std::set<int>& items_for_delete) {
        auto rbegin = items_for_delete.rbegin();
        auto rend = items_for_delete.rend();
        for(auto it = rbegin; it != rend; it++){
            lost_objects_.erase(lost_objects_.begin() + *it);
        }
    }

    void Game::AddMap(const Map& map, double speed, int capacity) {
        const size_t index = maps_.size();
        if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
            throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
        } else {
            try {
                maps_.emplace_back(map);
                maps_.back().SetSpeed(speed);
                maps_.back().SetBagCapacity(capacity);
            } catch (const std::exception& e) {
                map_id_to_index_.erase(it);
                throw e;
            }
        }
    }

    std::shared_ptr<GameSession> Game::FindSession(const Map::Id& map_id) {
        if(sessions_.count(map_id)){
            return sessions_.at(map_id); 
        }
        else { 
            if(auto* map = FindMap(map_id)) {
                auto session = sessions_.emplace(map_id, std::make_shared<GameSession>(GameSession{map, gen_data_}));
                if(random_points_){
                    session.first->second->SetRandom();
                }
                session.first->second->SetDogRetirementTime(retirement_time_);
                return session.first->second;
            }
            return nullptr;
        } 
    }

    const Game::GameSessions& Game::GetSessions() const {
        return sessions_;
    }

    const Game::Maps& Game::GetMaps() const noexcept {
        return maps_;
    }

    const Map* Game::FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

    void Game::SetRandomaizer() {
        random_points_ = true;
    }

    void Game::SetLootGenData(double period, double probability) {
        gen_data_.period = period;
        gen_data_.probability = probability;
    }

    void Game::SetDogRetirementTime(double time_s) {
        int ret_time_ms = time_s * ConvertValues::S_TO_MS;
        retirement_time_ = std::chrono::milliseconds(ret_time_ms );
    }

    const LootGenData& Game::GetLootGenData() const {
        return gen_data_;
    }

}  // namespace model
