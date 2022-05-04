set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

add_executable(tentris_server
        src/Dice/tentris_server/TentrisServer.cpp
        )

find_package(Taskflow REQUIRED)
find_package(spdlog REQUIRED)
find_package(cxxopts REQUIRED)

target_link_libraries(tentris_server PRIVATE
        ${TENTRIS_STATIC_DEF}
        ${TENTRIS_PTHREAD_DEF}
        Dice::triple_store
        Dice::node_store
        Dice::endpoint
        restinio::restinio
        Taskflow::Taskflow
        spdlog::spdlog
        cxxopts::cxxopts
        Metall
        )

target_include_directories(tentris_server PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src>
        )

if (CMAKE_BUILD_TYPE MATCHES "Release")
    set_target_properties(tentris_server PROPERTIES LINK_FLAGS_RELEASE -s)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result LANGUAGES CXX) # fatal error if IPO is not supported
    set_property(TARGET tentris_server PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif ()
