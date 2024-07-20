#pragma once 
#include "model.h"
#include "tagged_uuid.h"

namespace model {

    namespace detail {
        struct RetiredDogTag {};
    }  // namespace detail

    using RetiredDogId = util::TaggedUUID<detail::RetiredDogTag>;

    class RetiredDog {
    public:
        explicit RetiredDog(const RetiredDogId& id, const std::string& name, int score, int play_time);

        const RetiredDogId& GetId() const noexcept;
        const std::string& GetName() const noexcept;
        const int GetScore() const noexcept;
        const int GetPlayTime() const noexcept;

    private:
        RetiredDogId id_;
        std::string name_;
        int score_;
        double play_time_;
    };

    class RetiredDogRepository {
    public:
        virtual void Save(RetiredDog dog) = 0;
        virtual const std::vector<RetiredDog> LoadDataFromDB(int offset, int max_elem) const = 0;
    protected:
        ~RetiredDogRepository() = default;
    };

} //namespace model