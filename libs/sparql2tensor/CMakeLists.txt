set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)

add_library(sparql2tensor
        src/Dice/sparql2tensor/expressions/UnaryOperators.cpp
        src/Dice/sparql2tensor/expressions/BinaryOperators.cpp
        src/Dice/sparql2tensor/expressions/PrimaryExpressions.cpp
        src/Dice/sparql2tensor/expressions/Aggregates.cpp
        src/Dice/sparql2tensor/expressions/Expression.cpp
        src/Dice/sparql2tensor/parser/visitors/PrologueVisitor.cpp
        src/Dice/sparql2tensor/parser/visitors/SelectAskQueryVisitor.cpp
        src/Dice/sparql2tensor/SPARQLQuery.cpp
        )

add_library(Dice::sparql2tensor ALIAS sparql2tensor)

find_package(sparql-parser-base REQUIRED)
find_package(robin_hood REQUIRED)


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
