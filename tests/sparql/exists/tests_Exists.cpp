#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../tests_Commons.hpp"

#include <doctest/doctest.h>

namespace dice::tests::sparql {

	/* SPARQL EXISTS Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql11/data-sparql11/exists) */
	TEST_SUITE("SPARQL Exists Queries") {

		const std::string db_path = "test_db";

		TEST_CASE("Query: exists01") {
			const std::string path_to_data = "../sparql/exists/exists01.ttl";
			const std::string sparql_str = "prefix ex: <http://www.example.org/>\n"
										   "\n"
										   "select * where {\n"
										   "?s ?p ?o\n"
										   "filter exists {?s ?p ex:o}\n"
										   "}";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);


			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({rdf4cpp::rdf::IRI("http://www.example.org/o"),
					                   rdf4cpp::rdf::IRI("http://www.example.org/p"),
					                   rdf4cpp::rdf::IRI("http://www.example.org/s")}),
					rdf_tensor::Entry({rdf4cpp::rdf::IRI("http://www.example.org/o1"),
									   rdf4cpp::rdf::IRI("http://www.example.org/p"),
									   rdf4cpp::rdf::IRI("http://www.example.org/s")}),
					rdf_tensor::Entry({rdf4cpp::rdf::IRI("http://www.example.org/o2"),
									   rdf4cpp::rdf::IRI("http://www.example.org/p"),
									   rdf4cpp::rdf::IRI("http://www.example.org/s")})
			};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: exists02") {
			const std::string path_to_data = "../sparql/exists/exists01.ttl";
			const std::string sparql_str = "prefix ex: <http://www.example.org/>\n"
										   "\n"
										   "select * where {\n"
										   "?s ?p ex:o2\n"
										   "filter exists {ex:s ex:p ex:o}\n"
										   "}";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);


			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({rdf4cpp::rdf::IRI("http://www.example.org/p"),
									   rdf4cpp::rdf::IRI("http://www.example.org/t")}),
					rdf_tensor::Entry({rdf4cpp::rdf::IRI("http://www.example.org/p"),
									   rdf4cpp::rdf::IRI("http://www.example.org/s")}),
			};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: exists04") {
			const std::string path_to_data = "../sparql/exists/exists01.ttl";
			const std::string sparql_str = "prefix ex: <http://www.example.org/>\n"
										   "\n"
										   "select * where {\n"
										   "  ?s ?p ex:o\n"
										   "  filter exists { ?s ?p ex:o1  filter exists { ?s ?p ex:o2 } } \n"
										   "}";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);


			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({rdf4cpp::rdf::IRI("http://www.example.org/p"),
									   rdf4cpp::rdf::IRI("http://www.example.org/s")})
			};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: exists05") {
			const std::string path_to_data = "../sparql/exists/exists01.ttl";
			const std::string sparql_str = "prefix ex: <http://www.example.org/>\n"
										   "\n"
										   "select * where {\n"
										   "  ?s ?p ex:o\n"
										   "  filter exists { ?s ?p ex:o1  filter not exists { ?s ?p ex:o2 } } \n"
										   "}";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);
			for (auto const &res : actual_results) {
				std::cout << res[0] << " " << res[1] << std::endl;
			}


			std::vector<rdf_tensor::Entry> expected_results{};
			CHECK(compare_results(actual_results, expected_results));
		}

	}

}