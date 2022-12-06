# Find cmake packages
find_package(dice-hash REQUIRED)

# Find external libraries
find_package(serd REQUIRED VERSION EXACT 0)

# Define the library
add_library(triple-store INTERFACE)

add_library(tentris::triple-store ALIAS triple-store)

target_include_directories(triple-store INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

target_link_libraries(triple-store INTERFACE
        tentris::sparql2tensor
        tentris::rdf-tensor
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
install_interface_component(triple-store src)
