#ifndef TENTRIS_TESTS_TESTS_COMMONS_HPP
#define TENTRIS_TESTS_TESTS_COMMONS_HPP
#include <filesystem>

#include <dice/triple-store/TripleStore.hpp>
#include <dice/sparql2tensor/parser/SPARQLParser.hpp>
#include <dice/node-store/PersistentNodeStorageBackend.hpp>
#include <dice/query.hpp>

namespace dice::tests::sparql {

	void create_metall_db(std::string const &db_path) {
		using metall_manager =  rdf_tensor::metall_manager;
		auto const storage_path = std::filesystem::absolute(db_path);
		{
			metall_manager{metall::create_only, storage_path.c_str()};
		}
	}

	triple_store::TripleStore *init_stores(rdf_tensor::metall_manager &storage_manager,
										   std::string const &path_to_rdf_data) {
		// set up node store
		{
			using namespace rdf4cpp::rdf::storage::node;
			using namespace dice::node_store;
			auto *nodestore_backend = storage_manager.find_or_construct<PersistentNodeStorageBackendImpl>("node-store")(storage_manager.get_allocator());
			NodeStorage::default_instance(
					NodeStorage::new_instance<PersistentNodeStorageBackend>(nodestore_backend));
		}
		// setup triple store
		auto *store = storage_manager.find_or_construct<triple_store::TripleStore>("triple-store")(storage_manager.get_allocator());
		store->load_ttl({path_to_rdf_data});
		return store;
	}

	std::vector<rdf_tensor::Entry> eval_sparql_query(std::string const &query_str,
													 triple_store::TripleStore const &store) {
		// create sparql query
		auto sparql_query = dice::sparql2tensor::parser::SPARQLParser::parse_query(query_str, store);
		// evaluate sparql query
		std::vector<rdf_tensor::Entry> entries{};
		for (auto const &entry : rdf_tensor::QueryEvaluation::evaluate(sparql_query.raw_query())) {
			entries.push_back(entry);
		}
		return entries;
	}

	bool compare_results(std::vector<rdf_tensor::Entry> actual_results,
						 std::vector<rdf_tensor::Entry> expected_results) {
		std::sort(expected_results.begin(), expected_results.end());
		std::sort(actual_results.begin(), actual_results.end());
		return (actual_results == expected_results);
	}

}// namespace dice::tests::sparql

#endif//TENTRIS_TESTS_TESTS_COMMONS_HPP
