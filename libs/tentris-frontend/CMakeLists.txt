cmake_minimum_required(VERSION 3.21)
# meta library tentris-frontend::tentris-frontend
# imports everything and holds the version header
set(lib "${PROJECT_NAME}")

# Define the library
add_library(${lib} INTERFACE)
add_library(${PROJECT_NAME}::${lib} ALIAS ${lib})

configure_file(${PROJECT_SOURCE_DIR}/cmake/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/dice/${PROJECT_NAME}/${PROJECT_NAME}_version.hpp)

target_include_directories(${lib} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        )

target_link_libraries(${lib} INTERFACE
        ${PROJECT_NAME}::endpoint
        )

## Packaging and exporting the target
include(${PROJECT_SOURCE_DIR}/cmake/install_components.cmake)
install_component(INTERFACE ${lib} src)
