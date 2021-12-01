set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

add_library(sparql
        src/Dice/sparql/parser/visitors/PrologueVisitor.cpp
        src/Dice/sparql/parser/visitors/SelectQueryVisitor.cpp
        )

add_library(Dice::sparql ALIAS sparql)

find_package(sparql-parser-base REQUIRED)
find_package(hypertrie REQUIRED)

target_link_libraries(sparql PUBLIC
        rdf4cpp::rdf4cpp
        hypertrie::hypertrie
        sparql-parser-base::sparql-parser-base
        )

target_include_directories(sparql PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )
