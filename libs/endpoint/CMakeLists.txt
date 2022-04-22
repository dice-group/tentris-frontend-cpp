set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

add_library(endpoint
        src/Dice/endpoint/HTTPServer.cpp
        src/Dice/endpoint/SparqlEndpoint.cpp
        src/Dice/endpoint/CountEndpoint.cpp
        src/Dice/endpoint/AskEndpoint.cpp
        src/Dice/endpoint/SparqlStreamingEndpoint.cpp
        src/Dice/endpoint/SparqlQueryCache.cpp
        )

find_package(Taskflow REQUIRED)
find_package(restinio REQUIRED)
find_package(spdlog REQUIRED)

set(ENV{cppitertools_INSTALL_CMAKE_DIR} TRUE)
FetchContent_Declare(
        cppitertools
        GIT_REPOSITORY https://github.com/ryanhaining/cppitertools.git
        GIT_TAG v2.1
        GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(cppitertools)

add_library(Dice::endpoint ALIAS endpoint)

target_link_libraries(endpoint PUBLIC
        Dice::triple_store
        restinio::restinio
        Taskflow::Taskflow
        PRIVATE
        spdlog::spdlog
        cppitertools::cppitertools
        rapidjson
        )

target_include_directories(endpoint PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private-include
        )