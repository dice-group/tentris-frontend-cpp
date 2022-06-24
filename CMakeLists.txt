cmake_minimum_required(VERSION 3.18)
project(tentris
        LANGUAGES CXX
        VERSION 1.3.0)
include(GNUInstallDirs)

SET("tentris_lib_VERSION" "${tentris_VERSION}" CACHE STRING "" FORCE)
SET("tentris_lib_VERSION_MAJOR" "${tentris_VERSION_MAJOR}" CACHE STRING "" FORCE)
SET("tentris_lib_VERSION_MINOR" "${tentris_VERSION_MINOR}" CACHE STRING "" FORCE)
SET("tentris_lib_VERSION_PATCH" "${tentris_VERSION_PATCH}" CACHE STRING "" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET("rdf4cpp_VERSION" "v0.0.3" CACHE STRING "" FORCE)

# must be after rdf4cpp_VERSION is defined because it exposes the version

#[[if (NOT TENTRIS_BINARY_BUILD)
    ### conan cmake
    if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
        message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
        file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake"
                EXPECTED_HASH SHA256=5cdb3042632da3efff558924eecefd580a0e786863a857ca097c3d1d43df5dcd
                TLS_VERIFY ON)
    endif ()

    include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)
    conan_cmake_autodetect(settings)

    conan_cmake_run(CONANFILE lib_conanfile.txt
            GENERATORS cmake_find_package
            BASIC_SETUP CMAKE_TARGETS
            BUILD missing
            SETTINGS ${settings})

    include(${CMAKE_CURRENT_BINARY_DIR}/conan_paths.cmake)
endif ()]]

if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/CMakeCache.txt)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    endif ()
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g -O0")

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE
        thirdparty/RapidJSON/include
        )

find_package(Boost REQUIRED COMPONENTS system log_setup log thread)

include(FetchContent)
FetchContent_Declare(
        RDF4CPP
        GIT_REPOSITORY "https://github.com/rdf4cpp/rdf4cpp"
        GIT_TAG "${rdf4cpp_VERSION}"
)
FetchContent_MakeAvailable(RDF4CPP)

# Necessary includes
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# define export
install(TARGETS rdf4cpp
        EXPORT rdf4cpp-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )

# generate version file and install it
write_basic_package_version_file(rdf4cpp-config-version.cmake
        VERSION ${rdf4cpp_VERSION}
        COMPATIBILITY SameMajorVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/rdf4cpp-config-version.cmake DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# dummy config file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dummy-config.cmake.in rdf4cpp-config.cmake @ONLY)

# install header files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.hpp")

# Export the package
install(EXPORT rdf4cpp-targets
        FILE rdf4cpp-config.cmake
        NAMESPACE rdf4cpp::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/rdf4cpp)


add_subdirectory(libs)
