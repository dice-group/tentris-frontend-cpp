cmake_minimum_required(VERSION 3.15)
project(tentris
        LANGUAGES CXX
        VERSION 1.2.3)
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

if (NOT TENTRIS_BINARY_BUILD)
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
endif ()

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


install(TARGETS rdf4cpp
        EXPORT rdf4cpp_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(EXPORT rdf4cpp_Targets
        FILE rdf4cppTargets.cmake
        NAMESPACE Dice::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/rdf4cpp/cmake)

add_subdirectory(libs)
