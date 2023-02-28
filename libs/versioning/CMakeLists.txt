# Define the library
add_library(versioning INTERFACE)
add_library(tentris::versioning ALIAS versioning)

#target_link_libraries(rdf-tensor INTERFACE
#        rdf4cpp::rdf4cpp
#        hypertrie::hypertrie
#        Metall::Metall
#        Boost::headers
#        dice-hash::dice-hash # todo: shouldn't this come with hypertrie?
#        )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/dice/versioning/version.hpp)
target_include_directories(versioning INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        )

include(${PROJECT_SOURCE_DIR}/cmake/install_library.cmake)
install_interface_component(versioning src)