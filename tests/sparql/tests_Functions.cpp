#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "tests_Commons.hpp"
#include <doctest/doctest.h>


namespace dice::tests::sparql {

	/* SPARQL Functions Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql11/data-sparql11/functions) */
	TEST_SUITE("SPARQL Function Queries") {

		const std::string db_path = "test_db";
		const std::string const_url_1 = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql11/data-sparql11/functions/";
		const std::string const_url_2 = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql11/data-r2/bound/";

		/* STRLANG, LANGMATCHES and LANG test cases */
		GENERATE_SPARQL_TEST_CASE(const_url_1, "data.ttl", "strlang01.rq", "strlang01.srx", false);
		GENERATE_SPARQL_TEST_CASE(const_url_1, "data.ttl", "strlang02.rq", "strlang02.srx", false);
		// strlang03 fails due to the language tags not being normalized in rdf4cpp (currently en-us != en-US; should be en-us == en-US)
		// GENERATE_SPARQL_TEST_CASE(const_url, "data.ttl", "strlang03.rq", "strlang03-rdf11.srx", false);
		/* STRSTARTS */
		GENERATE_SPARQL_TEST_CASE(const_url_1, "data.ttl", "starts01.rq", "starts01.srx", false);
		/* STRENDS */
		GENERATE_SPARQL_TEST_CASE(const_url_1, "data.ttl", "ends01.rq", "ends01.srx", false);
		/* CONTAINS */
		GENERATE_SPARQL_TEST_CASE(const_url_1, "data.ttl", "contains01.rq", "contains01.srx", false);
		/* BOUND */
		// todo: update test_commons to handle static results (for test cases whose results are not in xml format)
//		const std::string sq05_ttl = "@prefix : <http://example.org/ns#> ."
//									 ":a1 :b :c1 . \n"
//									 ":a2 :b :c2 .\n"
//									 ":c2 :b :f .";
//		std::vector<std::map<rdf4cpp::rdf::query::Variable, rdf_tensor::NodeWrapper>> bound1_expected_results{
//				{{{rdf4cpp::rdf::query::Variable("a"), rdf4cpp::rdf::IRI("http://example.org/ns#c2")},
//				  {rdf4cpp::rdf::query::Variable("c"), rdf4cpp::rdf::IRI("http://example.org/ns#f")}},
//				 {{rdf4cpp::rdf::query::Variable("a"), rdf4cpp::rdf::IRI("http://example.org/ns#a2")},
//				  {rdf4cpp::rdf::query::Variable("c"), rdf4cpp::rdf::IRI("http://example.org/ns#c2")}}}};
//
//		GENERATE_SPARQL_TEST_CASE_STATIC_RESULTS(const_url_2, "data.ttl", "bound1.rq", {}, false);
	}

}// namespace dice::tests::sparql