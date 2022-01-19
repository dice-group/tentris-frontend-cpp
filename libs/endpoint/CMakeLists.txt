set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

add_library(endpoint
        src/Dice/endpoint/dummy.cpp
        )

find_package(Taskflow REQUIRED)
find_package(restinio REQUIRED)

add_library(Dice::endpoint ALIAS endpoint)

target_link_libraries(endpoint PUBLIC
        Dice::sparql2tensor
        restinio::restinio
        Taskflow::Taskflow
        rapidjson
        cppitertools::cppitertools
        )

target_include_directories(endpoint PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )
