#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "tests_Commons.hpp"
#include <doctest/doctest.h>


namespace dice::tests::sparql {

	/* SPARQL Functions Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql11/data-sparql11/functions) */
	TEST_SUITE("SPARQL Function Queries") {

		const std::string db_path = "test_db";
		const std::string const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql11/data-sparql11/functions/";

		/* STRLANG, LANGMATCHES and LANG test cases */
		GENERATE_SPARQL_TEST_CASE(const_url, "data.ttl", "strlang01.rq", "strlang01.srx");
		GENERATE_SPARQL_TEST_CASE(const_url, "data.ttl", "strlang02.rq", "strlang02.srx");
		// strlang03 fails due to the language tags not being normalized in rdf4cpp (currently en-us != en-US; should be en-us == en-US)
		// GENERATE_SPARQL_TEST_CASE(const_url, "data.ttl", "strlang03.rq", "strlang03-rdf11.srx");
		/* STRSTARTS */
		GENERATE_SPARQL_TEST_CASE(const_url, "data.ttl", "starts01.rq", "starts01.srx");
		/* STRENDS */
		GENERATE_SPARQL_TEST_CASE(const_url, "data.ttl", "ends01.rq", "ends01.srx");
		/* CONTAINS */
		GENERATE_SPARQL_TEST_CASE(const_url, "data.ttl", "contains01.rq", "contains01.srx");
	}

}// namespace dice::tests::sparql