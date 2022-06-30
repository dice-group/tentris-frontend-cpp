cmake_minimum_required(VERSION 3.18)
project(tentris-binaries)

include(${CMAKE_SOURCE_DIR}/../cmake/boilerplate_init.cmake)

boilerplate_init()
if(NOT IS_TOP_LEVEL)
    message(FATAL_ERROR "Must only be used as CMake top-level project.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/CMakeCache.txt)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    endif ()
endif ()

if (DEFINED ${MARCH})
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=${MARCH}")
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fomit-frame-pointer -momit-leaf-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fuse-ld=lld")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
else ()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fomit-frame-pointer")

endif ()

option(TENTRIS_STATIC "Build tentris executables as fully static binaries." OFF)
option(WITH_TCMALLOC "Build tentris with tcmalloc." OFF)

if (TENTRIS_STATIC)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .a)
endif ()

option(TENTRIS_STATIC "Build tentris executables statically" OFF)
if (WITH_TCMALLOC)
    find_library(TCMALLOCMINIMAL tcmalloc_minimal)
    if (NOT TCMALLOCMINIMAL)
        find_library(TCMALLOCMINIMAL tcmalloc-minimal)
    endif ()
    if (NOT TCMALLOCMINIMAL)
        message(FATAL_ERROR "Neither tcmalloc-minimal nor tcmalloc_minimal was found")
    endif ()
    message("tcmalloc minimal ${TCMALLOCMINIMAL}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${TCMALLOCMINIMAL}")
    if (TENTRIS_STATIC)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--eh-frame-hdr")
    endif ()
endif ()


# set library options
if (TENTRIS_STATIC)
    SET(Boost_USE_STATIC_LIBS ON)
    SET(RESTINIO_USE_BOOST_ASIO=static)
endif ()

include(${CMAKE_SOURCE_DIR}/../cmake/conan_cmake.cmake)

install_packages_via_conan("${CMAKE_SOURCE_DIR}/../conanfile.py" "with_exec_deps=True")

find_package(Boost REQUIRED COMPONENTS system log_setup log thread)


# for rt and pthread linkage see:
# * https://stackoverflow.com/questions/58848694/gcc-whole-archive-recipe-for-static-linking-to-pthread-stopped-working-in-rec
# * https://stackoverflow.com/questions/35116327/when-g-static-link-pthread-cause-segmentation-fault-why
if (TENTRIS_STATIC)
    set(TENTRIS_STATIC_DEF -static)
    set(TENTRIS_PTHREAD_DEF -Wl,--whole-archive -lrt -lpthread -Wl,--no-whole-archive)
else ()
    set(TENTRIS_PTHREAD_DEF -lpthread)
endif ()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/libtentris)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/tentris_server/src/Dice/tentris_server/tentris_version.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/tentris_loader/src/Dice/tentris_loader/tentris_version.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/tools/rdf2ids/src/Dice/tools/rdf2ids/tentris_version.hpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/tools/deduplicated_nt/src/Dice/tools/deduplicated_nt/tentris_version.hpp)

add_subdirectory(tentris_server)
add_subdirectory(tentris_loader)
add_subdirectory(tools)