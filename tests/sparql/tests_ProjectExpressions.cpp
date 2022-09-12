#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "tests_Commons.hpp"
#include <doctest/doctest.h>


namespace dice::tests::sparql {

	/* SPARQL Functions Test Queries (https://github.com/w3c/rdf-tests/blob/main/sparql11/data-sparql11/project-expression/) */
	TEST_SUITE("SPARQL Queries with Expressions in Projections") {

		const std::string db_path = "test_db";
		const std::string const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql11/data-sparql11/project-expression/";

		GENERATE_SPARQL_TEST_CASE(const_url, "projexp01.ttl", "projexp01.rq", "projexp01.srx");
		//GENERATE_SPARQL_TEST_CASE(const_url, "projexp02.ttl", "projexp02.rq", "projexp02.srx");
		//GENERATE_SPARQL_TEST_CASE(const_url, "projexp03.ttl", "projexp03.rq", "projexp03.srx");
		//GENERATE_SPARQL_TEST_CASE(const_url, "projexp04.ttl", "projexp04.rq", "projexp04.srx");
		GENERATE_SPARQL_TEST_CASE(const_url, "projexp05.ttl", "projexp05.rq", "projexp05.srx");
		GENERATE_SPARQL_TEST_CASE(const_url, "projexp06.ttl", "projexp06.rq", "projexp06.srx");
		GENERATE_SPARQL_TEST_CASE(const_url, "projexp07.ttl", "projexp07.rq", "projexp07.srx");
	}

}// namespace dice::tests::sparql