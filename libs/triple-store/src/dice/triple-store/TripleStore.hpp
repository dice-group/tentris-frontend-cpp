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
		using BoolHypertrie = rdf_tensor::BoolHypertrie;
		using const_BoolHypertrie = rdf_tensor::const_BoolHypertrie;
		using Key = rdf_tensor::Key;
		using htt_t = rdf_tensor::htt_t;

	public:
		using allocator_type = rdf_tensor::allocator_type;

	private:
		HypertrieContext context_;
		BoolHypertrie hypertrie_;
		std::shared_mutex mutex_;

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
				HypertrieBulkInserter::BulkInserted_callback const &call_back = [](size_t, size_t, size_t) -> void {});

		void add_statement(const rdf4cpp::rdf::Statement &statement);

		/**
		 * @brief Evaluation of SPARQL SELECT queries.
		 * @param query The parsed SPARQL query.
		 * @param endtime The timeout value
		 * @return A generator yielding the solutions of the query
		 */
		std::generator<rdf_tensor::Entry const &>
		eval_select(const sparql2tensor::SPARQLQuery &query,
					std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max());

		/**
		 * @brief Evaluation of SPARQL ASK queries.
		 * @param query The parsed SPARQL query.
		 * @param endtime The timeout value
		 * @return The result of the ask query (true or false).
		 */
		bool eval_ask(const sparql2tensor::SPARQLQuery &query,
					  std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max());

		size_t count(const sparql2tensor::SPARQLQuery &query,
					 std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max());

		bool contains(const rdf4cpp::rdf::Statement &statement) {
			std::shared_lock<std::shared_mutex> reader_lock{mutex_};
			return hypertrie_[Key{statement.subject(), statement.predicate(), statement.object()}];
		}

		[[nodiscard]] size_t size() const {
			return hypertrie_.size();
		}
	};
};    // namespace dice::triple_store
#endif//TENTRIS_STORE_TRIPLESTORE
