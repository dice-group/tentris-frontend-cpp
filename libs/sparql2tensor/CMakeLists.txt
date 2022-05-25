set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

find_package(sparql-parser-base REQUIRED)
find_package(robin_hood REQUIRED)
find_package(dice-hash REQUIRED)

add_library(sparql2tensor
        src/Dice/sparql2tensor/parser/visitors/PrologueVisitor.cpp
        src/Dice/sparql2tensor/parser/visitors/SelectQueryVisitor.cpp
        src/Dice/sparql2tensor/SPARQLQuery.cpp
        )

add_library(Dice::sparql2tensor ALIAS sparql2tensor)

target_link_libraries(sparql2tensor PUBLIC
        rdf_tensor
        robin_hood::robin_hood
        PRIVATE
        sparql-parser-base::sparql-parser-base
        )

target_include_directories(sparql2tensor PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
        private-include
        )


install(TARGETS sparql2tensor
        EXPORT sparql2tensor_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(EXPORT sparql2tensor_Targets
        FILE sparql2tensorTargets.cmake
        NAMESPACE Dice::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/sparql2tensor/cmake)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

