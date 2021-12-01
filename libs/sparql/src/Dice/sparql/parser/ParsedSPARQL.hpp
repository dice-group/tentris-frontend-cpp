#ifndef DICE_SPARQL_PARSEDSPARQL_HPP
#define DICE_SPARQL_PARSEDSPARQL_HPP

#include <rdf4cpp/rdf.hpp>

#include <Dice/query/OperandDependencyGraph.hpp>

namespace Dice::sparql::parser {

	struct ParsedSPARQL {

		Dice::query::OperandDependencyGraph odg;

		std::vector<rdf4cpp::rdf::query::Variable> projected_variables;

		std::map<rdf4cpp::rdf::query::Variable, char> var_to_id;

		std::vector<rdf4cpp::rdf::query::TriplePattern> triple_patterns;

		std::map<std::string, std::string> prefixes;

		bool distinct = false;

	};

}// namespace Dice::sparql::parser

#endif//DICE_SPARQL_PARSEDSPARQL_HPP