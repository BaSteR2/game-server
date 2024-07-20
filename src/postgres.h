#pragma once

#include <pqxx/connection>
#include <pqxx/transaction>
#include <pqxx/pqxx>
#include <condition_variable>
#include "model.h"
#include "retired_dogs.h"
#include <iostream>


namespace postgres {

    struct AppConfig {
        std::string db_url;
    };

    constexpr const char DB_URL_ENV_NAME[]{"GAME_DB_URL"};
    
    AppConfig GetConfigFromEnv();

    class ConnectionPool {
        using PoolType = ConnectionPool;
        using ConnectionPtr = std::shared_ptr<pqxx::connection>;

    public:
        class ConnectionWrapper {
        public:
            ConnectionWrapper(std::shared_ptr<pqxx::connection>&& conn, PoolType& pool) noexcept
                : conn_{std::move(conn)}
                , pool_{&pool} {
            }

            ConnectionWrapper(const ConnectionWrapper&) = delete;
            ConnectionWrapper& operator=(const ConnectionWrapper&) = delete;

            ConnectionWrapper(ConnectionWrapper&&) = default;
            ConnectionWrapper& operator=(ConnectionWrapper&&) = default;

            pqxx::connection& operator*() const& noexcept {
                return *conn_;
            }
            pqxx::connection& operator*() const&& = delete;

            pqxx::connection* operator->() const& noexcept {
                return conn_.get();
            }

            ~ConnectionWrapper() {
                if (conn_) {
                    pool_->ReturnConnection(std::move(conn_));
                }
            }

        private:
            std::shared_ptr<pqxx::connection> conn_;
            PoolType* pool_;
        };

        template <typename ConnectionFactory>
        ConnectionPool(size_t capacity, ConnectionFactory&& connection_factory) {
            pool_.reserve(capacity);
            for (size_t i = 0; i < capacity; ++i) {
                pool_.emplace_back(connection_factory());
            }
        }

        ConnectionWrapper GetConnection() {
            std::unique_lock lock{mutex_};
            cond_var_.wait(lock, [this] {
                return used_connections_ < pool_.size();
            });
            return {std::move(pool_[used_connections_++]), *this};
        }

    private:
        void ReturnConnection(ConnectionPtr&& conn) {
            {
                std::lock_guard lock{mutex_};
                assert(used_connections_ != 0);
                pool_[--used_connections_] = std::move(conn);
            }
            cond_var_.notify_one();
        }

        std::mutex mutex_;
        std::condition_variable cond_var_;
        std::vector<ConnectionPtr> pool_;
        size_t used_connections_ = 0;
    }; 

    class RetiredDogRepositoryImpl : public model::RetiredDogRepository {
    public:
        explicit RetiredDogRepositoryImpl(ConnectionPool& conn_pull);
        void Save(model::RetiredDog dog) override;
        const std::vector<model::RetiredDog> LoadDataFromDB(int offset, int max_elem) const override;
    private:
        ConnectionPool& conn_pull_;
    };


    class DataBase {
    public:
        explicit DataBase(const AppConfig& config);
        void SaveRetiredDog(const model::ToRetiredDogInfo& dog);
        const std::vector<model::RetiredDog> GetRetiredDogs(int offset, int max_elem) const ;

    private:
        ConnectionPool conn_pull_;
        RetiredDogRepositoryImpl retired_dogs_{conn_pull_};
    };

} //namespace postgres