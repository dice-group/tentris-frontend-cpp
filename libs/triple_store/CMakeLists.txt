set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

find_library(serd
        NAMES serd-0
        NO_CACHE)
find_package(dice-hash REQUIRED)

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


install(TARGETS triple_store
        EXPORT triple_store_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(EXPORT triple_store_Targets
        FILE triple_storeTargets.cmake
        NAMESPACE Dice::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/triple_store/cmake)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

