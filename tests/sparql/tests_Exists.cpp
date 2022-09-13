#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "tests_Commons.hpp"

#include <doctest/doctest.h>

namespace dice::tests::sparql {

	/* SPARQL EXISTS Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql11/data-sparql11/exists) */
	TEST_SUITE("SPARQL EXISTS Queries") {
		const std::string db_path = "test_db";
		const std::string const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql11/data-sparql11/exists/";

		GENERATE_SPARQL_TEST_CASE(const_url, "exists01.ttl", "exists01.rq", "exists01.srx");
		GENERATE_SPARQL_TEST_CASE(const_url, "exists01.ttl", "exists02.rq", "exists02.srx");
		GENERATE_SPARQL_TEST_CASE(const_url, "exists01.ttl", "exists04.rq", "exists04.srx");
		GENERATE_SPARQL_TEST_CASE(const_url, "exists01.ttl", "exists05.rq", "exists05.srx");
	}

}// namespace dice::tests::sparql