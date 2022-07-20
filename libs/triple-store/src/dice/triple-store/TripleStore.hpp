#ifndef TENTRIS_STORE_TRIPLESTORE
#define TENTRIS_STORE_TRIPLESTORE

#include <dice/rdf-tensor/Query.hpp>
#include <dice/rdf-tensor/RDFTensor.hpp>

#include <dice/sparql2tensor/SPARQLQuery.hpp>

#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#include <metall/metall.hpp>

#include <shared_mutex>

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
		mutable std::shared_mutex mutex_;

	public:
		explicit TripleStore(allocator_type const &allocator)
			: context_(allocator),
			  hypertrie_(3, HypertrieContext_ptr(&context_)) {}

		[[nodiscard]] BoolHypertrie const &get_hypertrie() const {
			return hypertrie_;
		}

		/**
		 * This function enforces stricter requirements upon rdf:Lists than described in <a href="https://www.w3.org/TR/2014/REC-rdf11-mt-20140225/#rdf-containers">D.3 RDF collections</a>.
		 * An rdf:List must either be the IRI rdf:nil or must have the properties rdf:first and rdf:rest, both with cardinality 1.
		 * @param list the node to be checked if it is a list
		 * @return if list is an rdf:List
		 */
		[[nodiscard]] bool is_rdf_list(rdf4cpp::rdf::Node list) const noexcept;

		/**
		 * Returns the items of an rdf:List as vector.
		 *
		 * Restrictions from is_rdf_list(rdf4cpp::rdf::Node) const noexcept apply.
		 *
		 * @param list the start node of the list
		 * @return the elements of the list as vector
		 * @throws std::runtime_error If the list is malformed.
		 */
		std::vector<rdf4cpp::rdf::Node> get_rdf_list(rdf4cpp::rdf::Node list);

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
