#ifndef TENTRIS_STORE_TRIPLESTORE
#define TENTRIS_STORE_TRIPLESTORE

#include <dice/rdf-tensor/Query.hpp>
#include <dice/rdf-tensor/RDFTensor.hpp>

#include <dice/sparql2tensor/SPARQLQuery.hpp>

#include <dice/triple-store/SerdLoad.hpp>

#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#include <metall/metall.hpp>

#include <mutex>
#include <shared_mutex>

namespace dice::triple_store {

	template<typename T>
	struct ReadMutexGuard {
		std::shared_lock<std::shared_mutex> guard;
		std::reference_wrapper<T const> value;

		[[nodiscard]] T const &get() const {
			return value.get();
		}
	};

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

		std::shared_mutex mutable hypertrie_mutex_;

	public:
		explicit TripleStore(allocator_type const &allocator)
			: context_(allocator),
			  hypertrie_(3, HypertrieContext_ptr(&context_)) {}

		[[nodiscard]] ReadMutexGuard<BoolHypertrie> get_hypertrie() const {
			return ReadMutexGuard<BoolHypertrie> {
				.guard = std::shared_lock{hypertrie_mutex_},
				.value = std::ref(hypertrie_)
			};
		}

		void load_ttl(
				const std::string &file_path,
				uint32_t bulk_size = 1'000'000,
				HypertrieBulkInserter::BulkInserted_callback const &call_back = [](size_t, size_t, size_t) -> void {}) {
			std::unique_lock lock{hypertrie_mutex_};

			HypertrieBulkInserter bulk_inserter{hypertrie_, bulk_size, call_back};
			AddTripleCallback_function add_entry_callback =
					[&bulk_inserter](rdf4cpp::rdf::Node subj, rdf4cpp::rdf::Node pred, rdf4cpp::rdf::Node obj) noexcept -> void {
						hypertrie::internal::raw::SingleEntry<3, htt_t> entry{{subj, pred, obj}};
						bulk_inserter.add(entry);
					};
			serd_load(file_path, add_entry_callback);
		}

		void remove(std::vector<rdf_tensor::NonZeroEntry> const &entries, uint32_t bulk_size = 1'000'000) {
			std::unique_lock lock{hypertrie_mutex_};
			HypertrieBulkRemover bulk_remover{hypertrie_, bulk_size};

			for (auto const &e : entries) {
				bulk_remover.add(e);
			}
		}

		void add_statement(const rdf4cpp::rdf::Statement &statement) {
			static_assert(sizeof(statement.subject()) == sizeof(uint64_t));
			static_assert(sizeof(Key::value_type) == sizeof(uint64_t));

			std::unique_lock lock{hypertrie_mutex_};
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
			std::unique_lock lock{hypertrie_mutex_};

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
			std::unique_lock lock{hypertrie_mutex_};

			auto operands = generate_operands(query.get_slice_keys());
			rdf_tensor::Query q{query.odg_, operands, {}, endtime};
			return dice::query::Evaluation::evaluate_ask<htt_t, allocator_type>(q);
		}

		size_t count(const sparql2tensor::SPARQLQuery &query,
					 std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max()) {
			std::unique_lock lock{hypertrie_mutex_};

			using namespace sparql2tensor;
			if (query.triple_patterns_.size() == 1) {// O(1)
				auto slice_key = query.get_slice_keys()[0];
				if (slice_key.get_fixed_depth() == 3)
					return (size_t) std::get<bool>(hypertrie_[slice_key]);
				else
					return std::get<const_BoolHypertrie>(hypertrie_[slice_key]).size();
			} else {
				size_t count = 0;
				for (auto const &entry : this->eval_select(query, endtime))
					count += entry.value();
				return count;
			}
		}

		bool contains(const rdf4cpp::rdf::Statement &statement) const {
			std::shared_lock lock{hypertrie_mutex_};
			return hypertrie_[Key{statement.subject(), statement.predicate(), statement.object()}];
		}

		[[nodiscard]] size_t size() const {
			std::shared_lock lock{hypertrie_mutex_};
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
