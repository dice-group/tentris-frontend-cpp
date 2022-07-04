add_executable(tentris_server
        src/Dice/tentris_server/TentrisServer.cpp
        )

find_package(Threads REQUIRED)
find_package(Taskflow REQUIRED)
find_package(spdlog REQUIRED)
find_package(cxxopts REQUIRED)
find_package(restinio REQUIRED)
find_package(Metall REQUIRED)

target_link_libraries(tentris_server PRIVATE
        ${TENTRIS_STATIC_DEF}
        Threads::Threads
        tentris::triple_store
        tentris::node_store
        tentris::endpoint
        restinio::restinio
        Taskflow::Taskflow
        spdlog::spdlog
        cxxopts::cxxopts
        Metall::Metall
        )

configure_file(${CMAKE_SOURCE_DIR}/../cmake/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/Dice/tentris_server/tentris_version.hpp)
target_include_directories(tentris_server PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src>
        )


if (CMAKE_BUILD_TYPE MATCHES "Release")
    set_target_properties(tentris_server PROPERTIES LINK_FLAGS_RELEASE -s)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result LANGUAGES CXX) # fatal error if IPO is not supported
    set_property(TARGET tentris_server PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif ()
