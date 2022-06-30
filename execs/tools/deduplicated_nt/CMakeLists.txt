set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

add_executable(deduplicated_nt
        src/Dice/tools/deduplicated_nt/DeduplicatedNT.cpp
        )

find_package(tsl-sparse-map REQUIRED)
find_package(robin_hood REQUIRED)
find_package(dice-hash REQUIRED)
find_package(spdlog REQUIRED)
find_package(cxxopts REQUIRED)
find_package(rdf4cpp REQUIRED)
find_package(serd REQUIRED)

target_link_libraries(deduplicated_nt PRIVATE
        ${TENTRIS_STATIC_DEF}
        ${TENTRIS_PTHREAD_DEF}
        rdf4cpp::rdf4cpp
        tsl::sparse_map
        robin_hood::robin_hood
        dice-hash::dice-hash
        spdlog::spdlog
        cxxopts::cxxopts
        serd::serd
        )

target_include_directories(deduplicated_nt PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src>
        )


if (CMAKE_BUILD_TYPE MATCHES "Release")
    set_target_properties(deduplicated_nt PROPERTIES LINK_FLAGS_RELEASE -s)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result LANGUAGES CXX) # fatal error if IPO is not supported
    set_property(TARGET deduplicated_nt PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif ()
