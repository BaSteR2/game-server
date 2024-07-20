#pragma once

#include "app.h"
#include "model.h"
#include "app_serialization.h"
#include <string>


namespace insfrastruct {

    using namespace std::literals;  
    
    class SerializationListener : public app::ApplicationListener {
    public:
        explicit SerializationListener(std::chrono::milliseconds save_period)
            : save_period_(save_period)
            , time_since_save_(0ms)
        {}

        void OnTick(std::chrono::milliseconds delta, app::Application& app) override {
            time_since_save_ += delta;
            if(time_since_save_ >= save_period_){
                serialization::AppSerialization(file_serialize_, app);           
                time_since_save_ = 0ms;
            }
        }

        void SetMainSerFile(const std::filesystem::path& file_name) {
            file_serialize_ = file_name;
        }   

    private:
        std::chrono::milliseconds time_since_save_ ;
        std::chrono::milliseconds save_period_;
        
        std::filesystem::path file_serialize_;
    };

} // namespace infrastruct