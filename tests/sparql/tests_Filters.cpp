#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "tests_Commons.hpp"

#include <doctest/doctest.h>

namespace dice::tests::sparql {

	/* SPARQL FILTER Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql11/data-r2/algebra) */
	TEST_SUITE("SPARQL Filter Queries") {
		const std::string db_path = "test_db";
		const std::string const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql11/data-r2/algebra/";

		GENERATE_SPARQL_TEST_CASE(const_url, "data-2.ttl", "filter-placement-1.rq", "filter-placement-1.srx");
		GENERATE_SPARQL_TEST_CASE(const_url, "data-2.ttl", "filter-placement-2.rq", "filter-placement-2.srx");
		GENERATE_SPARQL_TEST_CASE(const_url, "data-2.ttl", "filter-placement-3.rq", "filter-placement-3.srx");
		GENERATE_SPARQL_TEST_CASE(const_url, "data-1.ttl", "filter-nested-1.rq", "filter-nested-1.srx");
		// the filter expression in fitler-nested-2 is not safe
		// GENERATE_SPARQL_TEST_CASE(const_url, "data-1.ttl", "filter-nested-2.rq", "filter-nested-2.srx");
	}

}// namespace dice::tests::sparql