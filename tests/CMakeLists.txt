cmake_minimum_required(VERSION 3.18)
project(tentris-tests)

include(${CMAKE_SOURCE_DIR}/../cmake/boilerplate_init.cmake)

boilerplate_init()
if (NOT IS_TOP_LEVEL)
    message(FATAL_ERROR "Must only be used as CMake top-level project.")
endif ()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_SOURCE_DIR}/../cmake/conan_cmake.cmake)
install_packages_via_conan("${CMAKE_SOURCE_DIR}/../conanfile.py" "with_test_deps=True ")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/libtentris)

find_package(doctest REQUIRED)
find_package(pugixml REQUIRED)
find_package(CURL REQUIRED)

add_executable(tests_SPARQL_Aggregates sparql/tests_Aggregates.cpp)
target_link_libraries(tests_SPARQL_Aggregates
        doctest::doctest
        tentris::tentris
        pugixml::pugixml
        CURL::libcurl
        )
add_test(NAME tests_SPARQL_Aggregates COMMAND tests_SPARQL_Aggregates)

add_executable(tests_SPARQL_Filters sparql/tests_Filters.cpp)
target_link_libraries(tests_SPARQL_Filters
        doctest::doctest
        tentris::tentris
        pugixml::pugixml
        CURL::libcurl
        )
add_test(NAME tests_SPARQL_Filters COMMAND tests_SPARQL_Filters)


add_executable(tests_SPARQL_Subqueries sparql/tests_Subqueries.cpp)
target_link_libraries(tests_SPARQL_Subqueries
        doctest::doctest
        tentris::tentris
        pugixml::pugixml
        CURL::libcurl
        )
add_test(NAME tests_SPARQL_Subqueries COMMAND tests_SPARQL_Subqueries)

add_executable(tests_SPARQL_Exists sparql/tests_Exists.cpp)
target_link_libraries(tests_SPARQL_Exists
        doctest::doctest
        tentris::tentris
        pugixml::pugixml
        CURL::libcurl
        )
add_test(NAME tests_SPARQL_Exists COMMAND tests_SPARQL_Exists)

add_executable(tests_SPARQL_Functions sparql/tests_Functions.cpp)
target_link_libraries(tests_SPARQL_Functions
        doctest::doctest
        tentris::tentris
        pugixml::pugixml
        CURL::libcurl
        )
add_test(NAME tests_SPARQL_Functions COMMAND tests_SPARQL_Functions)

add_executable(tests_SPARQL_ProjectExpressions sparql/tests_ProjectExpressions.cpp)
target_link_libraries(tests_SPARQL_ProjectExpressions
        doctest::doctest
        tentris::tentris
        pugixml::pugixml
        CURL::libcurl
        )
add_test(NAME tests_SPARQL_ProjectExpressions COMMAND tests_SPARQL_ProjectExpressions)
