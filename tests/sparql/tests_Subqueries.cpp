#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "tests_Commons.hpp"
#include <doctest/doctest.h>


namespace dice::tests::sparql {

	/* SPARQL Subquery Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql11/data-sparql11/subquery) */
	TEST_SUITE("SPARQL Subqueries") {

		const std::string db_path = "test_db";
		const std::string const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql11/data-sparql11/subquery/";

		// # https://github.com/w3c/rdf-tests/blob/main/sparql11/data-sparql11/subquery/sq05.rdf
		const std::string sq05_ttl = "@prefix ex: <http://www.example.org/schema#> .\n"
									 "\n"
									 "<http://www.example.org/instance#a> ex:p <http://www.example.org/instance#b> .\n"
									 "<http://www.example.org/instance#c> ex:p \"\" .";
		// # https://github.com/w3c/rdf-tests/blob/main/sparql11/data-sparql11/subquery/sq08.rdf
		const std::string sq08_ttl = "@prefix ex: <http://www.example.org/schema#> .\n"
									 "\n"
									 "<http://www.example.org/instance#a> ex:p 1, 2 .\n"
									 "<http://www.example.org/instance#b> ex:p 3 .";
		// # https://github.com/w3c/rdf-tests/blob/main/sparql11/data-sparql11/subquery/sq09.rdf
		const std::string sq09_ttl = "@prefix ex: <http://www.example.org/schema#> .\n"
									 "\n"
									 "<http://www.example.org/instance#a> ex:p <http://www.example.org/instance#b> ; ex:q <http://www.example.org/instance#c> .\n"
									 "\n"
									 "<http://www.example.org/instance#d> ex:p <http://www.example.org/instance#e> .";

		GENERATE_SPARQL_TEST_CASE(const_url, sq05_ttl, "sq06.rq", "sq06.srx", true);
		GENERATE_SPARQL_TEST_CASE(const_url, sq08_ttl, "sq08.rq", "sq08.srx", true);
		GENERATE_SPARQL_TEST_CASE(const_url, sq09_ttl, "sq09.rq", "sq09.srx", true);
	}

}// namespace dice::tests::sparql