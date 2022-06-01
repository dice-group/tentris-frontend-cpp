cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

message("CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}")
find_package(tentris REQUIRED)

include(FetchContent)
FetchContent_Declare(
        RDF4CPP
        GIT_REPOSITORY "https://github.com/rdf4cpp/rdf4cpp"
        GIT_TAG "develop"
)
FetchContent_MakeAvailable(RDF4CPP)

add_executable(example src/example.cpp)
target_link_libraries(example
        tentris::tentris
        rdf4cpp::rdf4cpp)
