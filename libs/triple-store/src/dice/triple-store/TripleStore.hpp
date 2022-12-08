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
	template<typename T, typename Mutex = std::shared_mutex>
	struct UniqueLockGuard {
		using value_type = T;
		using mutex_type = Mutex;
	private:
		std::unique_lock<Mutex> write_lock;
		T value;

	public:
		template<typename ...Us>
		explicit UniqueLockGuard(Mutex &mutex, Us &&...args) : write_lock{mutex}, value{std::forward<Us>(args)...} {
		}

		UniqueLockGuard(Mutex &mutex, value_type const &value) : write_lock{mutex}, value{value} {
		}

		UniqueLockGuard(Mutex &mutex, value_type &&value) : write_lock{mutex}, value{std::move(value)} {
		}

		value_type &operator*() noexcept { return value; }
		value_type const &operator*() const noexcept { return value; }
		value_type *operator->() noexcept { return &value; }
		value_type const *operator->() const noexcept { return &value; }
	};

	class TripleStore {
		using HypertrieContext = rdf_tensor::HypertrieContext;
		using HypertrieContext_ptr = rdf_tensor::HypertrieContext_ptr;
		using HypertrieSyncBulkInserter = rdf_tensor::HypertrieSyncBulkInserter;
		using HypertrieSyncBulkRemvoer = rdf_tensor::HypertrieSyncBulkRemover;
		using BoolHypertrie = rdf_tensor::BoolHypertrie;
		using const_BoolHypertrie = rdf_tensor::const_BoolHypertrie;
		using Key = rdf_tensor::Key;
		using htt_t = rdf_tensor::htt_t;

	public:
		using HypertrieBulkInserter = rdf_tensor::HypertrieBulkInserter;
		using HypertrieBulkRemover = rdf_tensor::HypertrieBulkRemover;
		using allocator_type = rdf_tensor::allocator_type;

	private:
		BoolHypertrie &hypertrie_;
		mutable std::shared_mutex mutex_;
		mutable HypertrieSyncBulkInserter inserter_;


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
				std::string const &file_path,
				uint32_t bulk_size = 1'000'000,
				HypertrieBulkInserter::BulkProcessed_callback const &call_back = [](size_t, size_t, size_t) -> void {},
				std::function<void(rdf_tensor::parser::ParsingError const &)> const &error_callback = [](rdf_tensor::parser::ParsingError const &) -> void {});

		[[nodiscard]] UniqueLockGuard<HypertrieBulkRemover> bulk_remove(uint32_t bulk_size = 1'000'000);
		[[nodiscard]] UniqueLockGuard<HypertrieBulkInserter> bulk_insert(uint32_t bulk_size = 1'000'000);

		void remove(std::vector<rdf_tensor::NonZeroEntry> const &entries, uint32_t bulk_size = 1'000'000);
		void insert(std::vector<rdf_tensor::NonZeroEntry> const &entries, uint32_t bulk_size = 1'000'000);

		void add_statement(const rdf4cpp::rdf::Statement &statement);

		/**
		 * @brief Evaluation of SPARQL SELECT queries.
		 * @param query The parsed SPARQL query.
		 * @param endtime The timeout value
		 * @return A generator yielding the solutions of the query
		 */
		std::generator<rdf_tensor::Entry const &>
		eval_select(const sparql2tensor::SPARQLQuery &query,
					std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max()) const;

		/**
		 * @brief Evaluation of SPARQL ASK queries.
		 * @param query The parsed SPARQL query.
		 * @param endtime The timeout value
		 * @return The result of the ask query (true or false).
		 */
		bool eval_ask(const sparql2tensor::SPARQLQuery &query,
					  std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max()) const;

		size_t count(const sparql2tensor::SPARQLQuery &query,
					 std::chrono::steady_clock::time_point endtime = std::chrono::steady_clock::time_point::max()) const;

		bool contains(const rdf4cpp::rdf::Statement &statement) const;

		// todo: add match

		[[nodiscard]] size_t size() const;

		void flush() const;
	};
};    // namespace dice::triple_store
#endif//TENTRIS_STORE_TRIPLESTORE
