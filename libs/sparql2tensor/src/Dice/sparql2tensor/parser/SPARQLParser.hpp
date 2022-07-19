#ifndef DICE_SPARQL_SPARQLPARSER_HPP
#define DICE_SPARQL_SPARQLPARSER_HPP

#include "Dice/sparql2tensor/SPARQLQuery.hpp"

namespace Dice::sparql2tensor::parser {

	class SPARQLParser {
	public:

		static SPARQLQuery parse_query(std::string const &sparql_query_str, triple_store::TripleStore const &triple_store);

		static void parse_update(std::string const &sparql_query_str, triple_store::TripleStore const &triple_store);

	};

}

#endif//DICE_SPARQL_SPARQLPARSER_HPP
