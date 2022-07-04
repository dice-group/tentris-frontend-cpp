include(GNUInstallDirs)

add_executable(rdf2ids
        src/Dice/tools/rdf2ids/RDF2IDs.cpp
        )

find_package(Threads REQUIRED)
find_package(tsl-sparse-map REQUIRED)
find_package(robin_hood REQUIRED)
find_package(dice-hash REQUIRED)
find_package(spdlog REQUIRED)
find_package(cxxopts REQUIRED)
find_package(vincentlaucsb-csv-parser REQUIRED)
find_package(rdf4cpp REQUIRED)
find_package(serd REQUIRED)



target_link_libraries(rdf2ids PRIVATE
        ${TENTRIS_STATIC_DEF}
        Threads::Threads
        dice-hash::dice-hash
        rdf4cpp::rdf4cpp
        tsl::sparse_map
        robin_hood::robin_hood
        spdlog::spdlog
        cxxopts::cxxopts
        vincentlaucsb-csv-parser::vincentlaucsb-csv-parser
        serd::serd
        )

configure_file(${CMAKE_SOURCE_DIR}/../cmake/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/Dice/tools/rdf2ids/tentris_version.hpp)
target_include_directories(rdf2ids PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src>
        )


if (CMAKE_BUILD_TYPE MATCHES "Release")
    set_target_properties(rdf2ids PROPERTIES LINK_FLAGS_RELEASE -s)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result LANGUAGES CXX) # fatal error if IPO is not supported
    set_property(TARGET rdf2ids PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
endif ()
