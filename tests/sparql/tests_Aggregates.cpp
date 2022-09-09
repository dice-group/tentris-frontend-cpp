#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "tests_Commons.hpp"

#include <doctest/doctest.h>

namespace dice::tests::sparql {

	/* SPARQL Aggregate Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql11/data-sparql11/aggregates) */
	TEST_SUITE("SPARQL Aggregate Queries") {
		const std::string db_path = "test_db";
		const std::string const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql11/data-sparql11/aggregates/";

		GENERATE_SPARQL_TEST_CASE(const_url, "agg01.ttl", "agg01.rq", "agg01.srx", false);
		GENERATE_SPARQL_TEST_CASE(const_url, "agg01.ttl", "agg02.rq", "agg02.srx", false);
		// agg03 contains HAVING; not supported yet
		// GENERATE_SPARQL_TEST_CASE(const_url, "agg01.ttl", "agg03.rq", "agg03.srx", false);
		GENERATE_SPARQL_TEST_CASE(const_url, "agg01.ttl", "agg04.rq", "agg04.srx", false);
		GENERATE_SPARQL_TEST_CASE(const_url, "agg01.ttl", "agg05.rq", "agg05.srx", false);
		// agg06 contains HAVING; not supported yet
		// GENERATE_SPARQL_TEST_CASE(const_url, "agg01.ttl", "agg06.rq", "agg06.srx", false);
		// agg07 contains HAVING; not supported yet
		// GENERATE_SPARQL_TEST_CASE(const_url, "agg01.ttl", "agg07.rq", "agg07.srx", false);
		// agg08 contains ORDER BY; not supported yet
		// GENERATE_SPARQL_TEST_CASE(const_url, "agg08.ttl", "agg08.rq", "agg08.srx", false);
		GENERATE_SPARQL_TEST_CASE_PARSE_EXCEPTION(const_url, "agg08.ttl", "agg09.rq", "Variable ?P is not part of the group key", false);
		GENERATE_SPARQL_TEST_CASE_PARSE_EXCEPTION(const_url, "agg08.ttl", "agg10.rq", "Variable ?P is not part of the group key", false);
		// agg11 contains expression in the GROUP BY clause; not supported yet
		// GENERATE_SPARQL_TEST_CASE_PARSE_EXCEPTION(const_url, "agg08.ttl", "agg11.rq", "Variable ?O1 is not part of the group key");
		GENERATE_SPARQL_TEST_CASE(const_url, "agg-numeric.ttl", "agg-max-01.rq", "agg-max-01.srx", false);
		// agg-max-02 fails due to false comparisons in rdf4cpp
		// GENERATE_SPARQL_TEST_CASE(const_url, "agg-numeric.ttl", "agg-max-02.rq", "agg-max-02.srx", false);
		GENERATE_SPARQL_TEST_CASE(const_url, "agg-numeric.ttl", "agg-min-01.rq", "agg-min-01.srx", false);
		// agg-min-02 fails due to false comparisons in rdf4cpp
		//GENERATE_SPARQL_TEST_CASE(const_url, "agg-numeric.ttl", "agg-min-02.rq", "agg-min-02.srx", false);
		GENERATE_SPARQL_TEST_CASE(const_url, "agg-numeric.ttl", "agg-sample-01.rq", "agg-sample-01.srx", false);
	}

}// namespace dice::tests::sparql