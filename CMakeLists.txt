cmake_minimum_required(VERSION 3.18)
project(tentris
        VERSION 1.3.0
        DESCRIPTION "TODO")

SET("tentris_lib_VERSION" "${tentris_VERSION}" CACHE STRING "" FORCE)
SET("tentris_lib_VERSION_MAJOR" "${tentris_VERSION_MAJOR}" CACHE STRING "" FORCE)
SET("tentris_lib_VERSION_MINOR" "${tentris_VERSION_MINOR}" CACHE STRING "" FORCE)
SET("tentris_lib_VERSION_PATCH" "${tentris_VERSION_PATCH}" CACHE STRING "" FORCE)

include(cmake/boilerplate_init.cmake)
boilerplate_init()

option(CONAN_CMAKE "If this should use conan cmake to fetch dependencies" On)
if (IS_TOP_LEVEL AND CONAN_CMAKE)
    include(cmake/conan_cmake.cmake)
    install_packages_via_conan("${CMAKE_CURRENT_SOURCE_DIR}/conanfile.py" "")
endif ()

if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/CMakeCache.txt)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    endif ()
endif ()



add_subdirectory(libs)
