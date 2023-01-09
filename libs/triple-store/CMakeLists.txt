# Find cmake packages
find_package(dice-hash REQUIRED)
find_package(spdlog REQUIRED)
find_package(tentris-query REQUIRED COMPONENTS sparql)

# Define the library
add_library(triple-store
        src/dice/triple-store/TripleStore.cpp
        )

add_library(tentris::triple-store ALIAS triple-store)

target_include_directories(triple-store PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private-include
        )

target_link_libraries(triple-store PUBLIC
        tentris::rdf-tensor
        tentris-query::sparql
        PRIVATE
        spdlog::spdlog
        )

## Packaging and exporting the target

set_target_properties(triple-store PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

include(${PROJECT_SOURCE_DIR}/cmake/install_library.cmake)
install_component(triple-store src)
