# Necessary includes
include(FetchContent)

# Find cmake packages
find_package(hypertrie REQUIRED)
#find_package(dice-sparse-map REQUIRED)
find_package(robin_hood REQUIRED)
find_package(dice-hash REQUIRED)
find_package(Boost REQUIRED)

# Define the library
add_library(node_store
        src/dice/node_store/PersistentNodeStorageBackendImpl.cpp
        src/dice/node_store/PersistentNodeStorageBackend.cpp
        src/dice/node_store/MetallBNodeBackend.cpp
        src/dice/node_store/MetallIRIBackend.cpp
        src/dice/node_store/MetallLiteralBackend.cpp
        src/dice/node_store/MetallVariableBackend.cpp
        )
add_library(tentris::node_store ALIAS node_store)

target_link_libraries(node_store PUBLIC
        tentris::rdf_tensor
        dice-sparse-map::dice-sparse-map
        )

target_include_directories(node_store PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        )

set_target_properties(node_store PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

include(${PROJECT_SOURCE_DIR}/cmake/install_library.cmake)
install_component(node_store src)
