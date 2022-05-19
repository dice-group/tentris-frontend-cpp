set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

add_library(node_store
        src/Dice/node_store/PersistentNodeStorageBackendImpl.cpp
        src/Dice/node_store/PersistentNodeStorageBackend.cpp
        src/Dice/node_store/MetallBNodeBackend.cpp
        src/Dice/node_store/MetallIRIBackend.cpp
        src/Dice/node_store/MetallLiteralBackend.cpp
        src/Dice/node_store/MetallVariableBackend.cpp
        )

add_library(Dice::node_store ALIAS node_store)

find_package(hypertrie REQUIRED)
find_package(tsl-sparse-map REQUIRED)
find_package(robin_hood REQUIRED)
find_package(dice-hash REQUIRED)
find_package(Boost REQUIRED)



target_link_libraries(node_store PUBLIC
        Dice::rdf_tensor
        )

target_include_directories(node_store PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )
