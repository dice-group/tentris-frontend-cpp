cmake_minimum_required(VERSION 3.21)

set(exec_name "tentris_loader")

add_executable(${exec_name}
        src/dice/tentris-loader/TentrisLoader.cpp
        )

find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)
find_package(cxxopts REQUIRED)

target_link_libraries(${exec_name} PRIVATE
        Threads::Threads
        tentris-frontend::tentris-frontend
        spdlog::spdlog
        cxxopts::cxxopts
        )

include(${PROJECT_SOURCE_DIR}/cmake/execs_optimizations.cmake)
execs_optimizations(${exec_name})
