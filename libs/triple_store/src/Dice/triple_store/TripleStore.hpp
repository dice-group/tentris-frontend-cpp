#ifndef TENTRIS_STORE_TRIPLESTORE
#define TENTRIS_STORE_TRIPLESTORE

#include <Dice/rdf_tensor/Query.hpp>
#include <Dice/rdf_tensor/RDFTensor.hpp>

#include <Dice/sparql2tensor/SPARQLQuery.hpp>

#include <Dice/triple_store/SerdLoad.hpp>

#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#include <metall/metall.hpp>

namespace Dice::triple_store {
	class TripleStore {

		using HypertrieContext = rdf_tensor::HypertrieContext;
		using HypertrieContext_ptr = rdf_tensor::HypertrieContext_ptr;
		using HypertrieBulkInserter = rdf_tensor::HypertrieBulkInserter;
		using BoolHypertrie = rdf_tensor::BoolHypertrie;
		using const_BoolHypertrie = rdf_tensor::const_BoolHypertrie;
		using Key = rdf_tensor::Key;
		using htt_t = rdf_tensor::htt_t;

	public:
		using allocator_type = rdf_tensor::allocator_type;
		using Entry = rdf_tensor::Entry;

	private:
		HypertrieContext context_;
		BoolHypertrie hypertrie_;

	public:
		explicit TripleStore(allocator_type const &allocator)
			: context_(allocator),
			  hypertrie_(3, HypertrieContext_ptr(&context_)) {}

		[[nodiscard]] BoolHypertrie const &get_hypertrie() const {
			return hypertrie_;
		}

		void load_ttl(
				const std::string &file_path,
				uint32_t bulk_size = 1'000'000,
				HypertrieBulkInserter::BulkInserted_callback const &call_back = [](size_t, size_t, size_t) -> void {}) {
			HypertrieBulkInserter bulk_inserter{hypertrie_, bulk_size, call_back};
			AddTripleCallback_function add_entry_callback =
					[&bulk_inserter](rdf4cpp::rdf::Node subj, rdf4cpp::rdf::Node pred, rdf4cpp::rdf::Node obj) noexcept -> void {
						hypertrie::internal::raw::SingleEntry<3, htt_t> entry{{subj, pred, obj}};
						bulk_inserter.add(entry);
					};
			serd_load(file_path, add_entry_callback);
		}

		void add_statement(const rdf4cpp::rdf::Statement &statement) {
			static_assert(sizeof(statement.subject()) == sizeof(uint64_t));
			static_assert(sizeof(Key::value_type) == sizeof(uint64_t));
			Key key{statement.subject(), statement.predicate(), statement.object()};
			hypertrie_.set(key, true);
		}

		/**
		 * @brief Evaluation of SPARQL SELECT queries.
		 * @param query The parsed SPARQL query.
		 * @param endtime The timeout value
		 * @return A generator yielding the solutions of the query
		 */
		std::generator<rdf_tensor::Entry const &>
		eval_select(const sparql2tensor::SPARQLQuery &query,
					std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max()) {
			if (query.contains_aggregates_) {
				co_yield std::elements_of(eval_select_aggr(query, endtime));
			} else {
				co_yield std::elements_of(eval_select_without_aggr(query, endtime));
			}
		}

		bool eval_ask(const sparql2tensor::SPARQLQuery &query,
					  std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max()) {
			auto operands = generate_operands(query.get_slice_keys());
			rdf_tensor::Query q{query.odg_, operands, {}, endtime};
			return Dice::query::Evaluation::evaluate_ask<htt_t, allocator_type>(q);
		}

		bool contains(const rdf4cpp::rdf::Statement &statement) {
			return hypertrie_[Key{statement.subject(), statement.predicate(), statement.object()}];
		}

		[[nodiscard]] size_t size() const {
			return hypertrie_.size();
		}

	private:
		/* evaluation of select queries that do not contain any aggregates */
		template<bool HavingClause = false>
		std::generator<rdf_tensor::Entry const &>
		eval_select_without_aggr(const sparql2tensor::SPARQLQuery &query,
								 std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max()) {
			auto &solution_expressions =  query.solution_.expressions();
			rdf_tensor::Entry solution_mapping;
			solution_mapping.key().resize(solution_expressions.size());
			auto operands = generate_operands(query.get_slice_keys());
			std::vector<char> tracked_vars_id{};
			tracked_vars_id.resize(query.tracked_variables_.size());
			for (auto const &tracked_var : query.tracked_variables_) {
				tracked_vars_id[tracked_var.second] = query.var_to_id_.at(tracked_var.first);
			}
			rdf_tensor::Query q{query.odg_, operands, tracked_vars_id, endtime};
			if (query.distinct_) {
				rdf_tensor::Entry entry;
				entry.resize(solution_expressions.size());
				for (auto const &distinct_entry : Dice::query::Evaluation::evaluate<htt_t, allocator_type, true>(q)) {
					std::copy(distinct_entry.key().begin(), distinct_entry.key().end(), entry.key().begin());
					for (size_t i = 0; i < solution_expressions.size(); i++) {
						solution_expressions[i]->evaluate(entry);
						solution_mapping[i] = solution_expressions[i]->result();
					}
					co_yield solution_mapping;
				}
			} else {
				for (auto const &entry : Dice::query::Evaluation::evaluate<htt_t, allocator_type>(q)) {
					for (size_t i = 0; i < solution_expressions.size(); i++) {
						solution_expressions[i]->evaluate(entry);
						solution_mapping[i] = solution_expressions[i]->result();
					}
					solution_mapping.value(entry.value());
					co_yield solution_mapping;
				}
			}
		}

		/* evaluation of select queries containing at least one aggregate */
		std::generator<rdf_tensor::Entry const &>
		eval_select_aggr(const sparql2tensor::SPARQLQuery &query,
						 std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max()) {
			std::map<rdf_tensor::Entry, sparql2tensor::expressions::ExpressionList> grouped_solutions{};
			rdf_tensor::Entry group_key;
			group_key.key().resize(query.grouping_keys_.size());
			rdf_tensor::Entry solution_mapping;
			solution_mapping.key().resize(query.solution_.expressions().size());
			auto operands = generate_operands(query.get_slice_keys());
			std::vector<char> tracked_vars_id{};
			tracked_vars_id.resize(query.tracked_variables_.size());
			for (auto const &tracked_var : query.tracked_variables_) {
				tracked_vars_id[tracked_var.second] = query.var_to_id_.at(tracked_var.first);
			}
			rdf_tensor::Query q{query.odg_, operands, tracked_vars_id, endtime};
			for (auto const &entry : Dice::query::Evaluation::evaluate<htt_t, allocator_type>(q)) {
				// prepare grouping key
				for (size_t i = 0; i < group_key.size(); i++) {
					query.grouping_keys_[i]->evaluate(entry);
					group_key[i] = query.grouping_keys_[i]->result();
				}
				// new key, clone solution
				if (not grouped_solutions.contains(group_key)) {
					grouped_solutions[group_key] = query.solution_.clone();
				}
				// update aggregate values
				auto card = entry.value();
				while (card > 0) {
					for (auto &aggr : grouped_solutions[group_key].expressions()) {
						aggr->evaluate(entry);
					}
					card--;
				}
			}
			/* non-distinct queries */
			if (not query.distinct_) {
				for (auto const &[key, solution] : grouped_solutions) {
					auto const &solution_expressions = solution.expressions();
					for (size_t i = 0; i < solution_expressions.size(); i++) {
						solution_mapping[i] = solution_expressions[i]->result();
					}
					co_yield solution_mapping;
				}
			}
			/* distinct queries -- for queries that do not project all grouping keys */
			else {
				boost::container::flat_set<Entry> seen_entries{};
				rdf_tensor::Entry entry;
				entry.resize(solution_mapping.size());
				for (auto const &[key, solution] : grouped_solutions) {
					auto const &solution_expressions = solution.expressions();
					for (size_t i = 0; i < solution_expressions.size(); i++) {
						solution_mapping[i] = solution_expressions[i]->result();
					}
					if (seen_entries.contains(solution_mapping))
						continue;
					seen_entries.insert(solution_mapping);
					co_yield solution_mapping;
				}
			}
		}

	private:
		/**
		 * @brief Generates the tensor operands of a query
		 * @param slice_keys The slice keys corresponding to the query being evaluated
		 * @return A vector of tensor operands (const_BoolHypertries).
		 */
		std::vector<const_BoolHypertrie> generate_operands(std::vector<rdf_tensor::SliceKey> const &slice_keys) {
			std::vector<const_BoolHypertrie> operands;
			for (auto const &slice_key : slice_keys) {
				auto slice_result = hypertrie_[slice_key];
				if (slice_key.get_fixed_depth() == 3) {
					auto entry_exists = std::get<bool>(slice_result);
					BoolHypertrie ht_0{0, &context_};
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
	};
};    // namespace Dice::triple_store
#endif//TENTRIS_STORE_TRIPLESTORE
