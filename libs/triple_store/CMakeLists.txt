# Necessary includes
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find external libraries
find_library(serd
        NAMES serd-0
        NO_CACHE)

# Find cmake packages
find_package(dice-hash REQUIRED)

# Define the library
add_library(triple_store
        src/Dice/triple_store/SerdLoad.cpp
        )

add_library(Dice::triple_store ALIAS triple_store)

target_include_directories(triple_store PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

target_link_libraries(triple_store PUBLIC
        Dice::sparql2tensor
        Dice::rdf_tensor
        ${serd}
        )

## Packaging and exporting the target

# define export
install(TARGETS triple_store
        EXPORT triple_store-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

# generate version file and install it
write_basic_package_version_file(triple_store-config-version.cmake
        VERSION ${tentris_lib_VERSION}
        COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/triple_store-config-version.cmake DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# dummy config file
configure_file(../../cmake/dummy-config.cmake.in triple_store-config.cmake @ONLY)

# install header files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp")

# Export the package
install(EXPORT triple_store-targets
        FILE triple_store-config.cmake
        NAMESPACE Dice::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/triple_store)