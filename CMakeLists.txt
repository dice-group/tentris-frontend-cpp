cmake_minimum_required(VERSION 3.21)
project(tentris-frontend
        VERSION 1.3.0
        DESCRIPTION "tensor-based triplestore")

include(cmake/boilerplate_init.cmake)
boilerplate_init()

OPTION(USE_CONAN "If available, use conan to retrieve dependencies." ON)
if (PROJECT_IS_TOP_LEVEL AND USE_CONAN)
    include(cmake/conan_cmake.cmake)
    install_packages_via_conan("${CMAKE_SOURCE_DIR}/conanfile.py" "${CONAN_OPTIONS}")
endif ()

if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/CMakeCache.txt)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    endif ()
endif ()



add_subdirectory(libs)
