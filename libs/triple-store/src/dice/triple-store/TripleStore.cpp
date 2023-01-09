#include "TripleStore.hpp"

#include <rdf4cpp/rdf.hpp>

#include <fstream>

namespace dice::triple_store {

	TripleStore::TripleStore(TripleStore::BoolHypertrie &hypertrie) : hypertrie_(hypertrie), inserter_(hypertrie_) {}

	TripleStore::~TripleStore() {
		std::unique_lock<std::shared_mutex> writer_lock{mutex_};
		inserter_.flush();
	}

	void TripleStore::load_ttl(std::string const &file_path,
							   uint32_t bulk_size,
							   rdf_tensor::HypertrieBulkInserter::BulkProcessed_callback const &call_back) {
		std::ifstream ifs{file_path};

		if (!ifs.is_open()) {
			throw std::runtime_error{"unable to open provided file " + file_path};
		}

		flush();
		std::unique_lock<std::shared_mutex> writer_lock{mutex_};
		HypertrieBulkInserter bulk_inserter{hypertrie_, bulk_size, call_back};
		for (rdf4cpp::rdf::parser::IStreamQuadIterator qit{ifs}; qit != rdf4cpp::rdf::parser::IStreamQuadIterator{}; ++qit) {
			if (qit->has_value()) {
				auto const &quad = qit->value();
				bulk_inserter.add(
						hypertrie::internal::raw::SingleEntry<3, htt_t>{{quad.subject(), quad.predicate(), quad.object()}});
			} else {
				std::cerr << qit->error() << '\n';
			}
		}
	}

	void TripleStore::add_statement(const rdf4cpp::rdf::Statement &statement) {
		std::unique_lock<std::shared_mutex> writer_lock{mutex_};
		hypertrie::internal::raw::SingleEntry<3, htt_t> entry{{statement.subject(), statement.predicate(), statement.object()}};
		inserter_.add(entry);
	}

	bool TripleStore::is_rdf_list(rdf4cpp::rdf::Node list) const noexcept {
		flush();
		std::shared_lock<std::shared_mutex> reader_lock{mutex_};

		using IRI = rdf4cpp::rdf::IRI;
		IRI rdf_nil("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil");

		if (list == rdf_nil) return true;// empty collection

		auto prop_obj = std::get<0>(hypertrie_[rdf_tensor::SliceKey{list, std::nullopt, std::nullopt}]);
		if (prop_obj.empty()) return false;

		{
			IRI rdf_first("http://www.w3.org/1999/02/22-rdf-syntax-ns#first");
			auto has_first = std::get<0>(prop_obj[rdf_tensor::SliceKey{rdf_first, std::nullopt}]);
			if (has_first.size() != 1) return false;
		}

		{
			IRI rdf_rest("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest");
			auto has_rest = std::get<0>(prop_obj[rdf_tensor::SliceKey{rdf_rest, std::nullopt}]);
			if (has_rest.size() != 1) return false;
		}

		return true;
	}

	std::vector<rdf4cpp::rdf::Node> TripleStore::get_rdf_list(rdf4cpp::rdf::Node list) const {
		flush();
		std::shared_lock<std::shared_mutex> reader_lock{mutex_};
		using IRI = rdf4cpp::rdf::IRI;
		using Node = rdf4cpp::rdf::Node;

		IRI rdf_first("http://www.w3.org/1999/02/22-rdf-syntax-ns#first");
		IRI rdf_rest("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest");
		IRI rdf_nil("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil");

		std::vector<Node> node_vector;
		auto head = list;
		while (head != rdf_nil) {
			auto element = std::get<0>(hypertrie_[rdf_tensor::SliceKey{list, rdf_first, std::nullopt}]);
			if (element.size() > 1)
				throw std::runtime_error("Invalid RDF seq. Multiple first elements for list node {}" + std::string(head));
			if (element.empty())
				throw std::runtime_error("Invalid RDF seq. No first elements for list node {}" + std::string(head));

			node_vector.push_back((*element.begin())[0]);
			auto rest = std::get<0>(hypertrie_[rdf_tensor::SliceKey{list, rdf_rest, std::nullopt}]);
			if (rest.size() > 1) {
				throw std::runtime_error("Invalid RDF seq. Multiple rest elements for list node {}" + std::string(head));
			} else if (rest.size() == 1) {
				head = (*element.begin())[0];
			} else /* rest.size() == 0 */ {

				head = rdf_nil;// this is not canonical but seems better than throwing an error
			}
		}
		return node_vector;
	}

	/**
	 * @brief Generates the tensor operands of a query
	 * @param slice_keys The slice keys corresponding to the query being evaluated
	 * @return A vector of tensor operands (const_BoolHypertries).
	 */
	std::vector<rdf_tensor::const_BoolHypertrie> generate_operands(rdf_tensor::BoolHypertrie rdf_tensor, std::vector<rdf_tensor::SliceKey> const &slice_keys) {
		using const_BoolHypertrie = rdf_tensor::const_BoolHypertrie;
		using BoolHypertrie = rdf_tensor::BoolHypertrie;

		std::vector<const_BoolHypertrie> operands;
		for (auto const &slice_key : slice_keys) {
			auto slice_result = rdf_tensor[slice_key];
			if (slice_key.get_fixed_depth() == 3) {
				auto entry_exists = std::get<bool>(slice_result);
				BoolHypertrie ht_0{0, rdf_tensor.context()};
				if (entry_exists)
					ht_0.set({}, true);
				operands.push_back(ht_0);
			} else {
				auto operand = std::get<const_BoolHypertrie>(slice_result);
				operands.push_back(std::move(operand));
			}
		}
		return operands;
	}

	TripleStore::QueryResult TripleStore::eval_query(std::string const &query_str,
													 std::chrono::steady_clock::time_point endtime) const {
		flush();
		std::shared_lock<std::shared_mutex> reader_lock{mutex_};
		auto query = dice::sparql::parser::SPARQLParser::parse_query(query_str, hypertrie_, endtime);
		auto raw_query = query.raw_query();
		auto generator = rdf_tensor::QueryEvalaution::evaluate(raw_query, endtime);
		if (query.ask())
			return generator.begin() != generator.end();
		auto proj_vars = query.projected_variables();
		return std::make_pair<std::vector<rdf4cpp::rdf::query::Variable>,
							  std::generator<rdf_tensor::SolutionMapping const &>>(std::move(proj_vars), std::move(generator));
	}

	bool TripleStore::contains(const rdf4cpp::rdf::Statement &statement) const {
		std::shared_lock<std::shared_mutex> reader_lock{mutex_};
		return hypertrie_[Key{statement.subject(), statement.predicate(), statement.object()}];
	}

	size_t TripleStore::size() const {
		flush();
		return hypertrie_.size();
	}

	void TripleStore::flush() const {
		std::unique_lock<std::shared_mutex> writer_lock{mutex_};
		inserter_.flush();
	}
}// namespace dice::triple_store
