set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

add_executable(tentris_server
        src/Dice/tentris_server/TentrisServer.cpp
        )

find_package(Taskflow REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(cxxopts REQUIRED)


target_link_libraries(tentris_server PUBLIC
        Dice::triple_store
        Dice::endpoint
        restinio::restinio
        Taskflow::Taskflow
        nlohmann_json::nlohmann_json
        spdlog::spdlog
        cxxopts::cxxopts
        )

target_include_directories(tentris_server PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src>
        )
