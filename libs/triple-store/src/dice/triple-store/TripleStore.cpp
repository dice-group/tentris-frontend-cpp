#include "TripleStore.hpp"

#include <dice/triple-store/SerdLoad.hpp>

namespace dice::triple_store {
	void TripleStore::load_ttl(const std::string &file_path, uint32_t bulk_size, const rdf_tensor::HypertrieBulkInserter::BulkInserted_callback &call_back) {
		std::unique_lock<std::shared_mutex> writer_lock{mutex_};
		HypertrieBulkInserter bulk_inserter{hypertrie_, bulk_size, call_back};
		AddTripleCallback_function add_entry_callback =
				[&bulk_inserter](rdf4cpp::rdf::Node subj, rdf4cpp::rdf::Node pred, rdf4cpp::rdf::Node obj) noexcept -> void {
			hypertrie::internal::raw::SingleEntry<3, htt_t> entry{{subj, pred, obj}};
			bulk_inserter.add(entry);
		};
		serd_load(file_path, add_entry_callback);
	}
	void TripleStore::add_statement(const rdf4cpp::rdf::Statement &statement) {
		std::unique_lock<std::shared_mutex> writer_lock{mutex_};
		Key key{statement.subject(), statement.predicate(), statement.object()};
		hypertrie_.set(key, true);
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

	std::generator<rdf_tensor::Entry const &> TripleStore::eval_select(const sparql2tensor::SPARQLQuery &query, std::chrono::steady_clock::time_point endtime) {
		std::shared_lock<std::shared_mutex> reader_lock{mutex_};
		auto operands = generate_operands(hypertrie_, query.get_slice_keys());
		std::vector<char> proj_vars_id{};
		for (auto const &proj_var : query.projected_variables_) {
			proj_vars_id.push_back(query.var_to_id_.at(proj_var));
		}
		rdf_tensor::Query q{query.odg_, operands, proj_vars_id, endtime};
		if (query.distinct_) {
			rdf_tensor::Entry entry;
			entry.key().resize(query.projected_variables_.size());
			for (auto const &distinct_entry : dice::query::Evaluation::evaluate<htt_t, allocator_type, true>(q)) {
				std::copy(distinct_entry.key().begin(), distinct_entry.key().end(), entry.key().begin());
				co_yield entry;
			}
		} else {
			for (auto const &entry : dice::query::Evaluation::evaluate<htt_t, allocator_type>(q)) {
				co_yield entry;
			}
		}
	}
	bool TripleStore::eval_ask(const sparql2tensor::SPARQLQuery &query, std::chrono::steady_clock::time_point endtime) {
		std::shared_lock<std::shared_mutex> reader_lock{mutex_};
		auto operands = generate_operands(hypertrie_, query.get_slice_keys());
		rdf_tensor::Query q{query.odg_, operands, {}, endtime};
		return dice::query::Evaluation::evaluate_ask<htt_t, allocator_type>(q);
	}
	size_t TripleStore::count(const sparql2tensor::SPARQLQuery &query, std::chrono::steady_clock::time_point endtime) {
		std::shared_lock<std::shared_mutex> reader_lock{mutex_};
		using namespace sparql2tensor;
		if (query.triple_patterns_.size() == 1) {// O(1)
			auto slice_key = query.get_slice_keys()[0];
			if (slice_key.get_fixed_depth() == 3)
				return (size_t) std::get<bool>(get_hypertrie()[slice_key]);
			else
				return std::get<const_BoolHypertrie>(get_hypertrie()[slice_key]).size();
		} else {
			size_t count = 0;
			for (auto const &entry : this->eval_select(query, endtime))
				count += entry.value();
			return count;
		}
	}
}// namespace dice::triple_store
