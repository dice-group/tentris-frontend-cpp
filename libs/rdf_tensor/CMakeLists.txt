set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

add_library(rdf_tensor INTERFACE)

add_library(Dice::rdf_tensor ALIAS rdf_tensor)


find_package(hypertrie REQUIRED)
find_package(Boost REQUIRED)

include(FetchContent)
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

install(TARGETS rdf_tensor
        EXPORT rdf_tensor_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(EXPORT rdf_tensor_Targets
        FILE rdf_tensorTargets.cmake
        NAMESPACE Dice::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/rdf_tensor/cmake)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

