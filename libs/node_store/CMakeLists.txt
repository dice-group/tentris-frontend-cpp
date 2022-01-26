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

include(FetchContent)
FetchContent_Declare(
        tsl-sparse-map-our-version
        GIT_REPOSITORY https://github.com/dice-group/sparse-map.git
        GIT_TAG custom_tests
        GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(tsl-sparse-map-our-version)

# TODO: remove work-around when merged
FetchContent_Declare(
        metall
#        GIT_REPOSITORY https://github.com/LLNL/metall.git
        GIT_REPOSITORY https://github.com/dice-group/metall.git
        GIT_TAG fix/inline
#        GIT_TAG v0.18
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(metall)


target_link_libraries(node_store PUBLIC
        rdf4cpp::rdf4cpp
        tsl::sparse_map
        dice-hash::dice-hash
        Metall
        )

target_include_directories(node_store PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )
