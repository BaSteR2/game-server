#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <random>
#include <memory>
#include <set>
#include <optional>
#include <map>
#include "tagged.h"
#include "loot_generator.h"

#include <iostream>

namespace model {

    using Dimension = int;
    using Coord = Dimension;

    struct Point {
        Coord x, y;
    };

    struct Position {
        double x, y;
    };

    struct FieldRoad {
        Position up_left, down_right;
    };

    struct Speed {
        double w, h;
    };

    struct Direction {
        Direction() = delete;
        constexpr static const char* NORTH = "U";
		constexpr static const char* SOUTH = "D";
		constexpr static const char* WEST = "L";
		constexpr static const char* EAST = "R";
    };

    struct ObjectsWidth {
        constexpr static double ROAD_WIDTH = 0.4;
        constexpr static double DOG_WIDTH = 0.3;
        constexpr static double OFFICE_WIDTH = 0.25;
        constexpr static double LOST_OBJ_WIDTH = 0;
    };

    struct ConvertValues {
        constexpr static double MS_TO_S = 0.001;
        constexpr static double S_TO_MS = 1000.0;
    };

    struct Size {
        Dimension width, height;
    };

    struct Rectangle {
        Point position;
        Size size;
    };

    struct Offset {
        Dimension dx, dy;
    };

    struct LootGenData {
        double period, probability;
    };

    struct ToRetiredDogInfo {
        std::string name;
        int score;
        int play_time;
    };

    class Road {
        struct HorizontalTag {
            HorizontalTag() = default;
        };

        struct VerticalTag {
            VerticalTag() = default;
        };

    public:
        constexpr static HorizontalTag HORIZONTAL{};
        constexpr static VerticalTag VERTICAL{};

        explicit Road(HorizontalTag, Point start, Coord end_x) noexcept;
        explicit Road(VerticalTag, Point start, Coord end_y) noexcept;

        bool IsHorizontal() const noexcept;
        bool IsVertical() const noexcept;

        const Point& GetStart() const noexcept;
        const Point& GetEnd() const noexcept;

    private:
        Point start_;
        Point end_;
    };

    class Building {
    public:
        explicit Building(Rectangle bounds) noexcept;
        const Rectangle& GetBounds() const noexcept;
    private:
        Rectangle bounds_;
    };

    class Office {
    public:
        using Id = util::Tagged<std::string, Office>;
        explicit Office(Id id, Point position, Offset offset) noexcept;

        const Id& GetId() const noexcept;
        const Point& GetPosition() const noexcept;
        const Offset& GetOffset() const noexcept;
    private:
        Id id_;
        Point position_;
        Offset offset_;
    };

    class Map {
    public:
        using Id = util::Tagged<std::string, Map>;
        using Roads = std::vector<Road>;
        using Buildings = std::vector<Building>;
        using Offices = std::vector<Office>;
        using ValueLoots = std::vector<int>;

        explicit Map(Id id, std::string name) noexcept;

        const Id& GetId() const noexcept;
        const std::string& GetName() const noexcept;
        const double GetSpeed() const noexcept;
        const Buildings& GetBuildings() const noexcept;
        const Roads& GetRoads() const noexcept;
        const Offices& GetOffices() const noexcept;
        const int GetTypeItemCount() const noexcept;
        const int GetBagCapacity() const noexcept;
        const ValueLoots& GetValueLoots() const noexcept;

        void AddRoad(const Road& road);
        void AddBuilding(const Building& building);
        void AddOffice(const Office& office);
        void SetSpeed(double speed); 
        void SetBagCapacity(int capacity);
        void SetScoreForLoot(int value);

    private:
        using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

        Id id_;
        std::string name_;
        Roads roads_;
        Buildings buildings_;
        OfficeIdToIndex warehouse_id_to_index_;
        Offices offices_;
        ValueLoots value_loots_;
        double speed_on_map_ = 1.0;
        int bag_capacity_on_map_ = 3;
        int type_loot_count_ = 0;
    };

    class DogListener {
    public:
        virtual std::optional<std::chrono::milliseconds> TimerChange(std::chrono::milliseconds delta, bool inaction) = 0;
    };

    class TimeChanger : public DogListener {
    public:
        explicit TimeChanger(std::chrono::milliseconds time);        
        std::optional<std::chrono::milliseconds> TimerChange(std::chrono::milliseconds delta, bool inaction);

    private:
        std::chrono::milliseconds all_time_;
        std::chrono::milliseconds inaction_time_;
        std::chrono::milliseconds accept_inaction_time_ ;
    };

    class Dog {
    public:

        struct FindItem {
            int id;
            int type_item;          
        };

        using BagContent = std::vector<FindItem>;

        explicit Dog(int id, const std::string& nickname, const Position& pos);
        const std::string& GetDogName() const; 
        int GetDogId() const;
        const Speed& GetSpeed() const;
        double GetSpeedValue() const;
        const int GetBagCapacity() const;
        const Position& GetPosition() const;
        const std::string& GetDirection() const;
        const BagContent& GetBagContent() const;
        const int GetScore() const;

        void SetSpeed(double speed);
        void SetBagCapacity(int capacity);
        void AddScore(int score);
        void SetListener(std::chrono::milliseconds time);

        std::optional<std::chrono::milliseconds> InActiveDog(std::chrono::milliseconds delta);

        void ChangeDirection(const std::string& dir);
        void ChangePosition(const Position& pos);
        void StopMove();
        bool PutInBag(const FindItem& item);
        void ReturnBagContents();

    private:
        bool DogIsStop();

        int id_; 
        std::string nickname_; 
        double speed_value_;
        int bag_capacity_ = 3;
        BagContent bag_content_;
        Speed speed_;
        Position pos_;
        int score_ = 0;
        std::string direction_ = Direction::NORTH;
        std::shared_ptr<DogListener> listener_ = nullptr;
    };

    class SessionListener {
    public:
        virtual void RetirementDog(std::shared_ptr<Dog> dog, const model::Map::Id& map_id, std::chrono::milliseconds time) = 0;
    };

    class GameSession {
    public:
    
        struct LootData{
            int id;
            int type_loot;
            Position pos;
        };

        using Dogs = std::map<int, std::shared_ptr<Dog>>;
        using LostObjects = std::vector<LootData>;

        explicit GameSession(const Map* map, LootGenData data);
        std::shared_ptr<Dog> AddDog(const std::string& user_name);
        std::shared_ptr<Dog> FindDog(int dog_id);
        
        void AddExistDog(std::shared_ptr<Dog> dog);
        void AddLootData(LootData loot);
        ToRetiredDogInfo DeleteDog(int dog_id, std::chrono::milliseconds time);

        void SetListener(std::shared_ptr<SessionListener> listener);
        bool HasListener() const;

        const Map::Id& GetMapId() const;
        Dogs& GetInfoDogs();
        LostObjects& GetLostObjects();

        void SetRandom();
        void SetDogRetirementTime(std::chrono::milliseconds time);

        void GenerateNewLoot(std::chrono::milliseconds interval, int item_count);
        void ExchangeItemForScore(int dog_id);
        void RemoveCollectedItems(const std::set<int>& items_for_delete);

    private:
        int GenerateRandomValue(int max_value);
        Position GenerateRandomPosition();

        bool random_points_ = false;

        Dogs dogs_;
        LostObjects lost_objects_; 
        const Map* map_; 
        int id_count = 0;
        int loot_count_ = 0;
        std::chrono::milliseconds retirement_time_;
        loot_gen::LootGenerator loot_gen_;

        std::shared_ptr<SessionListener> listener_ = nullptr;
    };

    class Game {
    private: 
        using MapIdHasher = util::TaggedHasher<Map::Id>;
        using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;
    public:
        using Maps = std::vector<Map>;
        using GameSessions = std::unordered_map<Map::Id, std::shared_ptr<GameSession>, MapIdHasher>;
        
        void AddMap(const Map& map, double speed, int capacity);
        std::shared_ptr<GameSession> FindSession(const Map::Id& map_id); 


        const GameSessions& GetSessions() const;
        const Maps& GetMaps() const noexcept;
        const Map* FindMap(const Map::Id& id) const noexcept;
        void SetRandomaizer();
        void SetLootGenData(double period, double probability);
        void SetDogRetirementTime(double time_s);
        const LootGenData& GetLootGenData() const;

    private:
        std::vector<Map> maps_;
        GameSessions sessions_;
        MapIdToIndex map_id_to_index_;
        bool random_points_ = false;
        std::chrono::milliseconds retirement_time_;
        LootGenData gen_data_;
    };

}  // namespace model
