set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

add_library(node_store
        src/Dice/node_store/TslSparseMapNodeStorageBackend.cpp
        )

add_library(Dice::node_store ALIAS node_store)

find_package(hypertrie REQUIRED)
find_package(tsl-sparse-map REQUIRED)
find_package(robin_hood REQUIRED)
find_package(dice-hash REQUIRED)


target_link_libraries(node_store PUBLIC
        rdf4cpp::rdf4cpp
        tsl::sparse_map
        dice-hash::dice-hash
        )

target_include_directories(node_store PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )
