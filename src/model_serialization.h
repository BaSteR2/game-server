#pragma once
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

#include "model.h"
#include "geom.h"


namespace geom {

    template <typename Archive>
    void serialize(Archive& ar, Point2D& point, [[maybe_unused]] const unsigned version) {
        ar& point.x;
        ar& point.y;
    }

    template <typename Archive>
    void serialize(Archive& ar, Vec2D& vec, [[maybe_unused]] const unsigned version) {
        ar& vec.x;
        ar& vec.y;
    }

}  // namespace geom

namespace model {
    
    template <typename Archive>
    void serialize(Archive& ar, Position& obj, [[maybe_unused]] const unsigned version) {   
        ar&(obj.x);
        ar&(obj.y);
    }

    template <typename Archive>
    void serialize(Archive& ar, Dog::FindItem& obj, [[maybe_unused]] const unsigned version) {
        
        ar&(obj.id);
        ar&(obj.type_item);
    }

    template <typename Archive> 
    void serialize(Archive& ar, GameSession::LootData& obj, [[maybe_unused]] const unsigned version) {
        
        ar&(obj.id);
        ar&(obj.pos);
        ar&(obj.type_loot);
    }
}  // namespace model


namespace serialization {

    class DogRepr {
    public:
        DogRepr() = default;

        explicit DogRepr(const model::Dog& dog)
            : id_(dog.GetDogId())
            , name_(dog.GetDogName())
            , pos_(dog.GetPosition())
            , bag_capacity_(dog.GetBagCapacity())
            , speed_value_(dog.GetSpeedValue()) 
            , direction_(dog.GetDirection())
            , score_(dog.GetScore())
            , bag_content_(dog.GetBagContent()) 
        {}

        [[nodiscard]] model::Dog Restore() const {
            model::Dog dog{id_, name_, pos_};
            dog.SetSpeed(speed_value_);     
            dog.ChangeDirection(direction_);
            dog.StopMove();
            dog.AddScore(score_);
            for (const auto& item : bag_content_) {
                if (!dog.PutInBag(item)) {
                    throw std::runtime_error("Failed to put bag content");
                }
            }
            return dog;
        }

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
            ar& id_;
            ar& name_;
            ar& pos_;
            ar& bag_capacity_;
            ar& speed_value_;
            ar& direction_;
            ar& score_;
            ar& bag_content_;
        }

    private:
        int id_ = 0;
        std::string name_; 
        model::Position pos_;
        size_t bag_capacity_ = 0;
        double speed_value_;
        std::string direction_ = model::Direction::NORTH; 
        int score_ = 0;
        model::Dog::BagContent bag_content_;
    };

    class SessionRepr { 
    public:
        using DogsRepr = std::vector<DogRepr>;
        SessionRepr() = default;

        explicit SessionRepr(model::GameSession& session)
            : map_id_(*session.GetMapId())
            , lost_objs_(session.GetLostObjects())
        {
            for(auto& dog : session.GetInfoDogs()){
                dogs_.emplace_back(DogRepr{*dog.second});
            }
        }

        std::string GetMapId() {
            return map_id_;
        }

        DogsRepr& GetDogs() {
            return dogs_;
        }

        model::GameSession::LostObjects GetLostObj() {
            return lost_objs_;
        }

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
            ar & map_id_;
            ar & dogs_;
            ar & lost_objs_;
        }

    private:
        std::string map_id_;
        DogsRepr dogs_;
        model::GameSession::LostObjects lost_objs_;
    };


}  // namespace serialization
