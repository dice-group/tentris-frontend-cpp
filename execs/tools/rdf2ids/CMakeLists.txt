set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

add_executable(rdf2ids
        src/Dice/tools/rdf2ids/RDF2IDs.cpp
        )

find_library(serd
        NAMES serd-0
        NO_CACHE)

find_package(tsl-sparse-map REQUIRED)
find_package(dice-hash REQUIRED)
find_package(spdlog REQUIRED)
find_package(cxxopts REQUIRED)
find_package(vincentlaucsb-csv-parser)

target_link_libraries(rdf2ids PRIVATE
        ${TENTRIS_STATIC_DEF}
        ${TENTRIS_PTHREAD_DEF}
        dice-hash::dice-hash
        rdf4cpp::rdf4cpp
        tsl::sparse_map
        spdlog::spdlog
        cxxopts::cxxopts
        vincentlaucsb-csv-parser::vincentlaucsb-csv-parser
        ${serd-0}
        )

target_include_directories(rdf2ids PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src>
        )


if (CMAKE_BUILD_TYPE MATCHES "Release")
    set_target_properties(rdf2ids PROPERTIES LINK_FLAGS_RELEASE -s)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result LANGUAGES CXX) # fatal error if IPO is not supported
    set_property(TARGET rdf2ids PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif ()
