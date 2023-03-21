cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

find_package(tentris-frontend REQUIRED)

add_executable(example example.cpp)

target_link_libraries(example PRIVATE
        tentris-frontend::tentris-frontend
        )

set_target_properties(
        example PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)
