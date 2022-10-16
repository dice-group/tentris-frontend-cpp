#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "tests_Commons.hpp"

#include <doctest/doctest.h>

namespace dice::tests::sparql {

	/* SPARQL Inline Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql11/data-sparql11/bindings) */
	TEST_SUITE("SPARQL Inline Queries") {
		const std::string db_path = "test_db";
		const std::string const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql11/data-sparql11/bindings/";

		GENERATE_SPARQL_TEST_CASE(const_url, "data01.ttl", "inline01.rq", "inline01.srx", false);
		// currently not supported -- ambiguity in the grammar (values clause vs inline data)?
		// GENERATE_SPARQL_TEST_CASE(const_url, "data01.ttl", "inline02.rq", "inline02.srx", false);
	}

}// namespace dice::tests::sparql