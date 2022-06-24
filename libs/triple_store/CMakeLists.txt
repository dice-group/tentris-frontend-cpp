# Find cmake packages
find_package(dice-hash REQUIRED)

# Find external libraries
find_package(serd REQUIRED VERSION EXACT 0)

# Define the library
add_library(triple_store
        src/Dice/triple_store/SerdLoad.cpp
        )

add_library(tentris::triple_store ALIAS triple_store)

target_include_directories(triple_store PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

target_link_libraries(triple_store PUBLIC
        tentris::sparql2tensor
        tentris::rdf_tensor
        PRIVATE
        serd::serd
        )

## Packaging and exporting the target

set_target_properties(triple_store PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

include(${PROJECT_SOURCE_DIR}/cmake/install_library.cmake)
install_component(triple_store src)
