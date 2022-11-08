#ifndef TENTRIS_TESTS_TESTS_COMMONS_HPP
#define TENTRIS_TESTS_TESTS_COMMONS_HPP
#include <filesystem>

#include <dice/node-store/PersistentNodeStorageBackend.hpp>
#include <dice/query.hpp>
#include <dice/sparql2tensor/parser/SPARQLParser.hpp>
#include <dice/triple-store/TripleStore.hpp>

#include <curl/curl.h>
#include <pugixml.hpp>

namespace dice::tests::sparql {

#define GENERATE_SPARQL_TEST_CASE(const_url, data, query, result, static_data)              \
	TEST_CASE(query) {                                                                      \
		const std::string data_url = (const_url) + (data);                                  \
		const std::string query_url = (const_url) + (query);                                \
		const std::string result_url = (const_url) + (result);                              \
		const std::string rdf_data = (static_data) ? (data) : read_file_from_url(data_url); \
		create_metall_db(db_path);                                                          \
		rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};     \
		auto *store = init_stores(storage_manager, rdf_data);                               \
		auto sparql_str = read_file_from_url(query_url);                                    \
		auto expected_results = parse_sparql_result_file(read_file_from_url(result_url));   \
		auto actual_results = eval_sparql_query(sparql_str, *store);                        \
		CHECK(compare_results(actual_results, expected_results));                           \
	}

#define GENERATE_SPARQL_TEST_CASE_PARSE_EXCEPTION(const_url, data, query, exception, static_data) \
	TEST_CASE(query) {                                                                            \
		const std::string data_url = (const_url) + (data);                                        \
		const std::string query_url = (const_url) + (query);                                      \
		const std::string rdf_data = (static_data) ? (data) : read_file_from_url(data_url);       \
		create_metall_db(db_path);                                                                \
		rdf_tensor::metall_manager storage_manager{metall::open_only, db_path.c_str()};           \
		auto *store = init_stores(storage_manager, rdf_data);                                     \
		auto sparql_str = read_file_from_url(query_url);                                          \
		CHECK_THROWS_WITH(eval_sparql_query(sparql_str, *store), exception);                      \
	}// namespace dice::tests::sparql


	void create_metall_db(std::string const &db_path) {
		using metall_manager = rdf_tensor::metall_manager;
		auto const storage_path = std::filesystem::absolute(db_path);
		{
			metall_manager{metall::create_only, storage_path.c_str()};
		}
	}

	triple_store::TripleStore *init_stores(rdf_tensor::metall_manager &storage_manager,
										   std::string const &rdf_data) {
		// set up node store
		{
			using namespace rdf4cpp::rdf::storage::node;
			using namespace dice::node_store;
			auto *nodestore_backend = storage_manager.find_or_construct<PersistentNodeStorageBackendImpl>("node-store")(storage_manager.get_allocator());
			NodeStorage::default_instance(
					NodeStorage::new_instance<PersistentNodeStorageBackend>(nodestore_backend));
		}
		// create temp file with the data
		char file_name[] = "/tmp/rdf-data.ttl";
		std::ofstream rdf_file{file_name};
		rdf_file << rdf_data;
		rdf_file.close();
		// setup triple store
		auto *store = storage_manager.find_or_construct<triple_store::TripleStore>("triple-store")(storage_manager.get_allocator());
		store->load_ttl({file_name});
		// delete the temp file
		std::remove(file_name);
		return store;
	}

	std::vector<std::map<rdf4cpp::rdf::query::Variable, rdf_tensor::NodeWrapper>>
	eval_sparql_query(std::string const &query_str, triple_store::TripleStore const &store) {
		// create sparql query
		auto sparql_query = dice::sparql2tensor::parser::SPARQLParser::parse_query(query_str, store);
		// evaluate sparql query
		std::vector<std::map<rdf4cpp::rdf::query::Variable, rdf_tensor::NodeWrapper>> actual_results{};
		auto raw_query = sparql_query.raw_query();
		for (auto const &entry : rdf_tensor::QueryEvaluation::evaluate(raw_query)) {
			for (size_t i = 0; i < entry.value(); i++) {
				std::map<rdf4cpp::rdf::query::Variable, rdf_tensor::NodeWrapper> result{};
				for (size_t j = 0; j < entry.key().size(); j++) {
					if (entry.key()[j].null())
						continue;
					result[sparql_query.projected_variables()[j]] = entry.key()[j];
				}
				actual_results.push_back(result);
			}
		}
		return actual_results;
	}

	bool compare_results(std::vector<std::map<rdf4cpp::rdf::query::Variable, rdf_tensor::NodeWrapper>> actual_results,
						 std::vector<std::map<rdf4cpp::rdf::query::Variable, rdf_tensor::NodeWrapper>> expected_results) {
		std::sort(expected_results.begin(), expected_results.end());
		std::sort(actual_results.begin(), actual_results.end());
		std::cout << "Actual Results" << std::endl;
		for (auto const &result : actual_results) {
			for (auto const &[var, binding] : result) {
				std::cout << var << ":" << binding << " ";
			}
			std::cout << std::endl;
		}
		std::cout << "Expected Results" << std::endl;
		for (auto const &result : expected_results) {
			for (auto const &[var, binding] : result) {
				std::cout << var << ":" << binding << " ";
			}
			std::cout << std::endl;
		}
		return (actual_results == expected_results);
	}

	std::vector<std::map<rdf4cpp::rdf::query::Variable, rdf_tensor::NodeWrapper>>
	parse_sparql_result_file(std::string const &sparql_results) {
		pugi::xml_document query_results;
		pugi::xml_parse_result parsing_result = query_results.load_string(sparql_results.c_str());
		assert(parsing_result);
		std::vector<std::map<rdf4cpp::rdf::query::Variable, rdf_tensor::NodeWrapper>> expected_results{};
		if (auto bool_res = query_results.child("sparql").child("boolean"); bool_res) {
			if (strcmp(bool_res.first_child().value(), "true") == 0)
				expected_results.emplace_back();
		} else {
			for (const auto &result : query_results.child("sparql").child("results").children()) {
				std::map<rdf4cpp::rdf::query::Variable, rdf_tensor::NodeWrapper> single_result{};
				for (const auto &binding : result.children()) {
					auto var = rdf4cpp::rdf::query::Variable(binding.attribute("name").value());
					rdf4cpp::rdf::Node term{};
					if (auto uri = binding.child("uri"); uri) {
						term = rdf4cpp::rdf::IRI(uri.first_child().value());
					} else if (auto literal = binding.child("literal"); literal) {
						auto lexical_form = literal.first_child().value();
						if (auto datatype = literal.attribute("datatype"); datatype) {
							term = rdf4cpp::rdf::Literal(lexical_form, rdf4cpp::rdf::IRI(datatype.value()));
						} else if (auto lang_tag = literal.attribute("xml:lang")) {
							term = rdf4cpp::rdf::Literal(lexical_form, lang_tag.value());
						} else {
							term = rdf4cpp::rdf::Literal(lexical_form);
						}
					} else if (auto bnode = binding.child("bnode"); bnode) {
						term = rdf4cpp::rdf::BlankNode(bnode.first_child().value());
					} else {
						assert(false);
					}
					single_result[var] = term;
				}
				expected_results.push_back(std::move(single_result));
			}
		}
		return expected_results;
	}

	// from: https://stackoverflow.com/a/9786295
	static size_t curl_write_callback_function(void *contents, size_t size, size_t nmemb, void *userp) {
		((std::string *) userp)->append((char *) contents, size * nmemb);
		return size * nmemb;
	}

	std::string read_file_from_url(std::string const &url) {
		CURL *curl;
		curl = curl_easy_init();
		std::string curl_result;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback_function);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curl_result);
		auto res = curl_easy_perform(curl);
		assert(res == CURLE_OK);
		curl_easy_cleanup(curl);
		return curl_result;
	}

}// namespace dice::tests::sparql

#endif//TENTRIS_TESTS_TESTS_COMMONS_HPP
