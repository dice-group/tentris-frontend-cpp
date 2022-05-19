set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

find_library(serd
        NAMES serd-0
        NO_CACHE)

add_library(triple_store
        src/Dice/triple_store/SerdLoad.cpp
        )

add_library(Dice::triple_store ALIAS triple_store)

target_link_libraries(triple_store PUBLIC
        Dice::sparql2tensor
        Dice::rdf_tensor
        ${serd}
        )

target_include_directories(triple_store PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )
