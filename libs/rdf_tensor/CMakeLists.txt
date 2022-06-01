# Necessary includes
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(FetchContent)

# Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find cmake packages
find_package(Boost REQUIRED)
find_package(hypertrie REQUIRED)

# Fetch external libraries
FetchContent_Declare(
        tsl-sparse-map-our-version
        GIT_REPOSITORY https://github.com/dice-group/sparse-map.git
        GIT_TAG custom_tests
        GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(tsl-sparse-map-our-version)
FetchContent_Declare(
        metall
        GIT_REPOSITORY https://github.com/LLNL/metall.git
        GIT_TAG v0.20
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(metall)

# Define the library
add_library(rdf_tensor INTERFACE)
add_library(Dice::rdf_tensor ALIAS rdf_tensor)
target_link_libraries(rdf_tensor INTERFACE
        rdf4cpp::rdf4cpp
        hypertrie::hypertrie
        Metall::Metall
        Boost::headers
        )

target_include_directories(rdf_tensor INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

## Packaging and exporting the target

# define export
install(TARGETS rdf_tensor
        EXPORT rdf_tensor-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

# generate version file and install it
write_basic_package_version_file(rdf_tensor-config-version.cmake
        VERSION ${tentris_lib_VERSION}
        COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rdf_tensor-config-version.cmake DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# dummy config file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/dummy-config.cmake.in rdf_tensor-config.cmake @ONLY)

# install header files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp")

# Export the package
install(EXPORT rdf_tensor-targets
        FILE rdf_tensor-config.cmake
        NAMESPACE Dice::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rdf_tensor)