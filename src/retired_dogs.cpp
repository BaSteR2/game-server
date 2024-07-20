#include "retired_dogs.h"

namespace model {

    RetiredDog::RetiredDog(const RetiredDogId& id, const std::string& name, int score, int play_time) 
        : id_(id)
        , name_(name)
        , score_(score)
        , play_time_(play_time)
    {}

    const RetiredDogId& RetiredDog::GetId() const noexcept {
        return id_;
    }

    const std::string& RetiredDog::GetName() const noexcept {
        return name_;
    }

    const int RetiredDog::GetScore() const noexcept {
        return score_;
    }

    const int RetiredDog::GetPlayTime() const noexcept {
        return play_time_;
    }

} //namespace model