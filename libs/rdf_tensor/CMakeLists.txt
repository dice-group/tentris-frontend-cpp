# Find cmake packages
find_package(Boost REQUIRED)
find_package(hypertrie REQUIRED)
find_package(Metall REQUIRED)
find_package(rdf4cpp REQUIRED)
find_package(dice-hash REQUIRED)

# Define the library
add_library(rdf_tensor INTERFACE)
add_library(tentris::rdf_tensor ALIAS rdf_tensor)
target_link_libraries(rdf_tensor INTERFACE
        rdf4cpp::rdf4cpp
        hypertrie::hypertrie
        Metall::Metall
        Boost::headers
        dice-hash::dice-hash # todo: shouldn't this come with hypertrie?
        )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/Dice/tentris/tentris_version.hpp)
target_include_directories(rdf_tensor INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        )

include(${PROJECT_SOURCE_DIR}/cmake/install_library.cmake)
install_interface_component(rdf_tensor src)