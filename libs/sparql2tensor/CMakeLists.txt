# Necessary includes
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Enforce C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find cmake packages
find_package(sparql-parser-base REQUIRED)
find_package(robin_hood REQUIRED)
find_package(dice-hash REQUIRED)

# Define the library
add_library(sparql2tensor
        src/Dice/sparql2tensor/parser/visitors/PrologueVisitor.cpp
        src/Dice/sparql2tensor/parser/visitors/SelectQueryVisitor.cpp
        src/Dice/sparql2tensor/SPARQLQuery.cpp
        )

add_library(Dice::sparql2tensor ALIAS sparql2tensor)

target_include_directories(sparql2tensor PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
        private-include
        )

target_link_libraries(sparql2tensor PUBLIC
        Dice::rdf_tensor
        robin_hood::robin_hood
        PRIVATE
        sparql-parser-base::sparql-parser-base
        )

## Packaging and exporting the target

# define export
install(TARGETS sparql2tensor
        EXPORT sparql2tensor-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

# generate version file and install it
write_basic_package_version_file(sparql2tensor-config-version.cmake
        VERSION ${tentris_lib_VERSION}
        COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sparql2tensor-config-version.cmake DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# dummy config file
configure_file(../../cmake/dummy-config.cmake.in sparql2tensor-config.cmake @ONLY)

# install header files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp")

# Export the package
install(EXPORT sparql2tensor-targets
        FILE sparql2tensor-config.cmake
        NAMESPACE Dice::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sparql2tensor)

