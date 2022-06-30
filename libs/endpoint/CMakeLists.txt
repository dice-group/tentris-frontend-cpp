find_package(Taskflow REQUIRED)
find_package(restinio REQUIRED)
find_package(spdlog REQUIRED)
find_package(cppitertools REQUIRED)
find_package(RapidJSON REQUIRED)

add_library(endpoint
        src/Dice/endpoint/HTTPServer.cpp
        src/Dice/endpoint/SparqlEndpoint.cpp
        src/Dice/endpoint/CountEndpoint.cpp
        src/Dice/endpoint/SparqlStreamingEndpoint.cpp
        src/Dice/endpoint/SparqlQueryCache.cpp
        )
add_library(tentris::endpoint ALIAS endpoint)

target_include_directories(endpoint PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private-include
        )

set_target_properties(endpoint PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

target_link_libraries(endpoint PUBLIC
        tentris::triple_store
        tentris::node_store
        restinio::restinio
        Taskflow::Taskflow
        PRIVATE
        spdlog::spdlog
        cppitertools::cppitertools
        RapidJSON::RapidJSON
        )

include(${PROJECT_SOURCE_DIR}/cmake/install_library.cmake)
install_component(endpoint src)