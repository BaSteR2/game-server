#include "app_serialization.h"


namespace serialization {
    
    using namespace std::literals;

    void AppSerialization(const std::filesystem::path& file_serialize, app::Application& app) {
        auto temp_serialize = file_serialize.parent_path();
        temp_serialize  += ("/temp_file");

        std::ofstream out(temp_serialize , std::ios_base::binary);
        boost::archive::text_oarchive output{out};
        ApplicationRepr app_repr{app};
        output << app_repr;

        std::filesystem::rename(temp_serialize , file_serialize);
    }

    void AppDeserialization(const std::filesystem::path& file_serialize, app::Application& app) {
        try {
            if(!std::filesystem::exists(file_serialize)){
                return;
            }
            std::ifstream in(file_serialize, std::ios_base::binary);
            if (!in.is_open()) {
                throw std::ios_base::failure("Save file is not open");
            }   
            boost::archive::text_iarchive input{in};          
            ApplicationRepr app_repr;
            input >> app_repr;
            app_repr.Restore(app);
        }
        catch(const std::exception& e) {
            throw std::ios_base::failure(e.what());
        }
    }
} // namespace serialization