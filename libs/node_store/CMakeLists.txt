# Necessary includes
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(FetchContent)

# Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find cmake packages
find_package(hypertrie REQUIRED)
find_package(tsl-sparse-map REQUIRED)
find_package(robin_hood REQUIRED)
find_package(dice-hash REQUIRED)
find_package(Boost REQUIRED)

# Define the library
add_library(node_store
        src/Dice/node_store/PersistentNodeStorageBackendImpl.cpp
        src/Dice/node_store/PersistentNodeStorageBackend.cpp
        src/Dice/node_store/MetallBNodeBackend.cpp
        src/Dice/node_store/MetallIRIBackend.cpp
        src/Dice/node_store/MetallLiteralBackend.cpp
        src/Dice/node_store/MetallVariableBackend.cpp
        )
add_library(Dice::node_store ALIAS node_store)

target_link_libraries(node_store PUBLIC
        Dice::rdf_tensor
        )

target_include_directories(node_store PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

## Packaging and exporting the target

# define export
install(TARGETS node_store
        EXPORT node_store-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

# generate version file and install it
write_basic_package_version_file(node_store-config-version.cmake
        VERSION ${tentris_lib_VERSION}
        COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/node_store-config-version.cmake DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# dummy config file
configure_file(../../cmake/dummy-config.cmake.in node_store-config.cmake @ONLY)

# install header files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp")

# Export the package
install(EXPORT node_store-targets
        FILE node_store-config.cmake
        NAMESPACE Dice::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/node_store)
