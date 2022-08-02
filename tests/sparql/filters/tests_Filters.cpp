#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../tests_Commons.hpp"

#include <doctest/doctest.h>

namespace dice::tests::sparql {

	/* SPARQL FILTER Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql11/data-r2/algebra) */
	TEST_SUITE("SPARQL Aggregate Queries") {

		const std::string db_path = "test_db";

		TEST_CASE("Query: filter-placement-1") {
			const std::string path_to_data = "../sparql/filters/data-2.ttl";
			const std::string sparql_str = "PREFIX : <http://example/>\n"
										   "\n"
										   "SELECT ?v \n"
										   "{ \n"
										   "    ?s :p ?v . \n"
										   "    FILTER (?v = 2)\n"
										   "}";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);

			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({rdf4cpp::rdf::Literal("2", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#integer"))})
			};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: filter-placement-2") {
			const std::string path_to_data = "../sparql/filters/data-2.ttl";
			const std::string sparql_str = "PREFIX : <http://example/>\n"
										   "\n"
										   "SELECT ?v \n"
										   "{ \n"
										   "    FILTER (?v = 2)\n"
										   "    ?s :p ?v . \n"
										   "}";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);

			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({rdf4cpp::rdf::Literal("2", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#integer"))})
			};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: filter-placement-3") {
			const std::string path_to_data = "../sparql/filters/data-2.ttl";
			const std::string sparql_str = "\n"
										   "PREFIX : <http://example/>\n"
										   "\n"
										   "SELECT ?v ?w\n"
										   "{ \n"
										   "    FILTER (?v = 2)\n"
										   "    FILTER (?w = 3)\n"
										   "    ?s :p ?v . \n"
										   "    ?s :q ?w .\n"
										   "}";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);

			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({rdf4cpp::rdf::Literal("2", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#integer")),
									   rdf4cpp::rdf::Literal("3", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#integer"))})
			};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: filter-nested-1") {
			const std::string path_to_data = "../sparql/filters/data-1.ttl";
			const std::string sparql_str = "PREFIX : <http://example/> \n"
										   "\n"
										   "SELECT ?v\n"
										   "{ :x :p ?v . FILTER(?v = 1) }";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);

			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({rdf4cpp::rdf::Literal("1", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#integer"))})
			};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: filter-nested-2") {
			MESSAGE("FILTER expression is not safe!"); return;
			const std::string path_to_data = "../sparql/filters/data-1.ttl";
			const std::string sparql_str = "\n"
										   "PREFIX : <http://example/> \n"
										   "\n"
										   "SELECT ?v\n"
										   "{ :x :p ?v . { FILTER(?v = 1) } }";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);

			std::vector<rdf_tensor::Entry> expected_results{};
			CHECK(compare_results(actual_results, expected_results));
		}

	}

}// namespace dice::tests::sparql
