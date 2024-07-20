cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(model_lib STATIC 
	src/model.h
	src/model.cpp
	src/loot_generator.h
	src/loot_generator.cpp
	src/geom.h
	src/tagged.h
	src/model_serialization.h
	src/retired_dogs.h
	src/retired_dogs.cpp
	src/tagged_uuid.h
	src/tagged_uuid.cpp
)

add_library(collision_detection_lib STATIC
	src/collision_detector.h
	src/collision_detector.cpp
)

target_include_directories(model_lib PUBLIC collision_detection_lib PUBLIC CONAN_PKG::boost CONAN_PKG::libpq CONAN_PKG::libpqxx)
target_link_libraries(model_lib PUBLIC collision_detection_lib PUBLIC CONAN_PKG::boost Threads::Threads CONAN_PKG::libpq CONAN_PKG::libpqxx)

add_executable(game_server
	src/main.cpp
	src/http_server.cpp
	src/http_server.h
	src/sdk.h
	src/boost_json.cpp
	src/json_loader.h
	src/json_loader.cpp
	src/request_handler.cpp
	src/request_handler.h
	src/logger.h
	src/logger.cpp
	src/app.h
	src/app.cpp
	src/app_serialization.h
	src/app_serialization.cpp
	src/response_handler.h
	src/response_handler.cpp
	src/storage.h
	src/ticker.h
	src/extra_data.h
	src/extra_data.cpp
	src/infrastructure.h
	src/postgres.h
	src/postgres.cpp
)

add_executable(game_server_tests
    tests/collision-detector-tests.cpp
    tests/model-tests.cpp
    tests/loot_generator_tests.cpp
    tests/state-serialization-tests.cpp
)


target_link_libraries(game_server model_lib collision_detection_lib) 
target_link_libraries(game_server_tests CONAN_PKG::catch2 model_lib collision_detection_lib) 
