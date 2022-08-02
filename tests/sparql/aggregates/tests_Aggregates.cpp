#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../tests_Commons.hpp"

#include <doctest/doctest.h>
namespace dice::tests::sparql {

	using namespace rdf4cpp::rdf;

	/* SPARQL Aggregate Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql11/data-sparql11/aggregates) */
	TEST_SUITE("SPARQL Aggregate Queries") {

		std::string db_path = "test_db";

		TEST_CASE("Query: agg01") {
			const std::string path_to_data = "../sparql/aggregates/agg01.ttl";
			const std::string sparql_str = "PREFIX : <http://www.example.org> SELECT (COUNT(?O) AS ?C) WHERE { ?S ?P ?O }";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);

			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({rdf4cpp::rdf::Literal("5", rdf4cpp::rdf::IRI("http://www.w3.org/2001/XMLSchema#integer"))})};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: agg02") {
			const std::string path_to_data = "../sparql/aggregates/agg01.ttl";
			const std::string sparql_str = "PREFIX : <http://www.example.org>"
										   "SELECT ?P (COUNT(?O) AS ?C) WHERE { ?S ?P ?O }"
										   "GROUP BY ?P";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);

			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({IRI("http://www.example.org/p1"), Literal("3", IRI("http://www.w3.org/2001/XMLSchema#integer"))}),
					rdf_tensor::Entry({IRI("http://www.example.org/p2"), Literal("2", IRI("http://www.w3.org/2001/XMLSchema#integer"))})};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: agg03") {
			MESSAGE("HAVING NOT SUPPORTED YET");
			return;
			const std::string path_to_data = "../sparql/aggregates/agg01.ttl";
			const std::string sparql_str = "PREFIX : <http://www.example.org>"
										   "SELECT ?P (COUNT(?O) AS ?C) WHERE { ?S ?P ?O }"
										   "GROUP BY ?P"
										   "HAVING (COUNT(?O) > 2 )";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);

			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({IRI("http://www.example.org/p1"), Literal("3", IRI("http://www.w3.org/2001/XMLSchema#integer"))})};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: agg04") {
			const std::string path_to_data = "../sparql/aggregates/agg01.ttl";
			const std::string sparql_str = "PREFIX : <http://www.example.org>"
										   "SELECT (COUNT(*) AS ?C) WHERE { ?S ?P ?O }";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);

			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({Literal("5", IRI("http://www.w3.org/2001/XMLSchema#integer"))})};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: agg05") {
			const std::string path_to_data = "../sparql/aggregates/agg01.ttl";
			const std::string sparql_str = "PREFIX : <http://www.example.org>"
										   "SELECT ?P (COUNT(*) AS ?C) WHERE { ?S ?P ?O } GROUP BY ?P";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);

			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({IRI("http://www.example.org/p1"), Literal("3", IRI("http://www.w3.org/2001/XMLSchema#integer"))}),
					rdf_tensor::Entry({IRI("http://www.example.org/p2"), Literal("2", IRI("http://www.w3.org/2001/XMLSchema#integer"))})};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: agg06") {
			MESSAGE("HAVING NOT SUPPORTED YET");
			return;
			const std::string path_to_data = "../sparql/aggregates/agg01.ttl";
			const std::string sparql_str = "PREFIX : <http://www.example.org> "
										   "SELECT (COUNT(*) AS ?C) WHERE { ?S ?P ?O } HAVING (COUNT(*) > 0 )";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);

			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({Literal("5", IRI("http://www.w3.org/2001/XMLSchema#integer"))})};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: agg07") {
			MESSAGE("HAVING NOT SUPPORTED YET");
			return;
			const std::string path_to_data = "../sparql/aggregates/agg01.ttl";
			const std::string sparql_str = "PREFIX : <http://www.example.org> "
										   "SELECT ?P (COUNT(*) AS ?C) "
										   "WHERE { ?S ?P ?O } "
										   "GROUP BY ?P "
										   "HAVING ( COUNT(*) > 2 )";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);

			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({IRI("http://www.example.org/p1"), Literal("3", IRI("http://www.w3.org/2001/XMLSchema#integer"))})};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: agg08") {
			const std::string path_to_data = "../sparql/aggregates/agg08.ttl";
			const std::string sparql_str = "PREFIX : <http://www.example.org/> "
										   "SELECT ((?O1 + ?O2) AS ?O12) (COUNT(?O1) AS ?C) "
										   "WHERE { ?S :p ?O1; :q ?O2 } GROUP BY (?O1 + ?O2) "
										   "ORDER BY ?O12";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			CHECK_THROWS_WITH(eval_sparql_query(sparql_str, *store), "Variable ?O1 is not part of the group key.");
		}

		TEST_CASE("Query: agg08") {
			MESSAGE("ORDER BY NOT SUPPORTED YET");
			return;
			const std::string path_to_data = "../sparql/aggregates/agg08.ttl";
			const std::string sparql_str = "PREFIX : <http://www.example.org/> "
										   "SELECT ?O12 (COUNT(?O1) AS ?C) "
										   "WHERE { ?S :p ?O1; :q ?O2 } GROUP BY ((?O1 + ?O2) AS ?O12) "
										   "ORDER BY ?O12";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			std::vector<rdf_tensor::Entry> actual_results = eval_sparql_query(sparql_str, *store);

			std::vector<rdf_tensor::Entry> expected_results{
					rdf_tensor::Entry({Literal("0", IRI("http://www.w3.org/2001/XMLSchema#integer")),
									   Literal("1", IRI("http://www.w3.org/2001/XMLSchema#integer"))}),
					rdf_tensor::Entry({Literal("1", IRI("http://www.w3.org/2001/XMLSchema#integer")),
									   Literal("2", IRI("http://www.w3.org/2001/XMLSchema#integer"))}),
					rdf_tensor::Entry({Literal("2", IRI("http://www.w3.org/2001/XMLSchema#integer")),
									   Literal("3", IRI("http://www.w3.org/2001/XMLSchema#integer"))}),
					rdf_tensor::Entry({Literal("3", IRI("http://www.w3.org/2001/XMLSchema#integer")),
									   Literal("2", IRI("http://www.w3.org/2001/XMLSchema#integer"))}),
					rdf_tensor::Entry({Literal("4", IRI("http://www.w3.org/2001/XMLSchema#integer")),
									   Literal("1", IRI("http://www.w3.org/2001/XMLSchema#integer"))}),
			};
			CHECK(compare_results(actual_results, expected_results));
		}

		TEST_CASE("Query: agg09") {
			const std::string path_to_data = "../sparql/aggregates/agg08.ttl";
			const std::string sparql_str = "PREFIX : <http://www.example.org/> "
										   "SELECT ?P (COUNT(?O) AS ?C) "
										   "WHERE { ?S ?P ?O } GROUP BY ?S";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			CHECK_THROWS_WITH(eval_sparql_query(sparql_str, *store), "Variable ?P is not part of the group key");
		}

		TEST_CASE("Query: agg10") {
			const std::string path_to_data = "../sparql/aggregates/agg08.ttl";
			const std::string sparql_str = "PREFIX : <http://www.example.org/> "
										   "SELECT ?P (COUNT(?O) AS ?C) "
										   "WHERE { ?S ?P ?O }";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			CHECK_THROWS_WITH(eval_sparql_query(sparql_str, *store), "Variable ?P is not part of the group key");
		}

		TEST_CASE("Query: agg11") {
			const std::string path_to_data = "../sparql/aggregates/agg08.ttl";
			const std::string sparql_str = "PREFIX : <http://www.example.org/> "
										   "SELECT ((?O1 + ?O2) AS ?O12) (COUNT(?O1) AS ?C) "
										   "WHERE { ?S :p ?O1; :q ?O2 } GROUP BY (?S)";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			CHECK_THROWS_WITH(eval_sparql_query(sparql_str, *store), "Variable ?O1 is not part of the group key");
		}

		TEST_CASE("Query: agg12") {
			const std::string path_to_data = "../sparql/aggregates/agg08.ttl";
			const std::string sparql_str = "PREFIX : <http://www.example.org/> "
										   "SELECT ?O1 (COUNT(?O2) AS ?C) "
										   "WHERE { ?S :p ?O1; :q ?O2 } GROUP BY (?O1 + ?O2)";

			create_metall_db(db_path);
			rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};
			auto *store = init_stores(storage_manager, path_to_data);
			CHECK_THROWS_WITH(eval_sparql_query(sparql_str, *store), "Variable ?O1 is not part of the group key");
		}
	}

}// namespace dice::tests::sparql