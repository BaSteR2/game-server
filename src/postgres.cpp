#include "postgres.h"


namespace postgres {

    using namespace std::literals;
    using pqxx::operator"" _zv;

    AppConfig GetConfigFromEnv() {
        AppConfig config;
        if (const auto* url = std::getenv(DB_URL_ENV_NAME)) {
            config.db_url = url;
        } else {
            throw std::runtime_error(DB_URL_ENV_NAME + " environment variable not found"s);
        }
        return config;
    }

    RetiredDogRepositoryImpl::RetiredDogRepositoryImpl(ConnectionPool& conn_pull) 
        : conn_pull_(conn_pull)
    {}

    void RetiredDogRepositoryImpl::Save(model::RetiredDog dog) {
        auto conn = conn_pull_.GetConnection();
        pqxx::work w{*conn}; 
        std::string text = "INSERT INTO retired_players (id, name, score, play_time_ms) VALUES ($1, $2, $3, $4);";
        w.exec_params(text,
            dog.GetId().ToString(), dog.GetName(), dog.GetScore(), dog.GetPlayTime());
        w.commit();
    }

    const std::vector<model::RetiredDog> RetiredDogRepositoryImpl::LoadDataFromDB(int offset, int max_elem)  const {
        auto conn = conn_pull_.GetConnection();
        pqxx::read_transaction r{*conn};

        std::vector<model::RetiredDog> result;
        std::string query_text = "SELECT id, name, score, play_time_ms FROM retired_players ORDER BY score DESC, play_time_ms, name LIMIT " + 
                                                                std::to_string(max_elem)  +" OFFSET " + std::to_string(offset) + ";";

        for(const auto& [id, name, score, time] : r.query< std::string, std::string, int, int>(query_text)){
            result.emplace_back(model::RetiredDogId::FromString(id), name, score, time);
        }

        r.commit();
        return result;
    }


    DataBase::DataBase(const AppConfig& config) 
        : conn_pull_{std::thread::hardware_concurrency(), [config]{return std::make_shared<pqxx::connection>(config.db_url);}} 
    {
        auto conn = conn_pull_.GetConnection();
        pqxx::work w{*conn};
        w.exec(R"(
            CREATE TABLE IF NOT EXISTS retired_players (
                id UUID CONSTRAINT rettired_player_id_constraint PRIMARY KEY,
                name varchar(100) NOT NULL,
                score integer,
                play_time_ms integer
            );
            )"_zv);
        w.exec(R"(CREATE INDEX IF NOT EXISTS retired_players_score_play_time_name_idx ON retired_players (score DESC, play_time_ms, name);)"_zv);
        w.commit();
    }

    void DataBase::SaveRetiredDog(const model::ToRetiredDogInfo& dog) {  
        retired_dogs_.Save(model::RetiredDog{model::RetiredDogId::New(), dog.name, dog.score, dog.play_time});
    }

    const std::vector<model::RetiredDog> DataBase::GetRetiredDogs(int offset, int max_elem) const {
        return retired_dogs_.LoadDataFromDB(offset, max_elem);
    }
} //namespace postgres