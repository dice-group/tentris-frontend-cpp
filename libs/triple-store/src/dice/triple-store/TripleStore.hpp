#ifndef TENTRIS_STORE_TRIPLESTORE
#define TENTRIS_STORE_TRIPLESTORE

#include <dice/rdf-tensor/Query.hpp>
#include <dice/rdf-tensor/RDFTensor.hpp>

#include <dice/sparql2tensor/SPARQLQuery.hpp>

#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#include <metall/metall.hpp>

namespace dice::triple_store {
	class TripleStore {

		using HypertrieContext = rdf_tensor::HypertrieContext;
		using HypertrieContext_ptr = rdf_tensor::HypertrieContext_ptr;
		using HypertrieBulkInserter = rdf_tensor::HypertrieBulkInserter;
		using HypertrieBulkRemover = rdf_tensor::HypertrieBulkRemover;
		using BoolHypertrie = rdf_tensor::BoolHypertrie;
		using const_BoolHypertrie = rdf_tensor::const_BoolHypertrie;
		using Key = rdf_tensor::Key;
		using htt_t = rdf_tensor::htt_t;

	public:
		using allocator_type = rdf_tensor::allocator_type;

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
				HypertrieBulkInserter::BulkProcessed_callback const &call_back = [](size_t, size_t, size_t) -> void {},
				std::function<void(rdf_tensor::parser::ParsingError const &)> const &error_callback = [](rdf_tensor::parser::ParsingError const &) -> void {}) {
			HypertrieBulkInserter bulk_inserter{hypertrie_, bulk_size, call_back};

			std::ifstream ifs{file_path};
			if (!ifs.is_open()) {
				throw std::runtime_error{"cannot open file for reading"};
			}

			for (rdf_tensor::parser::IStreamQuadIterator qit{ifs}; qit != rdf_tensor::parser::IStreamQuadIterator{}; ++qit) {
				if (qit->has_value()) {
					auto const &triple = **qit;
					bulk_inserter.add(hypertrie::internal::raw::SingleEntry<3, htt_t>{{triple.subject(), triple.predicate(), triple.object()}});
				} else {
					error_callback(qit->error());
				}
			}
		}

		HypertrieBulkRemover bulk_remove(uint32_t bulk_size = 1'000'000) noexcept {
			return HypertrieBulkRemover{hypertrie_, bulk_size};
		}

		HypertrieBulkInserter bulk_insert(uint32_t bulk_size = 1'000'000) noexcept {
			return HypertrieBulkInserter{hypertrie_, bulk_size};
		}

		void remove(std::vector<rdf_tensor::NonZeroEntry> const &entries, uint32_t bulk_size = 1'000'000) {
			HypertrieBulkRemover bulk_remover{hypertrie_, bulk_size};

			for (auto const &e : entries) {
				bulk_remover.add(e);
			}
		}

		void insert(std::vector<rdf_tensor::NonZeroEntry> const &entries, uint32_t bulk_size = 1'000'000) {
			HypertrieBulkInserter bulk_inserter{hypertrie_, bulk_size};

			for (auto const &e : entries) {
				bulk_inserter.add(e);
			}
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
			auto operands = generate_operands(query.get_slice_keys());
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

		/**
		 * @brief Evaluation of SPARQL ASK queries.
		 * @param query The parsed SPARQL query.
		 * @param endtime The timeout value
		 * @return The result of the ask query (true or false).
		 */
		bool eval_ask(const sparql2tensor::SPARQLQuery &query,
					  std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max()) {
			auto operands = generate_operands(query.get_slice_keys());
			rdf_tensor::Query q{query.odg_, operands, {}, endtime};
			return dice::query::Evaluation::evaluate_ask<htt_t, allocator_type>(q);
		}

		size_t count(const sparql2tensor::SPARQLQuery &query,
					 std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max()) {
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

		bool contains(const rdf4cpp::rdf::Statement &statement) {
			return hypertrie_[Key{statement.subject(), statement.predicate(), statement.object()}];
		}

		[[nodiscard]] size_t size() const {
			return hypertrie_.size();
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
};    // namespace dice::triple-store
#endif//TENTRIS_STORE_TRIPLESTORE
