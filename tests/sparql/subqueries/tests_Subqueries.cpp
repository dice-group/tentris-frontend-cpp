#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../tests_Commons.hpp"

#include <doctest/doctest.h>
namespace dice::tests::sparql {

	using namespace rdf4cpp::rdf;

	/* SPARQL Subquery Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql11/data-sparql11/subquery) */
	TEST_SUITE("SPARQL Subqueries") {

		const std::string db_path = "test_db";

		TEST_CASE("Query: sq06") {
			const std::string path_to_data = "../sparql/subqueries/sq05.ttl";
			const std::string sparql_str = "prefix ex:\t<http://www.example.org/schema#>\n"
										   "prefix in:\t<http://www.example.org/instance#>\n"
										   "\n"
										   "select ?x\n"
										   "where {\n"
										   "{select * where {?x ?p ?y}}\n"
										   "}";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);

			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({rdf4cpp::rdf::IRI("http://www.example.org/instance#c")}),
					rdf_tensor::Entry({rdf4cpp::rdf::IRI("http://www.example.org/instance#a")})
			};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: sq09") {
			const std::string path_to_data = "../sparql/subqueries/sq09.ttl";
			const std::string sparql_str = "prefix ex:\t<http://www.example.org/schema#>\n"
										   "prefix in:\t<http://www.example.org/instance#>\n"
										   "\n"
										   "select * where {\n"
										   "\n"
										   "{select * where { \n"
										   "  {select ?x where {?x ex:q ?t}}\n"
										   "}}\n"
										   "\n"
										   "?x ex:p ?y \n"
										   "}";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);

			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({rdf4cpp::rdf::IRI("http://www.example.org/instance#b"),
									   rdf4cpp::rdf::IRI("http://www.example.org/instance#a")})
			};
			CHECK(compare_results(actual_results, expected_results));
		}

	}

}// namespace dice::tests::sparql