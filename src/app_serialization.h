#pragma once 

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/string.hpp>

#include "model_serialization.h"
#include "app.h"
#include "tagged.h"


namespace serialization {

    class SessionPlayersRepr { 
    public:
        SessionPlayersRepr() = default;

        explicit SessionPlayersRepr(model::GameSession& session, app::Players& players, app::PlayerTokens& tokens)
            : session_{session} 
        {          
            auto map_id = session.GetMapId();
            for(auto& dog : session.GetInfoDogs()){            
                auto player = players.FindByDogidAndMapid(dog.second->GetDogId(), map_id);
                tokens_.emplace(std::pair{dog.second->GetDogId(), **tokens.FindToken(player)});
            }
        }

        void Restore(app::Application& app){          
            auto session = app.GetGame().FindSession(model::Map::Id{session_.GetMapId()});
            auto& players = app.GetPlayers();
            auto& tokens = app.GetPlayerTokens();

            for(auto dog : session_.GetDogs()){
                auto dog_ptr = std::make_shared<model::Dog>(dog.Restore()); 
                session->AddExistDog(dog_ptr);

                auto player = players.AddPlayer(dog_ptr, session);
                if(tokens_.count(player->GetId())){
                    tokens.AddPlayerWithToken(player, app::Token{tokens_.at(player->GetId())});
                }
                else{
                    throw std::runtime_error("Failed to add token player");
                }
            }         
            for(auto obj : session_.GetLostObj()){
                session->AddLootData(obj);
            }        
        }

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
            ar & tokens_;
            ar & session_;
        }

    private:
        using Tokens = std::unordered_map<int, std::string>; 
        Tokens tokens_; 
        SessionRepr session_;
    };


    class ApplicationRepr {
    public:
        ApplicationRepr() = default;

        explicit ApplicationRepr(app::Application& app){
            auto& players = app.GetPlayers();
            auto& tokens = app.GetPlayerTokens();
            auto& game = app.GetGame();

            for(auto& session : game.GetSessions()){          
               sessions_.emplace_back(SessionPlayersRepr{*session.second, players, tokens});
            }
        }

        void Restore(app::Application& app) {       
            for(auto& session_repr : sessions_){
                session_repr.Restore(app);
            }
        }

        template <typename Archive>
        void serialize(Archive& ar, [[maybe_unused]] const unsigned version) {
            ar & sessions_;
        }

    private:
        std::vector<SessionPlayersRepr> sessions_;
    };

    void AppSerialization(const std::filesystem::path& file_serialize, app::Application& app);
    void AppDeserialization(const std::filesystem::path& file_serialize, app::Application& app);
    
} // namespace serialization