set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

add_executable(tentris_loader
        src/Dice/tentris_loader/TentrisLoader.cpp
        )

find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)
find_package(cxxopts REQUIRED)
find_package(Metall REQUIRED)

target_link_libraries(tentris_loader PRIVATE
        ${TENTRIS_STATIC_DEF}
        Threads::Threads
        tentris::triple_store
        tentris::node_store
        spdlog::spdlog
        cxxopts::cxxopts
        Metall::Metall
        )

target_include_directories(tentris_loader PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src>
        )


if (CMAKE_BUILD_TYPE MATCHES "Release")
    set_target_properties(tentris_loader PROPERTIES LINK_FLAGS_RELEASE -s)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result LANGUAGES CXX) # fatal error if IPO is not supported
    set_property(TARGET tentris_loader PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif ()
