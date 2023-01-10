#ifndef TENTRIS_STORE_TRIPLESTORE
#define TENTRIS_STORE_TRIPLESTORE

#include <dice/rdf-tensor/Query.hpp>
#include <dice/triple-store/SyncedLRUCache.hpp>

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
		using HypertrieSyncBulkInserter = rdf_tensor::HypertrieSyncBulkInserter;
		using BoolHypertrie = rdf_tensor::BoolHypertrie;
		using const_BoolHypertrie = rdf_tensor::const_BoolHypertrie;
		using Key = rdf_tensor::Key;
		using htt_t = rdf_tensor::htt_t;
		using SPARQLQueryCache = SyncedLRUCache<std::string, rdf_tensor::SPARQLQuery>;

	public:
		using allocator_type = rdf_tensor::allocator_type;
		using SolutionMappingGenerator = std::generator<rdf_tensor::SolutionMapping const &>;

	private:
		BoolHypertrie &hypertrie_;
		mutable std::shared_mutex mutex_;
		mutable HypertrieSyncBulkInserter inserter_;
		mutable SPARQLQueryCache sparql_cache_;


	public:
		explicit TripleStore(BoolHypertrie &hypertrie);

		~TripleStore();

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
		std::vector<rdf4cpp::rdf::Node> get_rdf_list(rdf4cpp::rdf::Node list) const;

		void load_ttl(
				const std::string &file_path,
				uint32_t bulk_size = 1'000'000,
				HypertrieBulkInserter::BulkProcessed_callback const &call_back = [](size_t, size_t, size_t) -> void {});

		void add_statement(const rdf4cpp::rdf::Statement &statement);

		/**
		 * @brief Evaluation of SPARQL queries.
		 * @param query The SPARQL query.
		 * @param endtime The timeout value
		 * @return A generator yielding the solutions of the query or bool (for ASK)
		 */
		SolutionMappingGenerator
		eval_sparql_query(rdf_tensor::SPARQLQuery const &sparql_query,
						  std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max()) const;

		/**
		 * @brief Parsing of SPARQL queries. Makes use of caching.
		 * @param query The SPARQL query string.
		 * @param endtime The timeout value
		 * @return A SPARQL query
		 */
		std::shared_ptr<const rdf_tensor::SPARQLQuery>
		parse_sparql_query(std::string const &sparql_query_str,
						   std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max()) const;

		bool contains(const rdf4cpp::rdf::Statement &statement) const;

		// todo: add match

		[[nodiscard]] size_t size() const;

		void flush() const;
	};
};    // namespace dice::triple_store
#endif//TENTRIS_STORE_TRIPLESTORE
