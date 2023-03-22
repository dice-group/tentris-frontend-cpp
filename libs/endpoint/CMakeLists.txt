cmake_minimum_required(VERSION 3.21)
set(lib "endpoint")

find_package(tentris REQUIRED)
find_package(Taskflow REQUIRED)
find_package(restinio REQUIRED)
find_package(spdlog REQUIRED)
find_package(cppitertools REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(pugixml REQUIRED)

# Workaround for poor conan package
if (TARGET rapidjson)
    set(rapidjson_target "rapidjson")
else()
    set(rapidjson_target "RapidJSON::RapidJSON")
endif()

add_library(${lib}
        src/dice/endpoint/HTTPServer.cpp
        src/dice/endpoint/SPARQLEndpoint.cpp
        src/dice/endpoint/SPARQLStreamEndpoint.cpp
        )
add_library(${PROJECT_NAME}::${lib} ALIAS ${lib})

target_include_directories(${lib} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private-include
        )

target_link_libraries(${lib} PUBLIC
        tentris::tentris
        restinio::restinio
        Taskflow::Taskflow
        PRIVATE
        spdlog::spdlog
        cppitertools::cppitertools
        ${rapidjson_target}
        pugixml::pugixml
        )

set_target_properties(endpoint PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )


include(${PROJECT_SOURCE_DIR}/cmake/install_components.cmake)
install_component(PUBLIC ${lib} src)