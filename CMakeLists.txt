cmake_minimum_required(VERSION 3.18)
project(tentris
        LANGUAGES CXX
        VERSION 1.2.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/tentris/tentris_version.hpp)


### conan cmake
if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake"
            EXPECTED_HASH SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
            TLS_VERIFY ON)
endif ()

include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)
conan_cmake_autodetect(settings)

conan_cmake_run(CONANFILE conanfile.txt
        GENERATORS cmake_find_package
        BASIC_SETUP CMAKE_TARGETS
        BUILD missing
        SETTINGS ${settings})

include(${CMAKE_CURRENT_BINARY_DIR}/conan_paths.cmake)

if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/CMakeCache.txt)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    endif ()
endif ()

if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    endif ()
endif ()

if (DEFINED ${TENTRIS_MARCH})
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=${TENTRIS_MARCH}")
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fomit-frame-pointer -momit-leaf-frame-pointer")
else ()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fomit-frame-pointer")
endif ()

if (TENTRIS_STATIC)
    SET(CMAKE_FIND_LIBRARY_SUFFIXES .a)
endif ()

if (TENTRIS_BUILD_WITH_TCMALLOC)
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
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g -O0")

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE
        thirdparty/RapidJSON/include
        )

find_package(fmt REQUIRED)
find_package(hypertrie REQUIRED)
find_package(vincentlaucsb-csv-parser REQUIRED)
find_package(cxxopts REQUIRED)
if (TENTRIS_STATIC)
    SET(Boost_USE_STATIC_LIBS ON)
endif ()

find_package(Boost REQUIRED COMPONENTS system log_setup log thread)
if (TENTRIS_STATIC)
    SET(RESTINIO_USE_BOOST_ASIO=static)
endif ()
find_package(restinio REQUIRED)
find_package(string-view-lite REQUIRED)
find_package(optional-lite REQUIRED)

# make a library of the code
add_library(tentris INTERFACE)

include(FetchContent)

set(cppitertools_INSTALL_CMAKE_DIR share CACHE INTERNAL "")
FetchContent_Declare(
        cppitertools
        GIT_REPOSITORY https://github.com/ryanhaining/cppitertools.git
        GIT_TAG b2b98e60438f1ed6b04b77cdb6cc5d5516af301b
        GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(cppitertools)

set(rdf4cpp_INSTALL_CMAKE_DIR share CACHE INTERNAL "")
FetchContent_Declare(
        rdf4cpp
        GIT_REPOSITORY https://github.com/rdf4cpp/rdf4cpp.git
        GIT_TAG addition/node_equality_operator
        GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(rdf4cpp)

add_subdirectory(libs)

target_link_libraries(tentris
        INTERFACE
        stdc++fs # for #include <filesystem>
        cxxopts::cxxopts
        rapidjson
        fmt::fmt
        hypertrie::hypertrie
        Boost::Boost
        serd-0
        cppitertools::cppitertools
        rdf4cpp::rdf4cpp
        Dice::sparql
        )

target_include_directories(tentris INTERFACE
        # todo: move code to include/Dice/tentris
        src/lib/
        ${Boost_INCLUDE_DIRS}
        )

# for rt and pthread linkage see:
# * https://stackoverflow.com/questions/58848694/gcc-whole-archive-recipe-for-static-linking-to-pthread-stopped-working-in-rec
# * https://stackoverflow.com/questions/35116327/when-g-static-link-pthread-cause-segmentation-fault-why
if (TENTRIS_STATIC)
    set(TENTRIS_STATIC_DEF -static)
    set(TENTRIS_PTHREAD_DEF -Wl,--whole-archive -lrt -lpthread -Wl,--no-whole-archive)
else ()
    set(TENTRIS_PTHREAD_DEF -lpthread)
endif ()


# main executable targets
add_executable(tentris_server src/exec/TentrisServer.cpp src/exec/config/ServerConfig.hpp src/exec/config/TerminalConfig.hpp)
target_link_libraries(tentris_server
        PRIVATE
        ${TENTRIS_STATIC_DEF}
        ${TENTRIS_PTHREAD_DEF}
        tentris
        restinio::restinio
        nonstd::string-view-lite
        nonstd::optional-lite
        nonstd::variant-lite
        )
add_dependencies(tentris_server tentris)

add_executable(tentris_terminal src/exec/TentrisTerminal.cpp src/exec/config/ServerConfig.hpp src/exec/config/TerminalConfig.hpp)
target_link_libraries(tentris_terminal
        ${TENTRIS_STATIC_DEF}
        ${TENTRIS_PTHREAD_DEF}
        tentris
        )
add_dependencies(tentris_terminal tentris)

add_executable(rdf2ids src/exec/tools/RDF2IDs.cpp)
target_link_libraries(rdf2ids
        ${TENTRIS_STATIC_DEF}
        ${TENTRIS_PTHREAD_DEF}
        tentris
        )
add_dependencies(rdf2ids tentris)

add_executable(ids2hypertrie src/exec/tools/IDs2Hypertrie.cpp)
target_link_libraries(ids2hypertrie
        ${TENTRIS_STATIC_DEF}
        ${TENTRIS_PTHREAD_DEF}
        tentris
        vincentlaucsb-csv-parser::vincentlaucsb-csv-parser
        )
add_dependencies(ids2hypertrie tentris)

if (CMAKE_BUILD_TYPE MATCHES "Release")
    set_target_properties(tentris_server PROPERTIES LINK_FLAGS_RELEASE -s)
    set_target_properties(tentris_terminal PROPERTIES LINK_FLAGS_RELEASE -s)
    set_target_properties(rdf2ids PROPERTIES LINK_FLAGS_RELEASE -s)
    set_target_properties(ids2hypertrie PROPERTIES LINK_FLAGS_RELEASE -s)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result LANGUAGES CXX) # fatal error if IPO is not supported
    if ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "13"))
        message("Use full lto for clang >=13 instead of thinlto because otherwise the executable cause a SIGSEGV.")
        add_link_options(tentris_server PRIVATE -flto)
        add_link_options(tentris_terminal PRIVATE -flto)
        add_link_options(ids2hypertrie PRIVATE -flto)
        add_link_options(rdf2ids PRIVATE -flto)
    else ()
        message("INTERPROCEDURAL_OPTIMIZATION enabled.")
        set_property(TARGET tentris_server PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
        set_property(TARGET tentris_terminal PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
        set_property(TARGET ids2hypertrie PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
        set_property(TARGET rdf2ids PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
    endif ()
endif ()

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    include(CTest)
    enable_testing()# enable this to enable testing
    add_subdirectory(tests)
endif ()
